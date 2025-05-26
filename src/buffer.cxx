#include "buffer.hxx"
#include <cstring>

namespace niubcc{

std::string
BufferError::to_string()const{
  auto length = std::snprintf(
    0, 0, "Buffer Error with raw file: %s, %s\n", filename, msg);
  std::string formatted(length, '\0');
  std::snprintf(
    formatted.data(),
    length + 1, "Buffer Error with raw file: %s, %s\n", filename, msg
  );
  return formatted;
}

Buffer::Buffer(Buffer const& oth){
  capacity = oth.capacity;
  data = new char[capacity];
  std::memcpy(data, oth.data, capacity * sizeof(char));
}

Buffer::Buffer(Buffer&& oth) noexcept{
  capacity = oth.capacity;
  data = oth.data;
}

Buffer&
Buffer::operator=(Buffer const& oth){
  if(this == &oth) return *this;
  if(data) delete[] data;
  data = new char[capacity];
  std::memcpy(data, oth.data, capacity * sizeof(char));
  return *this;
}

Buffer&
Buffer::operator=(Buffer&& oth) noexcept{
  if(this == &oth) return *this;
  capacity = oth.capacity;
  if(data) delete[] data;
  data = oth.data;
  return *this;
}

Buffer
Buffer::from_file(char const* file_name){
  FILE* file = std::fopen(file_name, "r");
}

char const*
Buffer::get_start(){

}

}