#pragma once

#include <cstddef>
#include <cstdint>
#include <ostream>

#include <glm/glm.hpp>

namespace soft_render {
struct mfb_color {
  uint8_t b = 0;
  uint8_t g = 0;
  uint8_t r = 0;
  uint8_t a = 0;

  static constexpr mfb_color red() {
    return {.b = 0, .g = 0, .r = 0xff, .a = 0};
  }
  static constexpr mfb_color green() {
    return {.b = 0, .g = 0xff, .r = 0, .a = 0};
  }
  static constexpr mfb_color blue() {
    return {.b = 0xff, .g = 0, .r = 0, .a = 0};
  }
  static constexpr mfb_color yello() {
    return {.b = 0, .g = 0xff, .r = 0xff, .a = 0};
  }

  explicit inline operator uint32_t() const noexcept {
    return (((uint32_t)a) << 24) | (((uint32_t)b) << 16) |
           (((uint32_t)g) << 8) | ((uint32_t)r);
  }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const mfb_color &value) {
    stream << "(" << int(value.r) << ", " << int(value.g) << ", "
           << int(value.b) << ", " << int(value.a) << ")";
    return stream;
  }
  friend bool operator<=>(const mfb_color &l,
                          const mfb_color &r) noexcept = default;

  [[nodiscard]] glm::vec3 as_rgb_vec() const noexcept {
    return glm::vec3(r, g, b) / 255.0f;
  }

  inline mfb_color &set(glm::vec3 rgb) noexcept {
    rgb *= 255.0f;
    b = rgb.b;
    g = rgb.g;
    r = rgb.r;

    return *this;
  }
};
} // namespace soft_render
