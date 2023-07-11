#include <cassert>
#include <cstddef>
#include <cstdint>
#include <glm/vec3.hpp>
#include <numeric>
#include <variant>
#include <vector>

#include <iostream>

namespace soft_render {

class pixel_coordinate_t {
public:
  explicit pixel_coordinate_t(size_t c) noexcept : component_(c) {}
  explicit operator float() const noexcept { return as_float(); }
  inline float as_float() const noexcept {
    return static_cast<float>(component_);
  }
  inline ssize_t as_ssize() const noexcept {
    return static_cast<ssize_t>(component_);
  }
  inline size_t as_size() const noexcept { return component_; }

private:
  size_t component_;
};

template <typename T> struct plane_t {
  T width{};
  T height{};
};

using canvas_size_t = plane_t<pixel_coordinate_t>;
struct viewport_size_t : plane_t<float> {
  viewport_size_t() : plane_t({1.0f, 1.0f}), distance(1.0f) {}
  /// Distance from a viewport position to a projection plane
  float distance = 1;
  glm::vec3 position;

  friend std::ostream &operator<<(std::ostream &os,
                                  const viewport_size_t &value) {
    os << value.width << "x" << value.height
       << ", distance = " << value.distance << ", position = ("
       << value.position.x << ", " << value.position.y << ", "
       << value.position.z << ")";
    // TODO: position
    return os;
  }
};

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

  explicit operator uint32_t() const noexcept {
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

  mfb_color &set(glm::vec3 rgb) noexcept {
    rgb *= 255.0f;
    b = rgb.b;
    g = rgb.g;
    r = rgb.r;

    return *this;
  }
};

struct sphere_t {
  mfb_color color;
  glm::vec3 position;
  float radius = 1;
};

struct ambient_light_t {
  float intensity = 0.0f;
};

struct directional_light_t {
  float intensity = 0.0f;
  glm::vec3 direction;
};

struct point_light_t {
  float intensity = 0.0f;
  glm::vec3 position;
};

using light_t =
    std::variant<ambient_light_t, directional_light_t, point_light_t>;

void render1(std::vector<mfb_color> &buffer, const canvas_size_t &canvas_size,
             const viewport_size_t &viewport_size,
             const std::vector<sphere_t> &objects,
             const std::vector<light_t> &lights);

} // namespace soft_render
