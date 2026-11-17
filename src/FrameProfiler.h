#pragma once

// Лёгкий CPU-профайлер для покадровых замеров.
//
// Использование:
//   #include "util/FrameProfiler.h"
//   { VG_PROF_SCOPE("chunks.solid"); chunkRenderer_->Render(...); }   // один макрос на строку!
//   VG_PROF_FRAME();   // ровно один раз в конце кадра
//
// Раз в секунду печатает: fps, средний/макс. кадр и по каждой секции
// среднее время НА КАДР (сумма всех вызовов / число кадров), максимум одного вызова и % кадра.
// Любой вызов дольше kSpikeMs логируется сразу (ловит рывки, а не только среднее).
//
// Выключить полностью: определить VG_PROFILING=0 (например в CMake).
// Вложенные секции считаются независимо (проценты пересекаются, это нормально).
// Только главный поток.

#ifndef VG_PROFILING
#define VG_PROFILING 1
#endif

#if VG_PROFILING

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <vector>

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace util {

    class FrameProfiler {
    public:
        using Clock = std::chrono::steady_clock;

        static constexpr double kSpikeMs = 8.0;
        static constexpr double kReportSeconds = 1.0;

        static FrameProfiler& Get() {
            static FrameProfiler instance;
            return instance;
        }

        size_t Register(const char* name) {
            sections_.push_back({name, 0.0, 0.0, 0});
            return sections_.size() - 1;
        }

        void Record(size_t id, double ms) {
            Section& s = sections_[id];
            s.total += ms;
            s.max = std::max(s.max, ms);
            ++s.calls;
            if (ms > kSpikeMs) Log("[SPIKE] %s: %.2f ms", s.name, ms);
        }

        void FrameMark() {
            const auto now = Clock::now();

            if (started_) {
                const double ms = std::chrono::duration<double, std::milli>(now - lastMark_).count();
                frameTotal_ += ms;
                frameMax_ = std::max(frameMax_, ms);
                ++frames_;
                if (ms > kSpikeMs) Log("[SPIKE] whole frame: %.2f ms", ms);
            } else {
                started_ = true;
                reportStart_ = now;
            }
            if (std::chrono::duration<double>(now - reportStart_).count() >= kReportSeconds) {
                Report(now);
            }

            // Ставим после Report: время печати в консоль не должно попадать в следующий кадр.
            lastMark_ = Clock::now();
        }

    private:
        struct Section {
            const char* name;
            double total;
            double max;
            uint32_t calls;
        };

        void Report(Clock::time_point now) {
            const double elapsed = std::chrono::duration<double>(now - reportStart_).count();

            if (frames_ > 0 && elapsed > 0.0) {
                const double avgFrame = frameTotal_ / static_cast<double>(frames_);

                Log("[PROF] ===== %.0f fps | frame avg %.3f ms, max %.3f ms =====",
                    static_cast<double>(frames_) / elapsed, avgFrame, frameMax_);
                Log("[PROF] %-32s %9s %9s %7s", "section", "avg ms", "max ms", "%frame");

                for (Section& s : sections_) {
                    if (s.calls == 0) continue;
                    const double avg = s.total / static_cast<double>(frames_);
                    Log("[PROF] %-32s %9.3f %9.3f %6.1f%%", s.name, avg, s.max, 100.0 * avg / avgFrame);
                    s.total = 0.0;
                    s.max = 0.0;
                    s.calls = 0;
                }
            }

            frames_ = 0;
            frameTotal_ = 0.0;
            frameMax_ = 0.0;
            reportStart_ = now;
        }

        static void Log(const char* fmt, ...) {
            char buf[256];
            va_list args;
            va_start(args, fmt);
            std::vsnprintf(buf, sizeof(buf), fmt, args);
            va_end(args);
#ifdef __ANDROID__
            __android_log_write(ANDROID_LOG_INFO, "VG_PROF", buf);
#else
            std::puts(buf);
#endif
        }

        std::vector<Section> sections_;
        Clock::time_point lastMark_{};
        Clock::time_point reportStart_{};
        bool started_ = false;
        uint32_t frames_ = 0;
        double frameTotal_ = 0.0;
        double frameMax_ = 0.0;
    };

    class ScopedTimer {
    public:
        explicit ScopedTimer(size_t id) : id_(id), start_(FrameProfiler::Clock::now()) {}
        ~ScopedTimer() {
            const double ms =
                    std::chrono::duration<double, std::milli>(FrameProfiler::Clock::now() - start_).count();
            FrameProfiler::Get().Record(id_, ms);
        }
        ScopedTimer(const ScopedTimer&) = delete;
        ScopedTimer& operator=(const ScopedTimer&) = delete;

    private:
        size_t id_;
        FrameProfiler::Clock::time_point start_;
    };

}  // namespace util

#define VG_PROF_CAT2(a, b) a##b
#define VG_PROF_CAT(a, b) VG_PROF_CAT2(a, b)

#define VG_PROF_SCOPE(name)                                                        \
  static const size_t VG_PROF_CAT(vgProfId_, __LINE__) =                           \
      ::util::FrameProfiler::Get().Register(name);                                 \
  ::util::ScopedTimer VG_PROF_CAT(vgProfTimer_, __LINE__)(VG_PROF_CAT(vgProfId_, __LINE__))

#define VG_PROF_FRAME() ::util::FrameProfiler::Get().FrameMark()

#else  // VG_PROFILING == 0

#define VG_PROF_SCOPE(name) ((void)0)
#define VG_PROF_FRAME() ((void)0)

#endif