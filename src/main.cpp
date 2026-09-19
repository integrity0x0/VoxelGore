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
#include "gfx/render/block/BlockPreviewRenderer.h"
#include "gfx/render/entity/EntityRenderSystem.h"
#include "gfx/render/billboard/BillboardRenderer.h"
#include "gfx/render/chunk/ChunkRenderer.h"
#include "gfx/render/particle/ParticleEngine.h"
#include "gfx/render/ui/LibGui.h"
#include "gfx/common/texture/Skybox.h"
#include "gfx/render/skybox/SkyboxRenderer.h"

#include "Game.h"

int main() {
  glfwInit();
  try {
    core::Window::Hint(GLFW_RESIZABLE, GLFW_TRUE);
    core::Window::Hint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = std::make_unique<core::Window>(1280, 720, "VoxelGore v0.10-alpha | WTF!!!");

    Game game(std::move(window));
    while (!game.window().IsShouldClose()) {
      glfwPollEvents();
      if (game.window().IsMinimized()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        continue;
      }   
      game.Frame();
    }
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] " << e.what() << "\n";
  }
  glfwTerminate();
  return 0;
}