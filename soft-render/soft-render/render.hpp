#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <numeric>
#include <variant>
#include <vector>

#include <boost/asio.hpp>
#include <fmt/format.h>
#include <glm/gtx/transform.hpp>
#include <glm/vec3.hpp>

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
  glm::vec2 rotation;
  glm::mat4 rotation_matrix = glm::mat4(1.0f);

  void rotate(glm::vec2 rotation) noexcept {
    this->rotation = rotation;

    // fmt::println("rotation: [{}, {}]", rotation.x, rotation.y);

    glm::mat4 xm(1.0f);
    if (rotation.x >= 0.001f || rotation.x <= -0.001f) {
      xm = glm::rotate(xm, glm::radians(glm::abs(rotation.x)),
                       {glm::sign(rotation.x), 0.0f, 0.0f});
    }

    glm::mat4 ym(1.0f);
    if (rotation.y >= 0.001f || rotation.y <= -0.001f) {
      ym = glm::rotate(ym, glm::radians(glm::abs(rotation.y)),
                       {0.0f, glm::sign(rotation.y), 0.0f});
    }

    rotation_matrix = xm * ym;
  }

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

class renderer {
public:
  renderer();

  void render1(std::vector<mfb_color> &buffer, const canvas_size_t &canvas_size,
               const viewport_size_t viewport_size, const scene_t &scene);

  inline void disable_mt() noexcept { mt_disabled = true; }
  inline void enable_mt() noexcept { mt_disabled = false; }
  inline void toggle_mt() noexcept { mt_disabled = !mt_disabled; }

private:
  boost::asio::thread_pool pool;
  std::mutex buf_mutex;
  bool mt_disabled = true;
};

} // namespace soft_render
