#include <MiniFB_cpp.h>
#include <cassert>
#include <chrono>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iostream>
#include <vector>

#include "render.hpp"

using namespace soft_render;

static constexpr unsigned window_width = 1024;
static constexpr unsigned window_height = 1024;

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

  [[nodiscard]] glm::vec3 apply(glm::vec3 position) const noexcept {
    const glm::vec3 self = to_vec3();
    fmt::println("moves: ({}, {}, {})", self.x, self.y, self.z);
    position += self;
    return position;
  }

  [[nodiscard]] glm::vec3 rotate(glm::vec3 vector) const noexcept {
    vector.x += rotate_left ? -0.1f : 0.0f;
    vector.x += rotate_right ? 0.1f : 0.0f;
    vector.y += rotate_up ? 0.1f : 0.0f;
    vector.y += rotate_down ? -0.1f : 0.0f;

    return vector;
  }
};

int main(int argc, char *argv[]) {
  mfb_window *window = mfb_open("my_app", window_width, window_height);
  if (!window)
    return 0;
  auto buffer =
      std::vector<mfb_color>(window_width * window_height, mfb_color::red());
  std::vector<sphere_t> objects = {

      {.color = mfb_color::red(),
       .position = glm::vec3(0, -1, 3),
       .radius = 1.0f,
       .specular = 500.0f,
       .reflective = 0.2f},
      {.color = mfb_color::blue(),
       .position = glm::vec3(2, 0, 4),
       .radius = 1.0f,
       .specular = 500.0f,
       .reflective = 0.3f},
      {.color = mfb_color::green(),
       .position = glm::vec3(-2, 0, 4),
       .radius = 1.0f,
       .specular = 10.0f,
       .reflective = 0.4f},
      {.color = mfb_color::yello(),
       .position = glm::vec3(0, -5001, 0),
       .radius = 5000.0f,
       .specular = 1000.0f,
       .reflective = 0.5f}

  };
  std::vector<light_t> lights = {
      ambient_light_t{.intensity = 0.2f},
      point_light_t{.intensity = 0.6f, .position = {2.0f, 1.0f, 0.0f}},
      directional_light_t{.intensity = 0.2f, .direction = {1.0f, 4.0f, 4.0f}},
  };

  scene_t scene = {.lights = lights, .objects = objects};

  movement_controller moves;
  viewport_size_t viewport;
  bool exit = false;

  mfb_set_keyboard_callback(
      [&moves, &exit]([[maybe_unused]] mfb_window *window, mfb_key key,
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

        default:
          // nothing to handle
          break;
        }
      },
      window);

  renderer main_renderer;

  do {
    fmt::println("moves: w: {}, a: {}, s: {}, d: {}", moves.forward, moves.left,
                 moves.backward, moves.right);
    viewport.position = moves.apply(viewport.position);
    viewport.rotation = moves.rotate(viewport.rotation);
    main_renderer.render1(buffer,
                          {.width = pixel_coordinate_t(window_width),
                           .height = pixel_coordinate_t(window_height)},
                          viewport, scene);

    const mfb_update_state state =
        mfb_update(window, static_cast<void *>(buffer.data()));
    if (state != STATE_OK) {
      window = nullptr;
      break;
    }
  } while (mfb_wait_sync(window) && !exit);

  return 0;
}
