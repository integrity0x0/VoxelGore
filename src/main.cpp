#include <chrono>
#include <iostream>
#include <thread>

#include "Engine.h"
#include "core/Camera.h"
#include "core/CameraShake.h"
#include "core/PathPrefixes.h"
#include "core/Window.h"
#include "game/LibControl.h"
#include "game/entity/EntityFactory.h"
#include "game/lighting/Lighting.h"
#include "game/physics/PhysicsSystem.h"
#include "gfx/block/PreviewRenderer.h"
#include "gfx/entity/EntityRenderSystem.h"
#include "gfx/render/billboard/BillboardRenderer.h"
#include "gfx/render/chunk/ChunkRenderer.h"
#include "gfx/render/particle/ParticleEngine.h"
#include "gfx/ui/LibGui.h"
#include "gfx/texture/Skybox.h"
#include "gfx/render/skybox/SkyboxRenderer.h"

constexpr uint32_t WORLD_WIDTH = 3;
constexpr uint32_t WORLD_HEIGHT = 2;
constexpr uint32_t WORLD_DEPTH = 3;

static bool cursorLocked = true;

static std::atomic<bool> join = false;

static void framePrinter(float* dt) {
  while (!join) {
    std::cout << "FPS: " << 1.0f / *(dt) << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

static std::optional<uint32_t> ParseBlockIdFromPreviewSuffix(std::string_view remainder) {
  auto dot = remainder.find('.');
  if (dot == std::string_view::npos) return std::nullopt;
  if (remainder.substr(dot + 1) != "preview") return std::nullopt;

  uint32_t id;
  auto [ptr, ec] = std::from_chars(remainder.data(), remainder.data() + dot, id);
  return ec == std::errc{} ? std::optional(id) : std::nullopt;
}

std::thread th;

class GameContext {
 public:
  static constexpr float EYE_HEIGHT = 1.7f;
  static constexpr float JUMP_VELOCITY = 7.953f;
  static constexpr float REACH_DISTANCE = 6.0f;

  explicit GameContext(std::unique_ptr<core::Window> window)
      : window_(std::move(window)), dt(0.0f) {
    if (!window_) {
      throw std::runtime_error("GameContext: window is null");
    }

    th = std::thread(framePrinter, &dt);

    engine = std::make_unique<Engine>(window_->getWindow());
    const vkcore::Device& device = engine->getDevice();
    vkcore::MemoryAllocator& memoryAllocator = engine->getMemoryAllocator();

    // ===== ПАЙПЛАЙН ПРИЦЕЛА =====
    crosshairPipelineLayout = std::make_unique<vkcore::PipelineLayout>(
        device, std::vector<const vkcore::DescriptorSetLayout*>{},
        std::vector<VkPushConstantRange>{});

    crosshairPipeline = std::make_unique<vkcore::Pipeline>(
        vkcore::GraphicsPipelineCreator(device)
            .AddShaderStage(core::kAssetsPrefix + "shaders/crosshair.vert.spv",
                            VK_SHADER_STAGE_VERTEX_BIT)
            .AddShaderStage(core::kAssetsPrefix + "shaders/crosshair.frag.spv",
                            VK_SHADER_STAGE_FRAGMENT_BIT)
            .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
            .AddColorBlendAttachment(false)
            .AddDynamicState(VK_DYNAMIC_STATE_VIEWPORT)
            .AddDynamicState(VK_DYNAMIC_STATE_SCISSOR)
            .setDepthTest(false, false)
            .setCullMode(VK_CULL_MODE_NONE)
            .Build(crosshairPipelineLayout->handle(), engine->getRenderPass().handle()));
    // ==================================================================================

    // ===== СОЗДАНИЕ МИРА =====

    world = std::make_unique<gm::World>(WORLD_WIDTH, WORLD_HEIGHT, WORLD_DEPTH);

    entityDefs = std::make_unique<gm::EntityDefinitionRegistry>();
    entityFactory =
        std::make_unique<gm::EntityFactory>(world->entities(), world->components(), *entityDefs);

    blockManager = std::make_unique<gm::BlockManager>();

    blockManager->Load(core::kAssetsPrefix + "blocks/air.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/stone.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/red_lamp.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/glass_lime.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/glass_purple.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/ice.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/lava.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/sand.json");
    blockManager->Load(core::kAssetsPrefix + "blocks/tnt.json");
    lighting = std::make_unique<gm::Lighting>(world->chunks(), *blockManager);
    lighting->lightUp();

    gameDataBinding = std::make_unique<gfx::GameDataBinding>(device, engine->getBufferAllocator(),
                                                             engine->getFramesInFlightCount());

    textureManager = std::make_unique<gfx::TextureManager>(device, engine->getTransferContext(),
                                                           engine->getMemoryAllocator());
    //skyboxRenderer = std::make_unique<gfx::SkyboxRenderer>(device, engine->getRenderPass(), *gameDataBinding);
   // daySkybox = std::make_unique<gfx::Skybox>(gfx::Skybox::Load(device, engine->getTransferContext(), memoryAllocator, skyboxRenderer->de);
    //nightSkybox;
    modelPipeline =
        std::make_unique<gfx::ModelPipeline>(device, engine->getRenderPass(), *gameDataBinding);

    modelCache = std::make_unique<gfx::ModelCache>(
        device, engine->getTransferContext(), engine->getBufferAllocator(),
        modelPipeline->pipelineLayout(), modelPipeline->descriptorSetLayout(), *textureManager,
        engine->getFramesInFlightCount());

    modelRenderer = std::make_unique<gfx::ModelRenderer>(
        device, engine->getBufferAllocator(), *modelPipeline, engine->getFramesInFlightCount());

    chunkRenderer = std::make_unique<gfx::ChunkRenderer>(
        device, engine->getTransferContext(), engine->getGraphicsQueue(), memoryAllocator,
        engine->getRenderPass().handle(), *gameDataBinding, world->chunks(), *blockManager,
        engine->getFramesInFlightCount());

    billboardsAtlas = std::make_unique<gfx::Atlas>(device, engine->getTransferContext(),
                                                   memoryAllocator, glm::ivec2{4096, 4096}, 4u);

    billboardRenderer =
        std::make_unique<gfx::BillboardRenderer>(device, engine->getRenderPass(), *gameDataBinding);

    generalBucket = &billboardRenderer->CreateBucket(engine->getBufferAllocator(), *billboardsAtlas,
                                                     engine->getFramesInFlightCount());

    blockBucket = &billboardRenderer->CreateBucket(
        engine->getBufferAllocator(), chunkRenderer->getAtlas(), engine->getFramesInFlightCount());

    particleEngine = std::make_unique<gfx::ParticleEngine>(
        *billboardsAtlas, chunkRenderer->blockRenderData(), *blockManager, world->chunks(),
        *generalBucket, *blockBucket);

    std::cout << "World built: " << world->chunks().getChunks().size() << " chunks" << std::endl;
    // ================================================================

    collisionResolver = std::make_unique<gm::CollisionResolver>(world->chunks(), *blockManager);

    physicsSystem = std::make_unique<gm::PhysicsSystem>(*collisionResolver);

    healthSystem = std::make_unique<gm::HealthSystem>();

    entityRenderSystem = std::make_unique<gfx::EntityRenderSystem>(*modelCache, *billboardsAtlas);

    glm::vec3 worldCenter((WORLD_WIDTH * (float)gm::Chunk::kLength) * 0.5f,
                          (WORLD_HEIGHT * (float)gm::Chunk::kLength) * 0.5f,
                          (WORLD_DEPTH * (float)gm::Chunk::kLength) * 0.5f);

    // ================================================

    blockPreviewRenderer = std::make_unique<gfx::block::PreviewRenderer>(
        device, engine->getCommandPool(), engine->getGraphicsQueue(), engine->getMemoryAllocator(),
        chunkRenderer->blockRenderData());

    // ===== UI + CONTROL =====
    std::ignore =
        textureManager->Load(core::kAssetsPrefix + std::string("images/blank.png"), "blank");

    textureManager->AddResolver(
        "blocks.", [this](std::string_view remainder) -> std::optional<vkcore::SampledTexture> {
          auto id = ParseBlockIdFromPreviewSuffix(remainder);
          if (!id) return std::nullopt;
          return blockPreviewRenderer->Render(*id);
        });

    ui.emplace(device, engine->getCommandPool(), engine->getRenderPass().handle(), luaState,
               *textureManager, engine->extent());
    libGui.emplace(*ui, luaState);
    libControl.emplace(controlState, luaState);
    ui->push(core::kAssetsPrefix + "ui/game.xml");

    std::cout << "[INFO] UI pushed: ui/game.xml" << std::endl;
    // ========================
    glfwSetInputMode(window_->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;
    lastFpsTime = startTime;

    entityDefs->Register(core::kAssetsPrefix + "entities/barrel.json");
    entityDefs->Register(core::kAssetsPrefix + "entities/integrity.json");

    playerEntity = world->CreateEntity();

    world->components().Storage<gm::HitboxComponent>().Add(
        playerEntity.id, gm::HitboxComponent{
                             .pos = worldCenter + glm::vec3(16.0f, 70.0f, 16.0f),
                             .size = glm::vec3(0.6f, 1.8f, 0.6f),
                             .vel = glm::vec3(0.0f),
                         });

    barrelEntity = entityFactory->Create("barrel", worldCenter + glm::vec3(4.0f));
    integrityEntity = entityFactory->Create("integrity", glm::vec3(20.0f));

    camera.lookAt(GetPlayerHitbox().pos, worldCenter);
  }

  ~GameContext() {
    if (engine) {
      engine->getGraphicsQueue().waitIdle();
    }
  }

  GameContext(const GameContext&) = delete;
  GameContext& operator=(const GameContext&) = delete;
  GameContext(GameContext&& other) noexcept = default;
  GameContext& operator=(GameContext&& other) noexcept = default;

  [[nodiscard]] core::Window& getWindow() { return *window_; }
  [[nodiscard]] const core::Window& getWindow() const { return *window_; }
  gm::HitboxComponent& GetPlayerHitbox() {
    return *world->components().Storage<gm::HitboxComponent>().Get(playerEntity.id);
  }

  const gm::HitboxComponent& GetPlayerHitbox() const {
    return *world->components().Storage<gm::HitboxComponent>().Get(playerEntity.id);
  }

  void SetViewportAndScissor(VkCommandBuffer cmd) {
    VkViewport viewports[] = {{.x = 0.0f,
                               .y = 0.0f,
                               .width = static_cast<float>(engine->extent().width),
                               .height = static_cast<float>(engine->extent().height),
                               .maxDepth = 1.0f}};

    VkRect2D scissors[] = {{.offset = {}, .extent = engine->extent()}};

    engine->getDevice().dispatchTable().vkCmdSetViewport(cmd, 0u, 1u, viewports);
    engine->getDevice().dispatchTable().vkCmdSetScissor(cmd, 0u, 1u, scissors);
  }

  glm::vec3 cameraForward() const {
    float yaw = glm::radians(camera.yaw);
    float pitch = glm::radians(camera.pitch);
    return glm::normalize(glm::vec3(cosf(pitch) * cosf(yaw), sinf(pitch), cosf(pitch) * sinf(yaw)));
  }

  static float msSince(std::chrono::high_resolution_clock::time_point t0) {
    return std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - t0)
        .count();
  }

  void modifyVoxel(const glm::ivec3& worldPos, uint16_t voxelId) {
    auto t0 = std::chrono::high_resolution_clock::now();

    gm::Voxel v{};
    v.id = voxelId;
    world->chunks().setVoxel(worldPos, v);

    auto t1 = std::chrono::high_resolution_clock::now();

    lighting->onVoxelSetted(worldPos, {voxelId});

    auto t2 = std::chrono::high_resolution_clock::now();

    lastSetVoxelMs = std::chrono::duration<float, std::milli>(t1 - t0).count();
    lastLightingMs = std::chrono::duration<float, std::milli>(t2 - t1).count();
  }

  void tryBreakBlock() {
    auto hit = collisionResolver->Raycast(camera.position, cameraForward(), world->components(),
                                          REACH_DISTANCE);
    if (!hit) return;

    if (hit->entity.id != gm::kInvalidEntityId) {
      auto* hb = world->components().Storage<gm::HitboxComponent>().Get(hit->entity.id);

      auto* hl = world->components().Storage<gm::HealthComponent>().Get(hit->entity.id);

      if (!hb) return;

      if (hl) {
        hl->Damage(10.0f);
      }

      glm::vec3 dir = hb->pos - GetPlayerHitbox().pos;
      dir.y = 0.0f;

      if (glm::length2(dir) < 0.0001f) {
        dir = cameraForward();
        dir.y = 0.0f;
      }

      dir = glm::normalize(dir);

      constexpr float KNOCKBACK = 10.0f;

      hb->vel.x = dir.x * KNOCKBACK;
      hb->vel.z = dir.z * KNOCKBACK;
      hb->vel.y = glm::max(hb->vel.y, 1.8f);

      return;
    }

    auto voxel = world->chunks().getVoxel(hit->ipos);
    if (!voxel) return;

    particleEngine->SpawnBlockDebris(voxel->id, glm::vec3(hit->ipos) + glm::vec3(0.5f));
    if (voxel->id == 8) particleEngine->SpawnExplosion(hit->pos);

    modifyVoxel(hit->ipos, 0);
  }

  void tryPlaceBlock(uint32_t id) {
    auto hit = collisionResolver->Raycast(camera.position, cameraForward(), world->components(),
                                          REACH_DISTANCE);
    if (!hit) {
      return;
    }
    const glm::ivec3 placePos = hit->ipos + glm::ivec3(hit->normal);
    if (!collisionResolver->CanPlaceBlock(placePos, world->components())) {
      return;
    }
    modifyVoxel(placePos, id);
  }

  void handleInput() {
    if (!window_) return;

    auto& input = window_->getInput();
    const auto& state = input.getState();

    controlState.move.z = 0;
    controlState.move.x = 0;
    if (state.Down(GLFW_KEY_W)) controlState.move.z = 1;
    if (state.Down(GLFW_KEY_S)) controlState.move.z = -1;
    if (state.Down(GLFW_KEY_A)) controlState.move.x = -1;
    if (state.Down(GLFW_KEY_D)) controlState.move.x = 1;

    controlState.jump = state.Down(GLFW_KEY_SPACE);
    controlState.breakBlock = state.MousePressed(GLFW_MOUSE_BUTTON_LEFT);
    controlState.interact = state.MousePressed(GLFW_MOUSE_BUTTON_RIGHT);

    for (int i = 0; i < 8; ++i) {
      if (state.pressed(GLFW_KEY_1 + i)) {
        controlState.hotbarSlot = i + 1;
      }
    }

    if (state.pressed(GLFW_KEY_ESCAPE)) {
      if (glfwGetKey(window_->getWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        cursorLocked = !cursorLocked;
        glfwSetInputMode(window_->getWindow(), GLFW_CURSOR,
                         cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
      }
    }
  }

  bool renderFrame() {
    handleInput();

    uint32_t imageIndex = 0;

    if (!engine->beginFrame(imageIndex)) {
      engine->recreateSwapchain();
      return true;
    }

    const vkcore::CommandBuffer& commandBuffer = engine->getCommandBuffer();
    VkCommandBuffer cmd = commandBuffer.handle();

    uint32_t frameIndex = engine->getCurrentFrameIndex();

    auto currentTime = std::chrono::high_resolution_clock::now();

    dt = std::chrono::duration<float>(currentTime - lastFrameTime).count();

    lastFrameTime = currentTime;

    if (frameCount == 0) {
      dt = 0.0f;
    }

    float screenW = static_cast<float>(engine->extent().width);
    float screenH = static_cast<float>(engine->extent().height);

    gfx::ui::CameraSwipe swipe = ui->routeTouches(window_->getInput().getState().pointers());

    if (window_->isResized()) {
      ui->resize({
          window_->width(),
          window_->height(),
      });
    }

    ui->Update();

    !cursorLocked ? camera.rotate(swipe.deltaX, swipe.deltaY)
                  : camera.rotate(window_->getInput().getState().cursorDeltaX(),
                                  window_->getInput().getState().cursorDeltaY());

    float forwardAxis = static_cast<float>(controlState.move.z);
    float rightAxis = static_cast<float>(controlState.move.x);

    glm::vec3 flatForward(cosf(glm::radians(camera.yaw)), 0.0f, sinf(glm::radians(camera.yaw)));

    glm::vec3 flatRight = glm::normalize(glm::cross(flatForward, camera.up));

    glm::vec3 moveDir = flatForward * static_cast<float>(controlState.move.z) +
                        flatRight * static_cast<float>(controlState.move.x);

    const float moveLength2 = glm::length2(moveDir);

    auto& playerHitbox = GetPlayerHitbox();

    if (moveLength2 > 0.0001f) {
      moveDir /= glm::sqrt(moveLength2);

      constexpr float MOVE_SPEED = 6.0f;

      playerHitbox.vel.x = moveDir.x * MOVE_SPEED;
      playerHitbox.vel.z = moveDir.z * MOVE_SPEED;
    }

    if (controlState.jump && playerHitbox.grounded) {
      playerHitbox.vel.y = JUMP_VELOCITY;
    }

    lastSetVoxelMs = 0.0f;
    lastLightingMs = 0.0f;

    auto tActionStart = std::chrono::high_resolution_clock::now();

    if (controlState.breakBlock) {
      tryBreakBlock();
    }

    if (controlState.interact) {
      tryPlaceBlock(controlState.hotbarSlot);
    }

    float actionMs = msSince(tActionStart);

    auto tDirtyStart = std::chrono::high_resolution_clock::now();

    chunkRenderer->updateDirty(cmd, frameIndex);

    float updateDirtyMs = msSince(tDirtyStart);

    healthSystem->Update(world->components(), *world, dt);

    physicsSystem->Update(world->components(), dt);

    cameraShake.Update(dt, playerHitbox.vel * glm::vec3(1.0f, 0.0f, 1.0f), playerHitbox.grounded);

    camera.position = playerHitbox.pos + glm::vec3(0.0f, EYE_HEIGHT, 0.0f) * 0.5f +
                      cameraShake.getPositionOffset();

    glm::mat4 view = camera.getView();

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), screenW / screenH, 0.1f, 500.0f);

    proj[1][1] *= -1;

    gfx::UniformGameData gameData;

    gameData.proj = proj;
    gameData.view = view;
    gameData.projView = proj * view;
    gameData.cameraPos = camera.position;
    gameData.cameraDir = cameraForward();

    gameDataBinding->Update(frameIndex, gameData);

    particleEngine->Update(dt, frameIndex);

    modelRenderer->BeginFrame(frameIndex);

    entityRenderSystem->Render(world->components(), *modelRenderer, *generalBucket, *world,
                               frameIndex);

    gameDataBinding->Bind(cmd, frameIndex);

    engine->beginRenderPass(imageIndex, 0.53f, 0.81f, 0.92f);

    SetViewportAndScissor(cmd);

    chunkRenderer->Render(cmd, dt, frameIndex, camera.position, gfx::RenderLayer::Solid);
    chunkRenderer->Render(cmd, dt, frameIndex, camera.position, gfx::RenderLayer::Cutout);

    billboardRenderer->Render(cmd, camera.position, gfx::RenderLayer::Solid, frameIndex);
    billboardRenderer->Render(cmd, camera.position, gfx::RenderLayer::Cutout, frameIndex);

    modelPipeline->Bind(cmd);
    modelRenderer->Render(cmd);

    chunkRenderer->Render(cmd, dt, frameIndex, camera.position, gfx::RenderLayer::Translucent);
    billboardRenderer->Render(cmd, camera.position, gfx::RenderLayer::Translucent, frameIndex);

    ui->render(cmd);

    crosshairPipeline->Bind(cmd);

    engine->getDevice().dispatchTable().vkCmdDraw(cmd, 4, 1, 0, 0);

    engine->endRenderPass();
    engine->endFrame(imageIndex);

    ++frameCount;

    controlState.Update();
    window_->Update();

    return true;
  }

 private:
  std::unique_ptr<core::Window> window_;
  std::unique_ptr<Engine> engine;

  std::unique_ptr<vkcore::PipelineLayout> crosshairPipelineLayout;
  std::unique_ptr<vkcore::Pipeline> crosshairPipeline;
  std::unique_ptr<gm::World> world;
  std::unique_ptr<gm::EntityDefinitionRegistry> entityDefs;
  std::unique_ptr<gm::EntityFactory> entityFactory;
  std::unique_ptr<gm::BlockManager> blockManager;
  std::unique_ptr<gfx::GameDataBinding> gameDataBinding;
  std::unique_ptr<gfx::ChunkRenderer> chunkRenderer;
  std::unique_ptr<gfx::Atlas> billboardsAtlas;
  std::unique_ptr<gfx::BillboardRenderer> billboardRenderer;
  gfx::BillboardRenderBucket* generalBucket;
  gfx::BillboardRenderBucket* blockBucket;

  std::unique_ptr<gfx::ParticleEngine> particleEngine;
  std::unique_ptr<gm::Lighting> lighting;
  std::unique_ptr<gm::CollisionResolver> collisionResolver;

  std::unique_ptr<gfx::ModelPipeline> modelPipeline;
  std::unique_ptr<gfx::ModelRenderer> modelRenderer;
  std::unique_ptr<gfx::ModelCache> modelCache;

  std::unique_ptr<gm::PhysicsSystem> physicsSystem;
  std::unique_ptr<gm::HealthSystem> healthSystem;
  std::unique_ptr<gfx::EntityRenderSystem> entityRenderSystem;

  core::Camera camera;
  core::CameraShake cameraShake;
  std::unique_ptr<gfx::block::PreviewRenderer> blockPreviewRenderer;
  std::unique_ptr<gfx::TextureManager> textureManager;
  std::unique_ptr<gfx::SkyboxRenderer> skyboxRenderer;
  std::unique_ptr<gfx::Skybox> daySkybox;
  std::unique_ptr<gfx::Skybox> nightSkybox;
  script::LuaState luaState;
  std::optional<gfx::ui::Ui> ui;
  std::optional<gfx::ui::LibGui> libGui;

  gm::ControlState controlState;
  std::optional<gm::LibControl> libControl;

  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point lastFrameTime;
  std::chrono::high_resolution_clock::time_point lastFpsTime;
  int frameCount = 0;

  float lastSetVoxelMs = 0.0f;
  float lastLightingMs = 0.0f;

  float dt;

  gm::Entity playerEntity;
  gm::Entity barrelEntity;
  gm::Entity integrityEntity;
};

static std::unique_ptr<GameContext> g_ctx;

int main() {
  glfwInit();
  try {
    core::Window::hint(GLFW_RESIZABLE, GLFW_TRUE);
    core::Window::hint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = std::make_unique<core::Window>(1280, 720, "Voxel Game");

    g_ctx = std::make_unique<GameContext>(std::move(window));

    std::cout << "[INFO] Game started successfully!" << std::endl;

    while (g_ctx && !g_ctx->getWindow().shouldClose()) {
      glfwPollEvents();
      if (g_ctx->getWindow().isMinimized()) {
        glfwPollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        continue;
      }

      try {
        g_ctx->renderFrame();
      } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception in render loop: " << e.what() << std::endl;
        break;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] Fatal error: " << e.what() << std::endl;
  }

  join = true;
  th.join();
  g_ctx.reset();
  std::cout << "[INFO] Goodbye!" << std::endl;
  return 0;
}