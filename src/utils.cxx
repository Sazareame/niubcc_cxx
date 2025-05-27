#include "utils.hxx"
#include <cstdarg>

namespace niubcc{
namespace utils{

std::string fmt(char const* fmt, ...){
  va_list args;
  va_start(args, fmt);

  auto len = vsnprintf(0, 0, fmt, args);
  std::string buf(len, '\0');
  vsnprintf(buf.data(), len + 1, fmt, args);

  va_end(args);
  return buf;
}

}
}