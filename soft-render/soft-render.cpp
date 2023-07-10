#include <MiniFB_cpp.h>
#include <cassert>
#include <iostream>
#include <vector>

#include "render.hpp"

using namespace soft_render;

static constexpr unsigned window_width = 1024;
static constexpr unsigned window_height = 1024;

int main(int argc, char *argv[]) {
  mfb_window *window = mfb_open("my_app", window_width, window_height);
  if (!window)
    return 0;
  auto buffer =
      std::vector<mfb_color>(window_width * window_height, mfb_color::red());
  std::vector<sphere_t> objects = {{.color = mfb_color::red(),
                                    .position = glm::vec3(0, -1, 3),
                                    .radius = 1.0f},
                                   {.color = mfb_color::blue(),
                                    .position = glm::vec3(2, 0, 4),
                                    .radius = 1.0f},
                                   {.color = mfb_color::green(),
                                    .position = glm::vec3(-2, 0, 4),
                                    .radius = 1.0f}

  };

  viewport_size_t viewport;

  mfb_set_mouse_scroll_callback(
      [&viewport](auto *window, auto, float x, float y) {
        viewport.position.z += y * 0.1f;
      },
      window);

  do {
    render1(buffer,
            {.width = pixel_coordinate_t(window_width),
             .height = pixel_coordinate_t(window_height)},
            viewport, objects);

    const mfb_update_state state =
        mfb_update(window, static_cast<void *>(buffer.data()));
    std::cout << "frame finished. "
              << "is black: "
              << (std::find(buffer.begin(), buffer.end(), mfb_color{}) ==
                          buffer.end()
                      ? "YES"
                      : "NO")
              << ", state = " << state << std::endl;
    if (state != STATE_OK) {
      window = nullptr;
      break;
    }
  } while (mfb_wait_sync(window));

  return 0;
}
