#include "utils.h"
#include <cstdarg>
#include <cstring>

namespace niubcc{
namespace utils{

std::string fmt(char const* fmt, ...){
  va_list args1, args2;
  va_start(args1, fmt);
  va_copy(args2, args1);

  auto len = vsnprintf(0, 0, fmt, args1);
  va_end(args1);

  std::string buf(len, '\0');
  vsnprintf(buf.data(), len + 1, fmt, args2);

  va_end(args2);
  return buf;
}

bool
string_equal(char const* s1, char const* s2, unsigned len){
  return strlen(s2) == len && memcmp(s1, s2, len) == 0;
}

}
}