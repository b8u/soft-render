#include "render.hpp"
#include <boost/asio.hpp>
#include <boost/thread/latch.hpp>
#include <fmt/core.h>
#include <glm/glm.hpp>
#include <libcommon/affine_space.hpp>
#include <mutex>
#include <strong_type/strong_type.hpp>
#include <tuple>

namespace raytracer {

using common::point3;
using common::vec3;

[[nodiscard]] std::pair<const sphere_t *, float>
closest_intersection(point3 origin, vec3 ray, float t_min, float t_max,
                     const scene_t &scene);

// light ray from the light point to the object!
float calculate_diffuse_light(vec3 normal, vec3 light_ray,
                              float intencity) noexcept {
  // In general, the intencity changes by cos(angle of the light).
  // cos(two vectors) == dot product of two normalized vectors.
  return intencity * common::dot(normal, common::normalize(light_ray));
}

/**
 * @param point_to_camera - "view vector" from a point to a camera. previoulsy
 * we traced the reversed vector.
 *
 * @return specular coefficient of additional intencity for the ray.
 */
float calculate_specular_light(vec3 point_to_camera, vec3 normal,
                               vec3 light_ray, float specular) {
  if (specular <= -1.0f)
    return 0.0f;
  // The picture looks like V (but the light ray in our case goes from the
  // object). The light ray reflects with the same angle for a normal. Light ray
  // projection:
  // * to normal = normal * <normal, light_ray>
  // * to object = light_ray - normal * <normal, light_ray>
  // reflected ray is a sum of those two rays.
  const vec3 reflected_ray =
      normal * common::dot(normal, light_ray) * 2.0f - light_ray;
  const float r_dot_v = common::dot(reflected_ray, point_to_camera);
  if (r_dot_v > 0) {
    return std::pow(r_dot_v / (common::length(reflected_ray) *
                               common::length(point_to_camera)),
                    specular);
  }
  // it's not reflected. do nothing with intencity.
  return 0.0f;
}

/**
 * @return intensity [0.0f, 1.0f] calculated by available light sources.
 */
float compute_lightning(point3 point, vec3 normal, const scene_t &scene,
                        vec3 point_to_camera, float specular) {
  float intensity = 0.0f;
  for (const auto &light : scene.lights) {
    if (std::holds_alternative<ambient_light_t>(light)) {
      // It's reflected light, so we don't care about phisics and assume that
      // all objects emit a bit of light.
      intensity += std::get<ambient_light_t>(light).intensity;
    } else {
      const auto [light_ray, light_intensity, t_max] = std::visit(
          [point](const auto &light) -> std::tuple<vec3, float, float> {
            using light_t = std::decay_t<decltype(light)>;
            if constexpr (std::is_same_v<light_t, directional_light_t>) {
              // Directional light goes always to one direction.
              const vec3 light_ray = light.direction;
              const float t_max = std::numeric_limits<float>::infinity();
              return {light_ray, light.intensity, t_max};
            } else if constexpr (std::is_same_v<light_t, point_light_t>) {
              // Once again, the light goes from the light position to the
              // object.
              const vec3 light_ray = light.position - point;
              const float t_max = 1.0f;
              return {light_ray, light.intensity, t_max};
            } else {
              static_assert("looks like we don't handle some sort of light");
              return {};
            }
          },
          light);

      // Shadow check
      const auto [shadow_sphere, shadow_t] =
          closest_intersection(point, light_ray, 0.001f, t_max, scene);
      if (shadow_sphere != nullptr) {
        continue;
      }

      intensity += std::max(
          calculate_diffuse_light(normal, light_ray, light_intensity), 0.0f);
      intensity +=
          light_intensity * calculate_specular_light(point_to_camera, normal,
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
intersect_ray_sphere(point3 viewport_position, vec3 ray,
                     const sphere_t &sphere) noexcept {
  // ray == D
  // a = <D, D>
  // b = 2<CO, D>
  // c = <CO, CO> - r^2
  // at^2 + bt + c = 0
  const float r = sphere.radius;
  const vec3 CO = viewport_position - sphere.position;

  const float a = common::dot(ray, ray);
  const float b = 2 * common::dot(CO, ray);
  const float c = common::dot(CO, CO) - r * r;

  const float discriminant = b * b - 4 * a * c;
  if (discriminant < 0) {
    return std::tuple(std::numeric_limits<float>::infinity(),
                      std::numeric_limits<float>::infinity());
  }

  const float t1 = (-b + sqrt(discriminant)) / (2 * a);
  const float t2 = (-b - sqrt(discriminant)) / (2 * a);
  return {t1, t2};
}

/**
 * \param origin is a point from where the ray is going.
 */
[[nodiscard]] std::pair<const sphere_t *, float>
closest_intersection(point3 origin, vec3 ray, float t_min, float t_max,
                     const scene_t &scene) {
  float closest_t = std::numeric_limits<float>::infinity();
  const sphere_t *closest_object = nullptr;
  for (const auto &object : scene.objects) {
    const auto [t1, t2] = intersect_ray_sphere(origin, ray, object);
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
  return {closest_object, closest_t};
}

vec3 reflect_ray(vec3 ray, vec3 normal) noexcept {
  return 2.0f * normal * common::dot(normal, ray) - ray;
}

/**
 * The raytraycer detects intersections with a spheres. It could be too close to
 * the camera (t_min) or too far from camers (t_max). We clip such
 * intersections.
 */
[[nodiscard]] color_t trace_ray(point3 viewport_position, vec3 ray, float t_min,
                                float t_max, const scene_t &scene,
                                int recursion_depth = 0,
                                color_t background_color = color_t{}) {
  auto [closest_object, closest_t] =
      closest_intersection(viewport_position, ray, t_min, t_max, scene);

  if (closest_object == nullptr) {
    return background_color;
  }

  const point3 point = viewport_position + ray * closest_t; // why plus???
  const vec3 normal = common::normalize(point - closest_object->position);
  color_t local_color = closest_object->color;
  const float light =
      compute_lightning(point, normal, scene, -ray, closest_object->specular);
  local_color *= light;

  // If we hit the recursion limit or the object is not reflective, we'ra done
  if (recursion_depth <= 0 or closest_object->reflective <= 0) {
    return local_color;
  }

  // Compute the reflected color
  const vec3 reflected_ray = reflect_ray(-ray, normal);
  const color_t reflected_color = trace_ray(
      point, reflected_ray, 0.001f, std::numeric_limits<float>::infinity(),
      scene, recursion_depth - 1, background_color);

  if (reflected_color == background_color) {
    return local_color;
  }

  return local_color * (1 - closest_object->reflective) +
         reflected_color * closest_object->reflective;
}

renderer::renderer() : pool(16) {}

void renderer::render1(std::vector<color_t> &buffer,
                       const canvas_size_t &canvas_size,
                       const viewport_size_t viewport_size,
                       const scene_t &scene) {

  boost::latch sync(canvas_size.height.as_size());

  /*
   * Canvas coordinates goes from left-top corner (x goes right, y goes
   * down). The projection plane has (0,0) in the center and y goes up.
   */
  for (ssize_t j = 0; j < canvas_size.height.as_ssize(); ++j) {
    // y goes from positive to negative (top-down).
    const auto y = canvas_size.width.as_ssize() / 2 - j;

    const auto task = [this, canvas_size, viewport_size, &scene, y, &buffer, j,
                       &sync] {
      std::vector<color_t> local_buffer(canvas_size.width.as_size());
      for (ssize_t i = 0; i < canvas_size.width.as_ssize(); ++i) {
        // x goes from negative to positive (left-right).
        const auto x = i - canvas_size.width.as_ssize() / 2;
        const auto canvas_position =
            glm::vec2(static_cast<float>(x), static_cast<float>(y));
        glm::vec3 ray =
            canvas_to_viewport(canvas_position, canvas_size, viewport_size);
        const glm::vec3 debug_ray =
            viewport_size.rotation_matrix * glm::vec4(ray, 1.0f);

        // fmt::println("ray: [{}, {}, {}], debug_ray: [{}, {}, {}]", ray.x,
        // ray.y, ray.z, debug_ray.x, debug_ray.y, debug_ray.z);

        ray = debug_ray;

        const color_t color =
            trace_ray(viewport_size.position, vec3{ray}, 1,
                      std::numeric_limits<float>::infinity(), scene, 3);
        local_buffer[i] = color;
      }

      // it should be useless here because we don't use the
      // same elements in different threads.
      std::lock_guard lock(buf_mutex);

      std::copy(local_buffer.begin(), local_buffer.end(),
                std::next(buffer.begin(), j * canvas_size.width.as_size()));

      sync.count_down();
    };

    if (mt_disabled) {
      task();
    } else {
      boost::asio::post(pool, task);
    }
  }

  sync.wait();
}
} // namespace raytracer
