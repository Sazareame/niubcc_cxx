#pragma once
#include <string>

namespace niubcc{
namespace utils{
  struct Pos{
    unsigned col;
    unsigned line;
  };
  std::string fmt(char const* fmt, ...);
  bool string_equal(char const*, char const*, unsigned);
}
}