#pragma once

#include <string_view>
#include <string>

namespace util {
[[nodiscard]] static std::u32string Utf8ToUtf32(std::string_view s) {
  static constexpr char32_t kMin[] = {0, 0, 0x80, 0x800, 0x10000};
  std::u32string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size();) {
    uint8_t c = static_cast<uint8_t>(s[i]);
    char32_t cp;
    size_t len;
    if (c < 0x80) {
      cp = c;
      len = 1;
    } else if ((c >> 5) == 0x6) {
      cp = c & 0x1F;
      len = 2;
    } else if ((c >> 4) == 0xE) {
      cp = c & 0x0F;
      len = 3;
    } else if ((c >> 3) == 0x1E) {
      cp = c & 0x07;
      len = 4;
    } else {
      out.push_back(0xFFFD);
      ++i;
      continue;
    }

    if (i + len > s.size()) {
      out.push_back(0xFFFD);
      break;
    }

    bool ok = true;
    for (size_t k = 1; k < len; ++k) {
      uint8_t cc = static_cast<uint8_t>(s[i + k]);
      if ((cc >> 6) != 0x2) {
        ok = false;
        len = k;
        break;
      }
      cp = (cp << 6) | (cc & 0x3F);
    }
    if (ok && (cp < kMin[len] || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))) ok = false;

    out.push_back(ok ? cp : 0xFFFD);
    i += len;
  }
  return out;
}

[[nodiscard]] static std::string Utf32ToUtf8(std::u32string_view s) {
  std::string out;
  out.reserve(s.size());
  for (char32_t cp : s) {
    if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) cp = 0xFFFD;

    if (cp < 0x80) {
      out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
      out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
      out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
      out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
      out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
  }
  return out;
}
}  // namespace util