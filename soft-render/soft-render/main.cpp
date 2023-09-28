#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

#include <MiniFB_cpp.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <glm/gtx/transform.hpp>
#include <glm/trigonometric.hpp>

#include <libraytracer/render.hpp>
#include <libraytracer/scene.hpp>

using namespace raytracer;

static constexpr unsigned window_width = 320;
static constexpr unsigned window_height = 320;

struct movement_controller {
  bool forward{};
  bool left{};
  bool right{};
  bool backward{};
  bool up{};
  bool down{};

  bool rotate_up{};
  bool rotate_down{};
  bool rotate_left{};
  bool rotate_right{};

  [[nodiscard]] glm::vec3 to_vec3() const noexcept {
    glm::vec3 x;
    x.z += forward ? 1.0f : 0.0f;
    x.z += backward ? -1.0f : 0.0f;
    x.x += left ? -1.0f : 0.0f;
    x.x += right ? 1.0f : 0.0f;
    x.y += up ? 1.0f : 0.0f;
    x.y += down ? -1.0f : 0.0f;

    return x;
  }

  [[nodiscard]] common::point3 apply(common::point3 position) const noexcept {
    const glm::vec3 self = to_vec3();
    // fmt::println("moves: ({}, {}, {})", self.x, self.y, self.z);
    position += common::point3::difference{self};
    return position;
  }

  [[nodiscard]] glm::vec2 rotate(glm::vec2 angles) const noexcept {
    if (rotate_up)
      angles.x -= 5.0f;
    if (rotate_down)
      angles.x += 5.0f;

    if (rotate_left)
      angles.y -= 5.0f;
    if (rotate_right)
      angles.y += 5.0f;

    return angles;
  }
};

int main(int argc, char *argv[]) {
  mfb_window *window = mfb_open("my_app", window_width, window_height);
  if (!window)
    return 0;
  auto buffer = std::vector<color_t>(window_width * window_height,
                                     color_t{1.0f, 0.0f, 0.0f});
  std::vector<sphere_t> objects = {

      {.color = common::color::red,
       .position = common::point3{glm::vec3(0, -1, 3)},
       .radius = 1.0f,
       .specular = 500.0f,
       .reflective = 0.2f},
      {.color = common::color::blue,
       .position = common::point3{glm::vec3(2, 0, 4)},
       .radius = 1.0f,
       .specular = 500.0f,
       .reflective = 0.3f},
      {.color = common::color::green,
       .position = common::point3{glm::vec3(-2, 0, 4)},
       .radius = 1.0f,
       .specular = 10.0f,
       .reflective = 0.4f},
      {.color = common::color::yello,
       .position = common::point3{glm::vec3(0, -5001, 0)},
       .radius = 5000.0f,
       .specular = 1000.0f,
       .reflective = 0.5f}

  };
  std::vector<light_t> lights = {
      ambient_light_t{.intensity = 0.2f},
      point_light_t{.intensity = 0.6f,
                    .position = common::point3{2.0f, 1.0f, 0.0f}},
      directional_light_t{.intensity = 0.2f,
                          .direction = common::vec3{1.0f, 4.0f, 4.0f}},
  };

  scene_t scene = {.lights = lights, .objects = objects};

  movement_controller moves;
  viewport_size_t viewport;
  bool exit = false;
  renderer main_renderer;

  mfb_set_keyboard_callback(
      [&moves, &exit,
       &main_renderer]([[maybe_unused]] mfb_window *window, mfb_key key,
                       [[maybe_unused]] mfb_key_mod mod, bool is_pressed) {
        switch (key) {
        case mfb_key::KB_KEY_W:
          moves.forward = is_pressed;
          break;
        case mfb_key::KB_KEY_S:
          moves.backward = is_pressed;
          break;
        case mfb_key::KB_KEY_A:
          moves.left = is_pressed;
          break;
        case mfb_key::KB_KEY_D:
          moves.right = is_pressed;
          break;
        case mfb_key::KB_KEY_Q:
          moves.up = is_pressed;
          break;
        case mfb_key::KB_KEY_E:
          moves.down = is_pressed;
          break;
        case mfb_key::KB_KEY_DOWN:
          moves.rotate_down = is_pressed;
          break;
        case mfb_key::KB_KEY_UP:
          moves.rotate_up = is_pressed;
          break;
        case mfb_key::KB_KEY_LEFT:
          moves.rotate_left = is_pressed;
          break;
        case mfb_key::KB_KEY_RIGHT:
          moves.rotate_right = is_pressed;
          break;

        case mfb_key::KB_KEY_ESCAPE:
          exit = true;
          break;

        case mfb_key::KB_KEY_F12:
          main_renderer.enable_mt();
          break;

        default:
          // nothing to handle
          break;
        }
      },
      window);

  auto start = std::chrono::steady_clock::now();
  int frame_counter = 0;
  do {
    // fmt::println("moves: w: {}, a: {}, s: {}, d: {}", moves.forward,
    // moves.left, moves.backward, moves.right);
    viewport.position = moves.apply(viewport.position);
    viewport.rotate(moves.rotate(viewport.rotation));

    main_renderer.render1(buffer,
                          {.width = pixel_coordinate_t(window_width),
                           .height = pixel_coordinate_t(window_height)},
                          viewport, scene);
    ++frame_counter;

    std::chrono::duration<double> frame =
        std::chrono::steady_clock::now() - start;

    if (frame.count() >= 1.0) {
      fmt::println("fps: {}",
                   static_cast<double>(frame_counter) / frame.count());
      start = std::chrono::steady_clock::now();
      frame_counter = 0;
    }

    std::vector<uint32_t> mfb_buffer(buffer.size());
    std::transform(buffer.begin(), buffer.end(), mfb_buffer.begin(),
                   [](const color_t c) -> uint32_t {
                     const uint32_t r = c.value_of().r * 255.0f;
                     const uint32_t g = c.value_of().g * 255.0f;
                     const uint32_t b = c.value_of().b * 255.0f;

                     const uint32_t value = MFB_RGB(r, g, b);
                     return value;
                   });

    const mfb_update_state state =
        mfb_update(window, static_cast<void *>(mfb_buffer.data()));
    if (state != STATE_OK) {
      window = nullptr;
      break;
    }
  } while (mfb_wait_sync(window) && !exit);

  return 0;
}
