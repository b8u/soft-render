#pragma once

#include <iosfwd>
#include <string>

#include <libraster/export.hpp>

namespace raster
{
  // Print a greeting for the specified name into the specified
  // stream. Throw std::invalid_argument if the name is empty.
  //
  LIBRASTER_SYMEXPORT void
  say_hello (std::ostream&, const std::string& name);
}
