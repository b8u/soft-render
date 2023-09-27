#pragma once

#include <glm/vec3.hpp>
#include <strong_type/strong_type.hpp>

namespace common::color {

using float_rgb =
    strong::type<glm::vec3, struct float_rgb_tag,
                 strong::regular, // strong::default_constructible,
                                  // strong::equality
                                  // and something else
                 strong::scalable_with<float>,
                 strong::arithmetic // what if I need not all the opeators?
                 >;

static constexpr float_rgb red{glm::vec3(1.0f, 0.0f, 0.0f)};
static constexpr float_rgb green{glm::vec3(0.0f, 1.0f, 0.0f)};
static constexpr float_rgb blue{glm::vec3(0.0f, 0.0f, 1.0f)};
static constexpr float_rgb yello{glm::vec3(1.0f, 0.984f, 0.0f)};

[[nodiscard]] float_rgb make_float_rgb(uint8_t r, uint8_t g,
                                       uint8_t b) noexcept;
} // namespace common::color
