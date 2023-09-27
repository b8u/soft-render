#include "color.hpp"

namespace common::color {

[[nodiscard]] float_rgb make_float_rgb(uint8_t r, uint8_t g,
                                       uint8_t b) noexcept {
  glm::vec3 color;
  color.r = static_cast<float>(r) / 255.0f;
  color.g = static_cast<float>(g) / 255.0f;
  color.b = static_cast<float>(b) / 255.0f;

  return float_rgb{color};
}
} // namespace common::color
