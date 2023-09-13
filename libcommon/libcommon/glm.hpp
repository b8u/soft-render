#include <fmt/core.h>
#include <glm/glm.hpp>

template <glm::length_t L, glm::qualifier Q>
struct fmt::formatter<glm::vec<L, float, Q>> {
  using Type = glm::vec<L, float, Q>;

  char tpl[20] = "{}";

  constexpr fmt::format_parse_context::iterator
  parse(fmt::format_parse_context &ctx) {
    // [ctx.begin(), ctx.end()) is a character range that contains a part of
    // the format string starting from the format specifications to be parsed,
    // e.g. in
    //
    //   fmt::format("{:f} - point of interest", point{1, 2});
    //
    // the range will contain "f} - point of interest". The formatter should
    // parse specifiers until '}' or the end of the range. In this example
    // the formatter should parse the 'f' specifier and return an iterator
    // pointing to '}'.

    // Please also note that this character range may be empty, in case of
    // the "{}" format string, so therefore you should check ctx.begin()
    // for equality with ctx.end().

    // Parse the presentation format and store it in the formatter:

    auto it = ctx.begin(), end = ctx.end();
    char tpl_[20] = "";
    if (it != end) {
      int i = 0;
      for (; *it != '}'; ++it) {
        tpl_[i] = *it;
        ++i;
      }
      tpl_[i] = '}';
      if (tpl_[1] != '}') {
        tpl[1] = ':';
        std::copy(tpl_, &tpl_[i + 1], &tpl[2]);
      }
    }

    // Check if reached the end of the range:
    if (it != end && *it != '}')
      fmt::detail::throw_format_error(
          "invalid format, can't reach the end of the expression");
    // Return an iterator past the end of the parsed range:
    return it;
  }

  fmt::format_context::iterator format(const Type &v,
                                       fmt::format_context &ctx) const {
    *ctx.out()++ = '(';
    for (glm::length_t i = 0; i < L; ++i) {
      fmt::format_to(ctx.out(), fmt::runtime(tpl), v[i]);
      if (i + 1 != L) {
        *ctx.out()++ = ',';
        *ctx.out()++ = ' ';
      }
    }
    *ctx.out()++ = ')';

    return ctx.out();
  }
};
