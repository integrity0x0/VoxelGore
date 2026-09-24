#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "Engine.h"
#include "Game.h"

int main() {
  glfwInit();

  std::atomic<uint64_t> frameCount{0};
  std::atomic<bool> running{true};

  std::thread fpsThread([&]() {
    while (running.load(std::memory_order_relaxed)) {
      std::this_thread::sleep_for(std::chrono::seconds(1));

      const uint64_t fps = frameCount.exchange(0, std::memory_order_relaxed);

      std::cout << "FPS: " << fps << '\n';
    }
  });

  try {
    core::Window::Hint(GLFW_RESIZABLE, GLFW_TRUE);
    core::Window::Hint(GLFW_CLIENT_API, GLFW_NO_API);

    auto window = std::make_unique<core::Window>(1280, 720, "VoxelGore v0.10-alpha");

    Game game(std::move(window));

    while (!game.window().IsShouldClose()) {
      glfwPollEvents();

      if (game.window().IsMinimized()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        continue;
      }

      game.Frame();
      frameCount.fetch_add(1, std::memory_order_relaxed);
    }
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] " << e.what() << "\n";
  }

  running.store(false, std::memory_order_relaxed);
  fpsThread.join();

  glfwTerminate();
  return 0;
}

// TODO: fix screen resize