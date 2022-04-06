#include "lib.hpp"

auto main() -> int
{
  library lib;

  return lib.name == "CygNES" ? 0 : 1;
}
