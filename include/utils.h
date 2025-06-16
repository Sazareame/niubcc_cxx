#pragma once
#include <string>

namespace niubcc{
namespace utils{

  std::string fmt(char const* fmt, ...);
  bool string_equal(char const*, char const*, unsigned);
}
}