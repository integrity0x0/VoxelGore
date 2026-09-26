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

#ifdef __ANDROID__

Game::Game(android_app* app) {
  if (!app) throw std::runtime_error("Game: android_app is null");

  engine_ = std::make_unique<Engine>(app);
  cursorLocked_ = false;
  frameTime_ = 1.0f / 120.0f;
  Init();
}

#else

Game::Game(std::unique_ptr<core::Window> window) : window_(std::move(window)) {
  if (!window_) throw std::runtime_error("Game: window is null");

  engine_ = std::make_unique<Engine>(window_->window());
  cursorLocked_ = true;

  GLFWmonitor* monitor = glfwGetWindowMonitor(window_->window());
  if (!monitor) monitor = glfwGetPrimaryMonitor();

  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  const int refreshRate = mode ? mode->refreshRate : 60;

  frameTime_ = 1.0f / static_cast<float>(refreshRate);

  Init();

  glfwSetInputMode(window_->window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

#endif

constexpr uint32_t kWorldW = 4;
constexpr uint32_t kWorldH = 1;
constexpr uint32_t kWorldD = 4;

void Game::Init() {
  const vkcore::Device& device = engine_->getDevice();

  session_ = std::make_unique<gm::WorldSession>(kWorldW, kWorldH, kWorldD, core::kAssetsPrefix);

  render_ = std::make_unique<gfx::RenderWorld>(*engine_, *session_, core::kAssetsPrefix);

  gfx::ShaderCompiler& compiler = render_->shaderCompiler();

  vkcore::ShaderModule crosshairVert = gfx::CompileShaderModule(
      compiler, device, core::kShadersPrefix + "crosshair.vert", shaderc_vertex_shader);

  vkcore::ShaderModule crosshairFrag = gfx::CompileShaderModule(
      compiler, device, core::kShadersPrefix + "crosshair.frag", shaderc_fragment_shader);

  crosshairLayout_ = std::make_unique<vkcore::PipelineLayout>(
      device, std::vector<const vkcore::DescriptorSetLayout*>{},
      std::vector<VkPushConstantRange>{});

  crosshairPipeline_ = std::make_unique<vkcore::Pipeline>(
      vkcore::GraphicsPipelineCreator(device)
          .AddShaderStage(crosshairVert, VK_SHADER_STAGE_VERTEX_BIT)
          .AddShaderStage(crosshairFrag, VK_SHADER_STAGE_FRAGMENT_BIT)
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
  session_->entities().Create("integrity", worldCenter + glm::vec3(10.0f));

  blockPreviewRenderer_ = std::make_unique<gfx::BlockPreviewRenderer>(
      device, engine_->getCommandPool(), engine_->getGraphicsQueue(), engine_->memoryAllocator(),
      compiler, render_->chunks().blockRenderData());

  std::ignore = render_->textures().Load(core::kAssetsPrefix + "images/blank.png", "blank");

  render_->textures().AddResolver(
      "blocks.", [this](std::string_view remainder) -> std::optional<vkcore::SampledTexture> {
        const auto id = ParseBlockIdFromPreviewSuffix(remainder);
        if (!id) return std::nullopt;
        return blockPreviewRenderer_->Render(*id);
      });

  ui_.emplace(device, engine_->getCommandPool(), engine_->getRenderPass().handle(), compiler,
              luaState_, render_->textures(), engine_->extent());

  libGui_.emplace(*ui_, luaState_);
  libControl_.emplace(controlState_, luaState_);

#ifdef __ANDROID__
  ui_->push(core::kAssetsPrefix + "ui/game_android.xml");
#else
  ui_->push(core::kAssetsPrefix + "ui/game.xml");
#endif

  uiExtent_ = engine_->extent();

  lastFrameTime_ = std::chrono::steady_clock::now();

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

  VkViewport vp = {};
  vp.x = 0.0f;
  vp.y = 0.0f;
  vp.width = static_cast<float>(extent.width);
  vp.height = static_cast<float>(extent.height);
  vp.minDepth = 0.0f;
  vp.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = extent;

  auto& dt = engine_->getDevice().dispatchTable();
  dt.vkCmdSetViewport(cmd, 0, 1, &vp);
  dt.vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void Game::UpdateInput() {
#ifndef __ANDROID__
  player_->HandleInput(*window_, controlState_, cursorLocked_);

  if (!cursorLocked_ && ui_) {
    auto swipe = ui_->routeTouches(window_->input().getState().pointers());
    player_->camera().Rotate(swipe.deltaX, swipe.deltaY);
  }
#endif
}

#ifdef __ANDROID__

void Game::UpdateInput(const std::unordered_map<int32_t, core::Pointer>& touches) {
  if (!cursorLocked_ && ui_) {
    auto swipe = ui_->routeTouches(touches);
    player_->camera().Rotate(swipe.deltaX, swipe.deltaY);
  }
}

#endif

bool Game::HandleResize() {
#ifndef __ANDROID__
  if (window_->isResized()) {
    engine_->recreateSwapchain();
  }
  
#else
  if (!engine_->IsRenderable()) return false;
  engine_->restoreSurface();
#endif

  return true;
}

void Game::UpdateUiSize() {
  if (!ui_) return;

  const VkExtent2D extent = engine_->extent();

  if (uiExtent_.width == extent.width && uiExtent_.height == extent.height) {
    return;
  }

  ui_->resize({extent.width, extent.height});
  uiExtent_ = extent;
}

void Game::UpdatePlatform() {
#ifndef __ANDROID__
  window_->Update();
#endif
}

// ---------------------------------------------------------------------------
// Frame
// ---------------------------------------------------------------------------

#ifndef __ANDROID__

bool Game::Frame() {
  UpdateInput();

  if (!HandleResize()) {
    UpdatePlatform();
    controlState_.Update();
    return true;
  }

#else

bool Game::Frame(const std::unordered_map<int32_t, core::Pointer>& touches) {
  UpdateInput(touches);

  if (!HandleResize()) {
    controlState_.Update();
    return true;
  }

#endif
  const auto now = std::chrono::steady_clock::now();
  dt_ = frameCount_ == 0 ? 0.0f : std::chrono::duration<float>(now - lastFrameTime_).count();
  lastFrameTime_ = now;

  uint32_t imageIndex = 0;

  if (!engine_->beginFrame(imageIndex)) {
    engine_->recreateSwapchain();
    UpdatePlatform();
    controlState_.Update();
    return true;
  }

  UpdateUiSize();

  VkCommandBuffer cmd = engine_->getCommandBuffer().handle();
  uint32_t frame = engine_->getCurrentFrameIndex();

  if (ui_) {
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
  
  render_->CollectEntities(frame);

  render_->UpdateParticles(dt_, frame);

  render_->UpdateGameData(frame, player_->camera(), screenW, screenH, session_->enviroment());

  render_->RenderShadowPass(cmd, frame);


  engine_->BeginRenderPass(imageIndex, 0.53f, 0.81f, 0.92f);
  SetViewportAndScissor(cmd);

  render_->Render(cmd, dt_, frame, player_->camera());

  if (ui_) {
    ui_->Render(cmd);
  }

  crosshairPipeline_->Bind(cmd);
  engine_->getDevice().dispatchTable().vkCmdDraw(cmd, 4, 1, 0, 0);

  engine_->endRenderPass();
  engine_->endFrame(imageIndex);

  ++frameCount_;
  ++fpsFrames_;

  controlState_.Update();
  UpdatePlatform();

  return true;
}