#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <unordered_map>

#include "Engine.h"
#include "PlayerController.h"
#include "RenderWorld.h"
#include "WorldSession.h"
#include "game/ControlState.h"
#include "game/LibControl.h"
#include "gfx/block/PreviewRenderer.h"
#include "gfx/ui/LibGui.h"
#include "gfx/ui/Ui.h"

#ifdef __ANDROID__
struct android_app;
#include "core/InputAndroid.h"  // для core::Pointer
#else
#include "core/Window.h"
#endif

class Game {
 public:
#ifdef __ANDROID__
  explicit Game(android_app* app);
  bool Frame(const std::unordered_map<int32_t, core::Pointer>& touches);
  bool IsRenderable() const { return engine_ && engine_->IsRenderable(); }
  void OnSurfaceRestored() { engine_->restoreSurface(); }
  void OnSurfaceDestroyed() { engine_->destroySurface(); }
#else
  explicit Game(std::unique_ptr<core::Window> window);
  bool Frame();
  core::Window& window() { return *window_; }
#endif

  ~Game();

 private:
  void Init();
  void SetViewportAndScissor(VkCommandBuffer cmd);

#ifndef __ANDROID__
  std::unique_ptr<core::Window> window_;
#endif

  std::unique_ptr<Engine> engine_;
  std::unique_ptr<gm::WorldSession> session_;
  std::unique_ptr<gfx::RenderWorld> render_;
  std::unique_ptr<gm::PlayerController> player_;

  script::LuaState luaState_;
  std::optional<gfx::ui::Ui> ui_;
  std::optional<gfx::ui::LibGui> libGui_;
  std::optional<gm::LibControl> libControl_;
  std::unique_ptr<gfx::block::PreviewRenderer> blockPreviewRenderer_;

  std::unique_ptr<vkcore::PipelineLayout> crosshairLayout_;
  std::unique_ptr<vkcore::Pipeline> crosshairPipeline_;

  gm::ControlState controlState_;
  bool cursorLocked_ = true;

  std::chrono::high_resolution_clock::time_point lastFrameTime_{};
  std::chrono::high_resolution_clock::time_point lastFpsTime_{};
  float dt_ = 0.0f;
  int frameCount_ = 0;
  int fpsFrames_ = 0;
};