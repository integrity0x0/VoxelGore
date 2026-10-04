#include "RenderWorld.h"

#include "Engine.h"
#include "core/PathPrefixes.h"
#include "WorldSession.h"

namespace gfx {

RenderWorld::RenderWorld(Engine& engine, gm::WorldSession& session,
                         script::LuaState& luaState, const std::string& assetsPrefix)
    : engine_(&engine), session_(&session) {
  const auto& device = engine.getDevice();
  auto& memoryAllocator = engine.memoryAllocator();

  shaderCompiler_ = std::make_unique<ShaderCompiler>(assetsPrefix);

  gameDataBinding_ = std::make_unique<GameDataBinding>(device, engine.GetBufferAllocator(),
                                                       engine.GetFramesCount());

  textureManager_ =
      std::make_unique<TextureManager>(device, engine.GetTransferContext(), memoryAllocator);

  shadowCtxt_ = std::make_unique<ShadowContext>(device, memoryAllocator);
  shaderCompiler_->AddDefinition("SHADOWS_ENABLED", "1");

  skyboxRenderer_ =
      std::make_unique<SkyboxRenderer>(device, engine.GetBufferAllocator(), engine.GetTransferContext(),
                                       engine.GetRenderPass(), *gameDataBinding_, *shaderCompiler_);

  std::array<std::string, 6> nightPaths = {
      assetsPrefix + "skybox/night/right.png", assetsPrefix + "skybox/night/left.png",
      assetsPrefix + "skybox/night/top.png",   assetsPrefix + "skybox/night/bottom.png",
      assetsPrefix + "skybox/night/front.png", assetsPrefix + "skybox/night/back.png",
  };

  nightSkybox_ =
      std::make_unique<Skybox>(Skybox::Load(device, engine.GetTransferContext(), memoryAllocator,
                                            skyboxRenderer_->descriptorPool(),
                                            skyboxRenderer_->GetDescriptorSetLayout(), nightPaths, 4u)
                                   .value());

  modelCache_ =
      std::make_unique<ModelCache>(device, engine.GetTransferContext(), engine.GetBufferAllocator(),
                                   *textureManager_, engine.GetFramesCount());

  modelRenderer_ = std::make_unique<ModelRenderer>(
      device, engine.GetBufferAllocator(), engine.GetRenderPass(), *gameDataBinding_,
      modelCache_->materialSetLayout(), shadowCtxt_.get(),
      *shaderCompiler_,
      engine.GetFramesCount());

  chunkRenderer_ = std::make_unique<ChunkRenderer>(
      device, engine.GetTransferContext(), engine.getGraphicsQueue(), memoryAllocator,
      engine.GetRenderPass(), *gameDataBinding_, shadowCtxt_.get(), *shaderCompiler_,
      session.GetWorld().GetChunks(), session.GetLighting(), session.GetBlocks(),
      engine.GetFramesCount());

  billboardRenderer_ = std::make_unique<BillboardRenderer>(device, engine.GetRenderPass(),
                                                           *gameDataBinding_, *shaderCompiler_);

  billboardsAtlas_ = std::make_unique<Atlas>(device, engine.GetTransferContext(), memoryAllocator,
                                             glm::ivec2{4096, 4096}, 4);
  if (!billboardsAtlas_->Load(kBlankTexturePath, kBlankTextureKey))
    throw std::runtime_error("Unable to load blank texture: " + kBlankTexturePath);

  generalBucket_ = &billboardRenderer_->CreateBucket(engine.GetBufferAllocator(), *billboardsAtlas_,
                                                     engine.GetFramesCount());
  blockBucket_ = &billboardRenderer_->CreateBucket(
      engine.GetBufferAllocator(), chunkRenderer_->GetAtlas(), engine.GetFramesCount());

  particleEngine_ = std::make_unique<ParticleEngine>(
      *billboardsAtlas_, chunkRenderer_->GetBlockRenderData(), session.GetBlocks(),
      session.GetWorld().GetChunks(), session.GetLighting(), session.GetEnviroment(), *generalBucket_,
      *blockBucket_);

  entityRenderSystem_ = std::make_unique<EntityRenderSystem>(*modelCache_, *billboardsAtlas_);
  materialManager_ = std::make_unique<MaterialManager>(device, *textureManager_);
  ui_ =
      std::make_unique<Ui>(device, engine.GetBufferAllocator(), engine.GetRenderPass(),
                           *shaderCompiler_, luaState, *materialManager_, engine.GetFramesCount());
  ui_->Resize(engine.extent());
  ui_->Push(core::kAssetsPrefix + "ui/game.xml");
}

void RenderWorld::UpdateDirty(VkCommandBuffer cmd, uint32_t frameIndex) {
  chunkRenderer_->UpdateDirty(cmd, frameIndex);
}

void RenderWorld::UpdateParticles(float dt, uint32_t frameIndex) {
  particleEngine_->Update(dt, frameIndex);
}

void RenderWorld::UpdateGameData(uint32_t frameIndex, const core::Camera& camera, float screenW,
                                 float screenH, const gm::Enviroment& enviroment) {
  glm::mat4 view = camera.view();
  glm::mat4 proj = glm::perspective(glm::radians(45.0f), screenW / screenH, 0.1f, 500.0f);
  proj[1][1] *= -1;

  const glm::vec3 sunDir = glm::normalize(glm::vec3(-0.6f, -0.35f, -0.5f));

  shadowCtxt_->UpdateLightMatrix(sunDir, camera.pos());

  UniformGameData data = {};
  data.proj = proj;
  data.view = view;
  data.projView = proj * view;
  data.cameraPos = camera.pos();
  data.cameraDir = camera.forward();
  data.ambientColor = enviroment.GetColor();  // glm::vec3(0.05f, 0.065f, 0.12f);
  data.fogDensity = 0.015f;
  data.lightProjView = shadowCtxt_->lightViewProj();
  data.lightDir = shadowCtxt_->lightDir();
  gameDataBinding_->Update(frameIndex, data);
}

void RenderWorld::Render(VkCommandBuffer cmd, float dt, uint32_t frameIndex,
                         const core::Camera& camera) {
  gameDataBinding_->Bind(cmd, frameIndex);

  chunkRenderer_->Render(cmd, dt, frameIndex, camera.pos(), RenderLayer::Solid);
  chunkRenderer_->Render(cmd, dt, frameIndex, camera.pos(), RenderLayer::Cutout);

  billboardRenderer_->Render(cmd, camera.pos(), RenderLayer::Solid, frameIndex);
  billboardRenderer_->Render(cmd, camera.pos(), RenderLayer::Cutout, frameIndex);

  modelRenderer_->Render(cmd, frameIndex);

  skyboxRenderer_->BindPipeline(cmd);
  skyboxRenderer_->Draw(cmd, *nightSkybox_);

  chunkRenderer_->Render(cmd, dt, frameIndex, camera.pos(), RenderLayer::Translucent);
  billboardRenderer_->Render(cmd, camera.pos(), RenderLayer::Translucent, frameIndex);

  ui_->Render(cmd, frameIndex);
}

void RenderWorld::RenderShadowPass(VkCommandBuffer cmd, uint32_t frameIndex) {
  gameDataBinding_->Bind(cmd, frameIndex);
  if (shadowCtxt_) {
    shadowCtxt_->Begin(cmd);
    modelRenderer_->RenderShadow(cmd, frameIndex);
    chunkRenderer_->RenderShadow(cmd, frameIndex);
    shadowCtxt_->End(cmd);
  }
}

void RenderWorld::CollectEntities(uint32_t frameIndex) {
  entityRenderSystem_->Render(session_->components(), *modelRenderer_, *particleEngine_, *generalBucket_,
                              session_->GetWorld(), session_->GetLighting(), session_->GetEnviroment(),
                              frameIndex);
  modelRenderer_->UploadInstances(frameIndex);
}

}  // namespace gfx