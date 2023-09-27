#pragma once

#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <strong_type/affine_point.hpp>
#include <strong_type/arithmetic.hpp>
#include <strong_type/scalable_with.hpp>

namespace common::affine_space {
using vec3 =
    strong::type<glm::vec3, struct vec3_tag, strong::default_constructible,
                 strong::scalable_with<float>, strong::arithmetic>;
using point3 =
    strong::type<glm::vec3, struct vec3_tag, strong::affine_point<vec3>,
                 strong::default_constructible
                 //, strong::arithmetic
                 >;

inline float dot(vec3 x, vec3 y) noexcept {
  return glm::dot(x.value_of(), y.value_of());
}

inline vec3 normalize(vec3 v) noexcept {
  return vec3{glm::normalize(v.value_of())};
}

inline float length(vec3 v) noexcept { return glm::length(v.value_of()); }

} // namespace common::affine_space
