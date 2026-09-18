#include "Game.h"

#include <charconv>
#include <iostream>

#include "core/PathPrefixes.h"

#ifdef __ANDROID__
#include <android_native_app_glue.h>
#else
#include <GLFW/glfw3.h>
#endif

namespace {

std::optional<uint32_t> ParseBlockIdFromPreviewSuffix(std::string_view remainder) {
  const auto dot = remainder.find('.');
  if (dot == std::string_view::npos) return std::nullopt;
  if (remainder.substr(dot + 1) != "preview") return std::nullopt;

  uint32_t id = 0;
  const auto [ptr, ec] = std::from_chars(remainder.data(), remainder.data() + dot, id);
  return ec == std::errc{} ? std::optional(id) : std::nullopt;
}

}  // namespace

// ---------------------------------------------------------------------------
// Platform ctors
// ---------------------------------------------------------------------------

#ifdef __ANDROID__
Game::Game(android_app* app) {
  if (!app) throw std::runtime_error("Game: android_app is null");
  engine_ = std::make_unique<Engine>(app);
  cursorLocked_ = false;
  Init();
}
#else
Game::Game(std::unique_ptr<core::Window> window) : window_(std::move(window)) {
  if (!window_) throw std::runtime_error("Game: window is null");
  engine_ = std::make_unique<Engine>(window_->window());
  cursorLocked_ = true;
  Init();
  glfwSetInputMode(window_->window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
#endif

// ---------------------------------------------------------------------------
// Shared init
// ---------------------------------------------------------------------------

void Game::Init() {
  const vkcore::Device& device = engine_->getDevice();
  // world
  constexpr uint32_t kWorldW = 3;
  constexpr uint32_t kWorldH = 1;
  constexpr uint32_t kWorldD = 3;

  session_ = std::make_unique<gm::WorldSession>(kWorldW, kWorldH, kWorldD, core::kAssetsPrefix);
  render_ = std::make_unique<gfx::RenderWorld>(*engine_, *session_, core::kAssetsPrefix);

  gfx::ShaderCompiler& compiler = render_->shaderCompiler();

  vkcore::ShaderModule crosshairVert =
      gfx::CompileShaderModule(compiler, device, core::kShadersPrefix + "crosshair.vert", shaderc_vertex_shader);

  vkcore::ShaderModule crosshairFrag = gfx::CompileShaderModule(
      compiler, device, core::kShadersPrefix + "crosshair.frag", shaderc_fragment_shader);

  // crosshair
  crosshairLayout_ = std::make_unique<vkcore::PipelineLayout>(
      device, std::vector<const vkcore::DescriptorSetLayout*>{},
      std::vector<VkPushConstantRange>{});

  crosshairPipeline_ = std::make_unique<vkcore::Pipeline>(
      vkcore::GraphicsPipelineCreator(device)
          .AddShaderStage(crosshairVert,
                          VK_SHADER_STAGE_VERTEX_BIT)
          .AddShaderStage(crosshairFrag,
                          VK_SHADER_STAGE_FRAGMENT_BIT)
          .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
          .AddColorBlendAttachment(false)
          .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
          .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
          .setDepthTest(false, false)
          .setCullMode(VK_CULL_MODE_NONE)
          .Build(crosshairLayout_->handle(), engine_->getRenderPass().handle()));

  
  const glm::vec3 worldCenter((kWorldW * static_cast<float>(gm::Chunk::kLength)) * 0.5f,
                              (kWorldH * static_cast<float>(gm::Chunk::kLength)) * 0.5f,
                              (kWorldD * static_cast<float>(gm::Chunk::kLength)) * 0.5f);

  player_ = std::make_unique<gm::PlayerController>(*session_, worldCenter + glm::vec3(16, 70, 16));

  session_->entities().Create("barrel", worldCenter + glm::vec3(4.0f));
  session_->entities().Create("integrity", glm::vec3(20.0f));

  blockPreviewRenderer_ = std::make_unique<gfx::BlockPreviewRenderer>(
      device, engine_->getCommandPool(), engine_->getGraphicsQueue(), engine_->memoryAllocator(), compiler,
      render_->chunks().blockRenderData());

  std::ignore = render_->textures().Load(core::kAssetsPrefix + "images/blank.png", "blank");
  render_->textures().AddResolver(
      "blocks.", [this](std::string_view remainder) -> std::optional<vkcore::SampledTexture> {
        const auto id = ParseBlockIdFromPreviewSuffix(remainder);
        if (!id) return std::nullopt;
        return blockPreviewRenderer_->Render(*id);
      });

  ui_.emplace(device, engine_->getCommandPool(), engine_->getRenderPass().handle(), compiler,
              luaState_,
              render_->textures(), engine_->extent());
  libGui_.emplace(*ui_, luaState_);
  libControl_.emplace(controlState_, luaState_);

#ifdef __ANDROID__
  ui_->push(core::kAssetsPrefix + "ui/game_android.xml");
#else
  ui_->push(core::kAssetsPrefix + "ui/game.xml");
#endif

  lastFrameTime_ = std::chrono::high_resolution_clock::now();

  std::cout << "[INFO] Game started. Chunks: " << session_->world().chunks().getChunks().size()
            << "\n";
}

Game::~Game() {
  if (engine_) {
    engine_->getGraphicsQueue().WaitIdle();
  }
}

void Game::SetViewportAndScissor(VkCommandBuffer cmd) {
  const VkExtent2D extent = engine_->extent();

  VkViewport vp{};
  vp.x = 0.0f;
  vp.y = 0.0f;
  vp.width = static_cast<float>(extent.width);
  vp.height = static_cast<float>(extent.height);
  vp.minDepth = 0.0f;
  vp.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = extent;

  auto& dt = engine_->getDevice().dispatchTable();
  dt.vkCmdSetViewport(cmd, 0, 1, &vp);
  dt.vkCmdSetScissor(cmd, 0, 1, &scissor);
}

bool Game::Frame() {
  player_->HandleInput(*window_, controlState_, cursorLocked_);

  // touch swipe for unlocked cursor
  if (!cursorLocked_ && ui_) {
    auto swipe = ui_->routeTouches(window_->input().getState().pointers());
    player_->camera().Rotate(swipe.deltaX, swipe.deltaY);
  }

  uint32_t imageIndex = 0;
  if (!engine_->beginFrame(imageIndex)) {
    engine_->recreateSwapchain();
    return true;
  }

  VkCommandBuffer cmd = engine_->getCommandBuffer().handle();
  uint32_t frame = engine_->getCurrentFrameIndex();

  const auto now = std::chrono::high_resolution_clock::now();
  dt_ = frameCount_ == 0 ? 0.0f : std::chrono::duration<float>(now - lastFrameTime_).count();
  lastFrameTime_ = now;

  if (ui_) {
    if (window_->isResized()) ui_->resize({window_->width(), window_->height()});
    ui_->Update();
  }

  player_->UpdateMovement(controlState_, dt_);

  if (controlState_.breakBlock) {
    player_->TryBreak(*session_, &render_->particles());
  }
  if (controlState_.interact) {
    player_->TryPlace(*session_, controlState_.hotbarSlot);
  }

  session_->Update(dt_);
  render_->UpdateDirty(cmd, frame);

  player_->UpdateCamera(dt_);

  const float screenW = static_cast<float>(engine_->extent().width);
  const float screenH = static_cast<float>(engine_->extent().height);
  render_->UpdateGameData(frame, player_->camera(), screenW, screenH);
  render_->UpdateParticles(dt_, frame);

  engine_->BeginRenderPass(imageIndex, 0.53f, 0.81f, 0.92f);
  SetViewportAndScissor(cmd);

  render_->Render(cmd, dt_, frame, player_->camera());

  if (ui_) ui_->Render(cmd);

  crosshairPipeline_->Bind(cmd);
  engine_->getDevice().dispatchTable().vkCmdDraw(cmd, 4, 1, 0, 0);

  engine_->endRenderPass();
  engine_->endFrame(imageIndex);

  ++frameCount_;
  controlState_.Update();
  window_->Update();
  return true;
}