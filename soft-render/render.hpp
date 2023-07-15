#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <glm/vec3.hpp>
#include <numeric>
#include <variant>
#include <vector>

#include <iostream>

#include <soft-render/mfb_color.hpp>
#include <soft-render/scene.hpp>

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

void render1(std::vector<mfb_color> &buffer, const canvas_size_t &canvas_size,
             const viewport_size_t &viewport_size, const scene_t &scene);

} // namespace soft_render
