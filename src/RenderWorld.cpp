#include "RenderWorld.h"

#include "Engine.h"
#include "core/PathPrefixes.h"
#include "WorldSession.h"

namespace gfx {

RenderWorld::RenderWorld(Engine& engine, gm::WorldSession& session, const std::string& assetsPrefix)
    : engine_(&engine), session_(&session) {
  const auto& device = engine.getDevice();
  auto& memoryAllocator = engine.memoryAllocator();

  shaderCompiler_ = std::make_unique<ShaderCompiler>(assetsPrefix);

  gameDataBinding_ = std::make_unique<GameDataBinding>(device, engine.bufferAllocator(),
                                                       engine.getFramesInFlightCount());

  textureManager_ =
      std::make_unique<TextureManager>(device, engine.transferContext(), memoryAllocator);

  skyboxRenderer_ =
      std::make_unique<SkyboxRenderer>(device, engine.bufferAllocator(), engine.transferContext(),
                                       engine.getRenderPass(), *gameDataBinding_, *shaderCompiler_);

  std::array<std::string, 6> nightPaths = {
      assetsPrefix + "skybox/night/right.png", assetsPrefix + "skybox/night/left.png",
      assetsPrefix + "skybox/night/top.png",   assetsPrefix + "skybox/night/bottom.png",
      assetsPrefix + "skybox/night/front.png", assetsPrefix + "skybox/night/back.png",
  };

  nightSkybox_ =
      std::make_unique<Skybox>(Skybox::Load(device, engine.transferContext(), memoryAllocator,
                                            skyboxRenderer_->descriptorPool(),
                                            skyboxRenderer_->descriptorSetLayout(), nightPaths, 4u)
                                   .value());

  modelPipeline_ = std::make_unique<ModelPipeline>(device, engine.getRenderPass(),
                                                   *gameDataBinding_, *shaderCompiler_);
  modelCache_ = std::make_unique<ModelCache>(
      device, engine.transferContext(), engine.bufferAllocator(), modelPipeline_->pipelineLayout(),
      modelPipeline_->descriptorSetLayout(), *textureManager_, engine.getFramesInFlightCount());

  modelRenderer_ = std::make_unique<ModelRenderer>(
      device, engine.bufferAllocator(), *modelPipeline_, engine.getFramesInFlightCount());

  chunkRenderer_ = std::make_unique<ChunkRenderer>(
      device, engine.transferContext(), engine.getGraphicsQueue(), memoryAllocator,
      engine.getRenderPass().handle(), *gameDataBinding_, *shaderCompiler_, session.world().chunks(),
      session.lighting(), session.blocks(), engine.getFramesInFlightCount());

  billboardRenderer_ =
      std::make_unique<BillboardRenderer>(device, engine.getRenderPass(), *gameDataBinding_, *shaderCompiler_);

  billboardsAtlas_ = std::make_unique<Atlas>(device, engine.transferContext(), memoryAllocator,
                                                 glm::ivec2{4096, 4096}, 4u);

  generalBucket_ = &billboardRenderer_->CreateBucket(engine.bufferAllocator(), *billboardsAtlas_,
                                                     engine.getFramesInFlightCount());
  blockBucket_ = &billboardRenderer_->CreateBucket(
      engine.bufferAllocator(), chunkRenderer_->getAtlas(), engine.getFramesInFlightCount());

  particleEngine_ = std::make_unique<ParticleEngine>(
      *billboardsAtlas_, chunkRenderer_->blockRenderData(), session.blocks(),
      session.world().chunks(), *generalBucket_, *blockBucket_);

  entityRenderSystem_ = std::make_unique<EntityRenderSystem>(*modelCache_, *billboardsAtlas_);
}

void RenderWorld::UpdateDirty(VkCommandBuffer cmd, uint32_t frameIndex) {
  chunkRenderer_->updateDirty(cmd, frameIndex);
}

void RenderWorld::UpdateParticles(float dt, uint32_t frameIndex) {
  particleEngine_->Update(dt, frameIndex);
}

void RenderWorld::UpdateGameData(uint32_t frameIndex, const core::Camera& camera, float screenW,
                                 float screenH) {
  glm::mat4 view = camera.GetView();
  glm::mat4 proj = glm::perspective(glm::radians(45.0f), screenW / screenH, 0.1f, 500.0f);
  proj[1][1] *= -1;

  UniformGameData data{};
  data.proj = proj;
  data.view = view;
  data.projView = proj * view;
  data.cameraPos = camera.position;
  data.cameraDir = /* forward from camera */ glm::normalize(
      glm::vec3(cosf(glm::radians(camera.pitch)) * cosf(glm::radians(camera.yaw)),
                sinf(glm::radians(camera.pitch)),
                cosf(glm::radians(camera.pitch)) * sinf(glm::radians(camera.yaw))));
  data.ambientColor = glm::vec3(0.05f, 0.065f, 0.12f);
  data.fogDensity = 0.015f;

  gameDataBinding_->Update(frameIndex, data);
}

void RenderWorld::Render(VkCommandBuffer cmd, float dt, uint32_t frameIndex,
                         const core::Camera& camera) {
  gameDataBinding_->Bind(cmd, frameIndex);

  entityRenderSystem_->Render(session_->components(), *modelRenderer_, *generalBucket_,
                              session_->world(), frameIndex);

  chunkRenderer_->Render(cmd, dt, frameIndex, camera.position, RenderLayer::Solid);
  chunkRenderer_->Render(cmd, dt, frameIndex, camera.position, RenderLayer::Cutout);

  billboardRenderer_->Render(cmd, camera.position, RenderLayer::Solid, frameIndex);
  billboardRenderer_->Render(cmd, camera.position, RenderLayer::Cutout, frameIndex);

  modelPipeline_->Bind(cmd);
  modelRenderer_->Render(cmd, frameIndex);

  skyboxRenderer_->BindPipeline(cmd);
  skyboxRenderer_->Draw(cmd, *nightSkybox_);

  chunkRenderer_->Render(cmd, dt, frameIndex, camera.position, RenderLayer::Translucent);
  billboardRenderer_->Render(cmd, camera.position, RenderLayer::Translucent, frameIndex);
}

}  // namespace gfx