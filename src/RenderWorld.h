#pragma once

#include <array>
#include <memory>
#include <string>

#include "core/Camera.h"
#include "gfx/render/GameDataBinding.h"
#include "gfx/common/mesh/ModelCache.h"
#include "gfx/render/model/ModelPipeline.h"
#include "gfx/render/model/ModelRenderer.h"
#include "gfx/render/entity/EntityRenderSystem.h"
#include "gfx/render/billboard/BillboardRenderer.h"
#include "gfx/render/chunk/ChunkRenderer.h"
#include "gfx/render/particle/ParticleEngine.h"
#include "gfx/render/skybox/SkyboxRenderer.h"
#include "gfx/common/texture/Skybox.h"
#include "gfx/common/texture/TextureManager.h"
#include "gfx/common/shader/ShaderCompiler.h"
#include "gfx/Settings.h"
#include "gfx/render/shadow/ShadowContext.h"

class Engine;

namespace gm {
class WorldSession;
}

namespace gfx {

class RenderWorld {
 public:
  RenderWorld(Engine& engine, gm::WorldSession& session, const std::string& assetsPrefix);

  void UpdateDirty(VkCommandBuffer cmd, uint32_t frameIndex);
  void UpdateParticles(float dt, uint32_t frameIndex);
  void UpdateGameData(uint32_t frameIndex, const core::Camera& camera, float screenW,
                      float screenH, const gm::Enviroment& enviroment);

  void Render(VkCommandBuffer cmd, float dt, uint32_t frameIndex, const core::Camera& camera);
  void RenderShadowPass(VkCommandBuffer cmd, uint32_t frameIndex);

  ChunkRenderer& chunks() { return *chunkRenderer_; }
  ParticleEngine& particles() { return *particleEngine_; }
  TextureManager& textures() { return *textureManager_; }
  GameDataBinding& gameData() { return *gameDataBinding_; }

  Atlas& blockAtlas() { return chunkRenderer_->getAtlas(); }

  ShaderCompiler& shaderCompiler() { return *shaderCompiler_; }
  const ShaderCompiler& shaderCompiler() const { return *shaderCompiler_; }

  void CollectEntities(uint32_t frameIndex);

 private:
  Engine* engine_;
  gm::WorldSession* session_;
  std::unique_ptr<ShaderCompiler> shaderCompiler_;
  std::unique_ptr<GameDataBinding> gameDataBinding_;
  std::unique_ptr<TextureManager> textureManager_;
  std::unique_ptr<ShadowContext> shadowCtxt_;
  std::unique_ptr<ChunkRenderer> chunkRenderer_;
  std::unique_ptr<gfx::Atlas> billboardsAtlas_;
  std::unique_ptr<BillboardRenderer> billboardRenderer_;
  BillboardRenderBucket* generalBucket_ = nullptr;
  BillboardRenderBucket* blockBucket_ = nullptr;
  std::unique_ptr<ParticleEngine> particleEngine_;
  std::unique_ptr<ModelCache> modelCache_;
  std::unique_ptr<ModelRenderer> modelRenderer_;
  std::unique_ptr<EntityRenderSystem> entityRenderSystem_;
  std::unique_ptr<SkyboxRenderer> skyboxRenderer_;
  std::unique_ptr<Skybox> nightSkybox_;
};

}  // namespace gfx