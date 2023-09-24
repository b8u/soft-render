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

}
