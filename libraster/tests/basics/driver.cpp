#include <sstream>
#include <stdexcept>

#include <libraster/version.hpp>
#include <libraster/raster.hpp>

#undef NDEBUG
#include <cassert>

int main ()
{
  using namespace std;
  using namespace raster;

  // Basics.
  //
  {
    ostringstream o;
    say_hello (o, "World");
    assert (o.str () == "Hello, World!\n");
  }

  // Empty name.
  //
  try
  {
    ostringstream o;
    say_hello (o, "");
    assert (false);
  }
  catch (const invalid_argument& e)
  {
    assert (e.what () == string ("empty name"));
  }
}
