#pragma once

#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include <libcommon/affine_space.hpp>
#include <libcommon/color.hpp>

namespace raytracer {

struct sphere_t {
  common::color::float_rgb color;
  common::point3 position;
  float radius = 1;
  float specular = -1.0f; // by default it's disabled
  float reflective = 0.5f;
};

struct ambient_light_t {
  float intensity = 0.0f;
};

struct directional_light_t {
  float intensity = 0.0f;
  common::vec3 direction;
};

struct point_light_t {
  float intensity = 0.0f;
  common::point3 position;
};

using light_t =
    std::variant<ambient_light_t, directional_light_t, point_light_t>;

struct scene_t {
  std::vector<light_t> lights;
  std::vector<sphere_t> objects;
  // A viewport is not here because you can render the same scene from different
  // camers (split screen).
};

} // namespace raytracer
