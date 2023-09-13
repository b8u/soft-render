#pragma once

#include <iosfwd>
#include <string>

#include <libraytracer/export.hpp>

namespace raytracer {
// Print a greeting for the specified name into the specified
// stream. Throw std::invalid_argument if the name is empty.
//
LIBRAYTRACER_SYMEXPORT void say_hello(std::ostream &, const std::string &name);
} // namespace raytracer
