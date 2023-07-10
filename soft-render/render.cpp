#include "render.hpp"
#include <glm/glm.hpp>
#include <tuple>

namespace soft_render {

/**
 * @return the same point on a projection plane
 * @param canvas - current canvas coordinates (pixels)
 */
[[nodiscard]] glm::vec3
canvas_to_viewport(glm::vec2 canvas, canvas_size_t canvas_size,
                   viewport_size_t viewport_size) noexcept {
  return {// simply scale the coordinate by canvas sizes
          canvas.x * viewport_size.width / canvas_size.width.as_float(),
          canvas.y * viewport_size.height / canvas_size.height.as_float(),
          // z component is a constant because it's a property of the viewport
          viewport_size.distance};
}

/**
 * The function simply finds intersections for a direction `ray` with a sphere
 * by the following equation:
 * <intersection_vec - sphere_vec, intersection_vec - sphere_vec> =
 * sphere_radius ^ 2
 *
 * The intersections must be along the ray vector (it goes from camera position
 * to a projection plane), so the intersection_vec is
 * viewport_position + t * ray.
 *
 * The equation could be transformed to
 * t^2 <ray,ray> + t (2 <viewport_position - sphere_vector, ray>) +
 * <viewport_position - sphere_vector, viewport_position - sphere_vector> -
 * r^2 = 0
 */
[[nodiscard]] std::tuple<float, float>
intersect_ray_sphere(glm::vec3 viewport_position, glm::vec3 ray,
                     const sphere_t &sphere) noexcept {
  // ray == D
  // a = <D, D>
  // b = 2<CO, D>
  // c = <CO, CO> - r^2
  // at^2 + bt + c = 0
  const float r = sphere.radius;
  const glm::vec3 CO = viewport_position - sphere.position;

  const auto a = glm::dot(ray, ray);
  const auto b = 2 * glm::dot(CO, ray);
  const auto c = glm::dot(CO, CO) - r * r;

  const auto discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    return std::tuple(std::numeric_limits<float>::infinity(),
                      std::numeric_limits<float>::infinity());
  }

  const float t1 = (-b + sqrt(discriminant)) / (2 * a);
  const float t2 = (-b - sqrt(discriminant)) / (2 * a);
  return {t1, t2};
}

/**
 * The raytraycer detects intersections with a spheres. It could be to close to
 * the camera (t_min) or to far from camers (t_max). We clip such intersections.
 */
[[nodiscard]] mfb_color trace_ray(glm::vec3 viewport_position, glm::vec3 ray,
                                  float t_min, float t_max,
                                  const std::vector<sphere_t> &objects,
                                  mfb_color background_color = {}) {

  float closest_t = std::numeric_limits<float>::infinity();
  const sphere_t *closest_object = nullptr;
  for (const auto &object : objects) {
    const auto [t1, t2] = intersect_ray_sphere(viewport_position, ray, object);
    if (t1 <= t_max && t1 >= t_min && t1 < closest_t) {
      closest_object = &object;
      closest_t = t1;
    }
    if (t2 <= t_max && t2 >= t_min && t2 < closest_t) {
      closest_object = &object;
      closest_t = t2;
    }
    if (t1 != std::numeric_limits<float>::infinity() ||
        t2 != std::numeric_limits<float>::infinity()) {
    }
  }

  if (closest_object == nullptr) {
    return background_color;
  }

  return closest_object->color;
}

void render1(std::vector<mfb_color> &buffer, const canvas_size_t &canvas_size,
             const viewport_size_t &viewport_size,
             const std::vector<sphere_t> &objects) {
  constexpr mfb_color color = {
      .b = 255,
      .g = 77,
      .r = 225,
      .a = 0,
  };

  /*
   * Canvas coordinates goes from left-top corner (x goes right, y goes down).
   * The projection plane has (0,0) in the center and y goes up.
   */
  for (ssize_t i = 0; i < canvas_size.width.as_ssize(); ++i) {
    // x goes from negative to positive (left-right).
    const auto x = i - canvas_size.width.as_ssize() / 2;
    for (ssize_t j = 0; j < canvas_size.height.as_ssize(); ++j) {
      // y goes from positive to negative (top-down).
      const auto y = canvas_size.width.as_ssize() / 2 - j;
      const auto canvas_position =
          glm::vec2(static_cast<float>(x), static_cast<float>(y));
      const auto ray =
          canvas_to_viewport(canvas_position, canvas_size, viewport_size);
      const mfb_color color =
          trace_ray(viewport_size.position, ray, 1,
                    std::numeric_limits<float>::infinity(), objects);
      const ssize_t index = i + j * canvas_size.width.as_ssize();
      // std::cout << color << std::endl;
      buffer[index] = color;
    }
  }
}
} // namespace soft_render
