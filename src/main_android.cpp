#include <android/asset_manager.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <optional>

#include "Engine.h"
#include "core/Camera.h"
#include "core/CameraShake.h"
#include "core/InputAndroid.h"
#include "core/PathPrefixes.h"
#include "game/LibControl.h"
#include "game/World.h"
#include "game/entity/EntityFactory.h"
#include "game/lighting/Lighting.h"
#include "game/physics/PhysicsSystem.h"
#include "gfx/block/PreviewRenderer.h"
#include "gfx/entity/EntityRenderSystem.h"
#include "gfx/render/chunk/ChunkRenderer.h"
#include "gfx/render/particle/ParticleEngine.h"
#include "gfx/ui/LibGui.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "VoxelGore", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "VoxelGore", __VA_ARGS__)

extern AAssetManager* g_AAssetManager;

constexpr uint32_t WORLD_WIDTH = 3;
constexpr uint32_t WORLD_HEIGHT = 2;
constexpr uint32_t WORLD_DEPTH = 3;

// ===== Статус-файлы для отладки без логката =====
static void writeStatusFile(const char* filename, const char* content) {
  std::string path = std::string("/sdcard/Android/data/com.voxelgore/files/") + filename;
  FILE* f = fopen(path.c_str(), "w");
  if (f) {
    fputs(content, f);
    fclose(f);
  } else {
    LOGE("Не смог открыть файл статуса: %s (errno: %d, %s)", path.c_str(), errno, strerror(errno));
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

class GameContext {
 public:
  static constexpr float EYE_HEIGHT = 1.7f;
  static constexpr float JUMP_VELOCITY = 7.953f;
  static constexpr float REACH_DISTANCE = 6.0f;

  explicit GameContext(android_app* app) : engine(std::make_unique<Engine>(app)) {
    const vkcore::Device& device = engine->getDevice();
    vkcore::MemoryAllocator& memoryAllocator = engine->getMemoryAllocator();

    // ===== ПРИЦЕЛ =====
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

    // ===== МИР =====
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

    LOGI("World built: %zu chunks", world->chunks().getChunks().size());

    // ===== КОЛЛИЗИИ =====
    collisionResolver = std::make_unique<gm::CollisionResolver>(world->chunks(), *blockManager);
    physicsSystem = std::make_unique<gm::PhysicsSystem>(*collisionResolver);
    healthSystem = std::make_unique<gm::HealthSystem>();
    entityRenderSystem = std::make_unique<gfx::EntityRenderSystem>(*modelCache, *billboardsAtlas);

    glm::vec3 worldCenter(WORLD_WIDTH * static_cast<float>(gm::Chunk::kLength) * 0.5f,
                          WORLD_HEIGHT * static_cast<float>(gm::Chunk::kLength) * 0.5f,
                          WORLD_DEPTH * static_cast<float>(gm::Chunk::kLength) * 0.5f);

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
    ui->push(core::kAssetsPrefix + "ui/game_android.xml");

    LOGI("UI pushed: ui/game_android.xml");

    startTime = std::chrono::high_resolution_clock::now();
    lastFrameTime = startTime;
    lastFpsTime = startTime;

    // ===== PLAYER =====
    playerEntity = world->CreateEntity();
    world->components().Storage<gm::HitboxComponent>().Add(
        playerEntity.id, gm::HitboxComponent{
                             .pos = worldCenter + glm::vec3(16.0f, 70.0f, 16.0f),
                             .size = glm::vec3(0.6f, 1.8f, 0.6f),
                             .vel = glm::vec3(0.0f),
                         });

    entityDefs->Register(core::kAssetsPrefix + "entities/barrel.json");
    entityDefs->Register(core::kAssetsPrefix + "entities/integrity.json");

    barrelEntity = entityFactory->Create("barrel", worldCenter + glm::vec3(4.0f));
    integrityEntity = entityFactory->Create("integrity", glm::vec3(20.0f));

    camera.lookAt(GetPlayerHitbox().pos, worldCenter);
  }

  ~GameContext() {
    if (engine) engine->getGraphicsQueue().waitIdle();
  }

  GameContext(const GameContext&) = delete;
  GameContext& operator=(const GameContext&) = delete;

  gm::HitboxComponent& GetPlayerHitbox() {
    return *world->components().Storage<gm::HitboxComponent>().Get(playerEntity.id);
  }

  const gm::HitboxComponent& GetPlayerHitbox() const {
    return *world->components().Storage<gm::HitboxComponent>().Get(playerEntity.id);
  }

  glm::vec3 cameraForward() const {
    float yaw = glm::radians(camera.yaw);
    float pitch = glm::radians(camera.pitch);
    return glm::normalize(glm::vec3(cosf(pitch) * cosf(yaw), sinf(pitch), cosf(pitch) * sinf(yaw)));
  }

  void modifyVoxel(const glm::ivec3& worldPos, uint16_t voxelId) {
    gm::Voxel voxel{};
    voxel.id = voxelId;
    world->chunks().setVoxel(worldPos, voxel);
    lighting->onVoxelSetted(worldPos, {voxelId});
  }

  void tryBreakBlock() {
    auto hit = collisionResolver->Raycast(camera.position, cameraForward(), world->components(),
                                          REACH_DISTANCE);
    if (!hit) return;

    if (hit->entity.id != gm::kInvalidEntityId) {
      auto* hitbox = world->components().Storage<gm::HitboxComponent>().Get(hit->entity.id);
      auto* health = world->components().Storage<gm::HealthComponent>().Get(hit->entity.id);
      if (!hitbox) return;
      if (health) health->Damage(10.0f);

      glm::vec3 dir = hitbox->pos - GetPlayerHitbox().pos;
      dir.y = 0.0f;
      if (glm::length2(dir) < 0.0001f) {
        dir = cameraForward();
        dir.y = 0.0f;
      }
      dir = glm::normalize(dir);

      constexpr float KNOCKBACK = 10.0f;
      hitbox->vel.x = dir.x * KNOCKBACK;
      hitbox->vel.z = dir.z * KNOCKBACK;
      hitbox->vel.y = glm::max(hitbox->vel.y, 1.8f);
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
    if (!hit) return;

    const glm::ivec3 placePos = hit->ipos + glm::ivec3(hit->normal);
    if (!collisionResolver->CanPlaceBlock(placePos, world->components())) return;

    modifyVoxel(placePos, id);
  }

  bool renderFrame(const std::unordered_map<int32_t, core::Pointer>& touches) {
    uint32_t imageIndex = 0;
    if (!engine->beginFrame(imageIndex)) {
      LOGI("Swapchain out of date");
      return false;
    }

    const vkcore::Device& device = engine->getDevice();
    const vkcore::CommandBuffer& commandBuffer = engine->getCommandBuffer();
    VkCommandBuffer cmd = commandBuffer.handle();
    uint32_t frameIndex = engine->getCurrentFrameIndex();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(currentTime - lastFrameTime).count();
    lastFrameTime = currentTime;
    if (frameCount == 0) dt = 0.0f;

    fpsFrames_++;
    float fpsElapsed = std::chrono::duration<float>(currentTime - lastFpsTime).count();
    if (fpsElapsed >= 0.5f) {
      LOGI("FPS: %.1f", fpsFrames_ / fpsElapsed);
      fpsFrames_ = 0;
      lastFpsTime = currentTime;
    }

    float screenW = static_cast<float>(engine->extent().width);
    float screenH = static_cast<float>(engine->extent().height);

    // ===== INPUT =====
    constexpr float kLookSensitivity = 1.0f;
    gfx::ui::CameraSwipe swipe = ui->routeTouches(touches);
    ui->update();
    camera.rotate(swipe.deltaX * kLookSensitivity, swipe.deltaY * kLookSensitivity);

    // ===== MOVEMENT =====
    constexpr float MOVE_SPEED = 6.0f;
    float forwardAxis = static_cast<float>(controlState.move.z);
    float rightAxis = static_cast<float>(controlState.move.x);
    glm::vec3 flatForward(cosf(glm::radians(camera.yaw)), 0.0f, sinf(glm::radians(camera.yaw)));
    glm::vec3 flatRight = glm::normalize(glm::cross(flatForward, camera.up));
    glm::vec3 moveDir = flatForward * forwardAxis + flatRight * rightAxis;
    const float moveLength2 = glm::length2(moveDir);

    gm::HitboxComponent& playerHitbox = GetPlayerHitbox();
    if (moveLength2 > 0.0001f) {
      moveDir /= glm::sqrt(moveLength2);
      playerHitbox.vel.x = moveDir.x * MOVE_SPEED;
      playerHitbox.vel.z = moveDir.z * MOVE_SPEED;
    }
    if (controlState.jump && playerHitbox.grounded) playerHitbox.vel.y = JUMP_VELOCITY;

    // ===== ACTIONS =====
    if (controlState.breakBlock) tryBreakBlock();
    if (controlState.interact) tryPlaceBlock(controlState.hotbarSlot);

    // ===== WORLD UPDATE =====
    chunkRenderer->updateDirty(cmd, frameIndex);
    healthSystem->Update(world->components(), *world, dt);
    physicsSystem->Update(world->components(), dt);

    // ===== CAMERA =====
    cameraShake.update(dt, playerHitbox.vel * glm::vec3(1.0f, 0.0f, 1.0f), playerHitbox.grounded);
    camera.position = playerHitbox.pos + glm::vec3(0.0f, EYE_HEIGHT, 0.0f) * 0.5f +
                      cameraShake.getPositionOffset();

    // ===== MATRICES =====
    glm::mat4 view = camera.getView();
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), screenW / screenH, 0.1f, 500.0f);
    proj[1][1] *= -1.0f;

    gfx::UniformGameData gameData{};
    gameData.proj = proj;
    gameData.view = view;
    gameData.projView = proj * view;
    gameData.cameraPos = camera.position;
    gameData.cameraDir = cameraForward();
    gameDataBinding->update(frameIndex, gameData);

    // ===== PARTICLES =====
    particleEngine->Update(dt, frameIndex);

    // ===== ENTITY RENDERING =====
    modelRenderer->BeginFrame(frameIndex);
    entityRenderSystem->Render(world->components(), *modelRenderer, *generalBucket, *world,
                               frameIndex);

    // ===== BEGIN RENDER PASS =====
    gameDataBinding->bind(cmd, chunkRenderer->pipelineLayout().handle(), frameIndex);
    engine->beginRenderPass(imageIndex, 0.53f, 0.81f, 0.92f);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = screenW;
    viewport.height = screenH;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    device.dispatchTable().vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = engine->extent();
    device.dispatchTable().vkCmdSetScissor(cmd, 0, 1, &scissor);

    // ===== SOLID / CUTOUT =====
    chunkRenderer->Render(cmd, dt, frameIndex, camera.position, gfx::RenderLayer::Solid);
    chunkRenderer->Render(cmd, dt, frameIndex, camera.position, gfx::RenderLayer::Cutout);
    billboardRenderer->Render(cmd, camera.position, gfx::RenderLayer::Solid, frameIndex);
    billboardRenderer->Render(cmd, camera.position, gfx::RenderLayer::Cutout, frameIndex);

    // ===== MODELS =====
    modelPipeline->Bind(cmd);
    modelRenderer->Render(cmd);

    // ===== TRANSLUCENT =====
    chunkRenderer->Render(cmd, dt, frameIndex, camera.position, gfx::RenderLayer::Translucent);
    billboardRenderer->Render(cmd, camera.position, gfx::RenderLayer::Translucent, frameIndex);

    // ===== CROSSHAIR =====
    crosshairPipeline->Bind(cmd);
    device.dispatchTable().vkCmdDraw(cmd, 4, 1, 0, 0);

    // ===== UI =====
    ui->render(cmd);

    engine->endRenderPass();
    engine->endFrame(imageIndex);

    ++frameCount;
    controlState.update();
    return true;
  }

  bool IsRenderable() const { return engine->IsRenderable(); }
  void onSurfaceRestored() { engine->restoreSurface(); }
  void onSurfaceDestroyed() { engine->destroySurface(); }

 private:
  std::unique_ptr<Engine> engine;

  std::unique_ptr<vkcore::PipelineLayout> crosshairPipelineLayout;
  std::unique_ptr<vkcore::Pipeline> crosshairPipeline;
  std::unique_ptr<gfx::TextureManager> textureManager;
  std::unique_ptr<gm::World> world;
  std::unique_ptr<gm::EntityDefinitionRegistry> entityDefs;
  std::unique_ptr<gm::EntityFactory> entityFactory;
  std::unique_ptr<gm::BlockManager> blockManager;

  std::unique_ptr<gfx::GameDataBinding> gameDataBinding;
  std::unique_ptr<gfx::ChunkRenderer> chunkRenderer;

  std::unique_ptr<gfx::Atlas> billboardsAtlas;
  std::unique_ptr<gfx::BillboardRenderer> billboardRenderer;
  gfx::BillboardRenderBucket* generalBucket = nullptr;
  gfx::BillboardRenderBucket* blockBucket = nullptr;

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

  script::LuaState luaState;
  std::optional<gfx::ui::Ui> ui;
  std::optional<gfx::ui::LibGui> libGui;

  gm::ControlState controlState;
  std::optional<gm::LibControl> libControl;

  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point lastFrameTime;
  std::chrono::high_resolution_clock::time_point lastFpsTime;

  int frameCount = 0;
  int fpsFrames_ = 0;

  gm::Entity playerEntity;
  gm::Entity barrelEntity;
  gm::Entity integrityEntity;
};

static std::unique_ptr<GameContext> g_ctx;
static core::InputAndroid g_input;

static void handleAppCmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (app->window != nullptr) {
        if (!g_ctx) {
          try {
            g_ctx = std::make_unique<GameContext>(app);
            writeStatusFile("status_ok.txt", "OK: GameContext created successfully\n");
          } catch (const vkcore::SystemError& e) {
            char buf[512];
            snprintf(buf, sizeof(buf), "SYSTEM ERROR: %s, code: %d\n", e.what(), e.code().value());
            LOGE("System error: %s, code: %d", e.what(), e.code().value());
            writeStatusFile("status_exception.txt", buf);
            g_ctx.reset();
          } catch (const std::exception& e) {
            char buf[512];
            snprintf(buf, sizeof(buf), "EXCEPTION: %s\n", e.what());
            LOGE("Exception: %s", e.what());
            writeStatusFile("status_exception.txt", buf);
            g_ctx.reset();
          }
        } else {
          g_ctx->onSurfaceRestored();
        }
      }
      break;

    case APP_CMD_TERM_WINDOW:
      g_ctx->onSurfaceDestroyed();
      break;
    case APP_CMD_DESTROY:
      g_ctx.reset();
      break;

    default:
      break;
  }
}

static int32_t handleInputEvent(android_app* app, AInputEvent* event) {
  if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION) return 0;
  g_input.Update(event);
  return 1;
}

AAssetManager* g_AAssetManager = nullptr;

void android_main(android_app* app) {
  app->onAppCmd = handleAppCmd;
  app->onInputEvent = handleInputEvent;
  g_AAssetManager = app->activity->assetManager;

  int events;
  android_poll_source* source;

  while (!app->destroyRequested) {
    g_input.Reset();
    while (ALooper_pollOnce(g_ctx ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
      if (source != nullptr) source->process(app, source);
      if (app->destroyRequested) break;
    }

    if (app->destroyRequested) break;

    if (g_ctx && g_ctx->IsRenderable()) {
      bool ok = true;
      try {
        ok = g_ctx->renderFrame(g_input.inputState().pointers());
      } catch (const vkcore::SystemError& e) {
        LOGE("Vulkan system error: %s, code: %d", e.what(), e.code().value());
        ok = false;
      } catch (const std::exception& e) {
        LOGE("Exception: %s", e.what());
        ok = false;
      }
      if (!ok) g_ctx.reset();
    }
  }

  g_ctx.reset();
  LOGI("Goodbye!");
}