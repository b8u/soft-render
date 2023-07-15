#pragma once

#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include <soft-render/mfb_color.hpp>

namespace soft_render {

struct sphere_t {
  mfb_color color;
  glm::vec3 position;
  float radius = 1;
  float specular = -1.0f; // by default it's disabled
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

struct scene_t {
  std::vector<light_t> lights;
  std::vector<sphere_t> objects;
  // A viewport is not here because you can render the same scene from different
  // camers (split screen).
};

} // namespace soft_render
