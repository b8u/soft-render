#include <fmt/format.h>
#include <glm/glm.hpp>
#include <libcommon/glm.hpp>

int main() {
  fmt::println("{:.6f}", glm::vec3{1.0f, 2.0f, 3.0f});

  return 0;
}
