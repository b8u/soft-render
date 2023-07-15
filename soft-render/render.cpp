#include "render.hpp"
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <tuple>

namespace soft_render {

// light ray from the light point to the object!
float calculate_diffuse_light(glm::vec3 normal, glm::vec3 light_ray,
                              float intencity) noexcept {
  // In general, the intencity changes by cos(angle of the light).
  // cos(two vectors) == dot product of two normalized vectors.
  return intencity * glm::dot(normal, glm::normalize(light_ray));
}

/**
 * @param point_to_camera - "view vector" from a point to a camera. previoulsy
 * we traced the reversed vector.
 *
 * @return specular coefficient of additional intencity for the ray.
 */
float calculate_specular_light(glm::vec3 point_to_camera, glm::vec3 normal,
                               glm::vec3 light_ray, float specular) {
  if (specular <= -1.0f)
    return 0.0f;
  // The picture looks like V (but the light ray in our case goes from the
  // object). The light ray reflects with the same angle for a normal. Light ray
  // projection:
  // * to normal = normal * <normal, light_ray>
  // * to object = light_ray - normal * <normal, light_ray>
  // reflected ray is a sum of those two rays.
  const auto reflected_ray =
      normal * glm::dot(normal, light_ray) * 2.0f - light_ray;
  const float r_dot_v = dot(reflected_ray, point_to_camera);
  if (r_dot_v > 0) {
    return std::pow(r_dot_v / (length(reflected_ray) * length(point_to_camera)),
                    specular);
  }
  // it's not reflected. do nothing with intencity.
  return 0.0f;
}

/**
 * @return intensity [0.0f, 1.0f] calculated by available light sources.
 */
float compute_lightning(glm::vec3 point, glm::vec3 normal,
                        const std::vector<light_t> &lights,
                        glm::vec3 point_to_camera, float specular) {
  float intensity = 0.0f;
  for (const auto &light : lights) {
    if (std::holds_alternative<ambient_light_t>(light)) {
      // It's reflected light, so we don't care about phisics and assume that
      // all objects emit a bit of light.
      intensity += std::get<ambient_light_t>(light).intensity;
    } else if (std::holds_alternative<directional_light_t>(light)) {
      const auto &dl = std::get<directional_light_t>(light);
      // Directional light goes always to one direction.
      intensity += std::max(
          calculate_diffuse_light(normal, dl.direction, dl.intensity), 0.0f);
      intensity +=
          dl.intensity * calculate_specular_light(point_to_camera, normal,
                                                  dl.direction, specular);
    } else if (std::holds_alternative<point_light_t>(light)) {
      const auto &pl = std::get<point_light_t>(light);
      // Once again, the light goes from the light position to the object.
      const glm::vec3 light_ray = pl.position - point;
      intensity += std::max(
          calculate_diffuse_light(normal, light_ray, pl.intensity), 0.0f);
      intensity +=
          pl.intensity * calculate_specular_light(point_to_camera, normal,
                                                  light_ray, specular);
    }
  }
  return std::min(intensity, 1.0f);
}

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
                                  const scene_t &scene,
                                  mfb_color background_color = {}) {

  float closest_t = std::numeric_limits<float>::infinity();
  const sphere_t *closest_object = nullptr;
  for (const auto &object : scene.objects) {
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

  const glm::vec3 point = viewport_position + ray * closest_t; // why plus???
  const glm::vec3 normal = glm::normalize(point - closest_object->position);
  mfb_color color = closest_object->color;
  const float light = compute_lightning(point, normal, scene.lights,
                                        ray * -1.0f, closest_object->specular);
  if (light < 0.003f) {
    fmt::println("light is {}", light);
  }
  color.set(color.as_rgb_vec() * light);
  if (color == mfb_color{.b = 0, .g = 0, .r = 0, .a = 0}) {
    fmt::println("lightning: {}", light);
  }
  return color;
}

void render1(std::vector<mfb_color> &buffer, const canvas_size_t &canvas_size,
             const viewport_size_t &viewport_size, const scene_t &scene) {
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
                    std::numeric_limits<float>::infinity(), scene);
      const ssize_t index = i + j * canvas_size.width.as_ssize();
      // std::cout << color << std::endl;
      buffer[index] = color;
    }
  }
}
} // namespace soft_render
