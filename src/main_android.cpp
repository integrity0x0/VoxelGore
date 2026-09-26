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
#include "Game.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "VoxelGore", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "VoxelGore", __VA_ARGS__)

AAssetManager* g_AAssetManager = nullptr;

static void WriteStatusFile(const char* filename, const char* content) {
  std::string path = std::string("/sdcard/Android/data/com.voxelgore/files/") + filename;
  FILE* f = fopen(path.c_str(), "w");
  if (f) {
    fputs(content, f);
    fclose(f);
  } else {
    LOGE("Failed to open status file: %s (errno: %d, %s)", path.c_str(), errno, strerror(errno));
  }
}

static std::unique_ptr<Game> g_game;
static core::InputAndroid g_input;

static void handleAppCmd(android_app* app, int32_t cmd) {
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (app->window == nullptr) break;
      if (!g_game) {
        try {
          g_game = std::make_unique<Game>(app);
          WriteStatusFile("status_ok.txt", "OK: Game created\n");
        } catch (const std::exception& e) {
          LOGE("Exception: %s", e.what());
          WriteStatusFile("status_exception.txt", e.what());
          g_game.reset();
        }
      } else {
        g_game->OnSurfaceRestored();
      }
      break;

    case APP_CMD_TERM_WINDOW:
      if (g_game) g_game->OnSurfaceDestroyed();
      break;

    case APP_CMD_DESTROY:
      g_game.reset();
      break;
  }
}

static int32_t handleInputEvent(android_app*, AInputEvent* event) {
    if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION) return 0;
    g_input.Update(event);
    return 1;
}

void android_main(android_app* app) {
    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;
    g_AAssetManager = app->activity->assetManager;

    int events = 0;
    android_poll_source* source = nullptr;

    while (!app->destroyRequested) {
        g_input.Reset();
        while (ALooper_pollOnce(g_game ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
            if (app->destroyRequested) break;
        }
        if (app->destroyRequested) break;

        if (g_game && g_game->IsRenderable()) {
            try {
                if (!g_game->Frame(g_input.inputState().pointers())) {
                    g_game.reset();
                }
            } catch (const std::exception& e) {
                LOGE("Exception: %s", e.what());
                g_game.reset();
            }
        }
    }

    g_game.reset();
    LOGI("Goodbye!");
}