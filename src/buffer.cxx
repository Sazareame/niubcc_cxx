#include "buffer.hxx"
#include <cstring>
#include "utils.hxx"

namespace niubcc{

std::string
BufferError::to_string()const{
  return utils::fmt("Buffer Error with raw file: %s, %s\n", filename, msg);
}

Buffer::Buffer(Buffer const& oth){
  capacity = oth.capacity;
  data = new char[capacity];
  std::memcpy(data, oth.data, capacity * sizeof(char));
}

Buffer::Buffer(Buffer&& oth) noexcept{
  capacity = oth.capacity;
  data = oth.data;
  oth.data = 0;
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
  oth.data = 0;
  return *this;
}

Expected<Buffer, BufferError>
Buffer::from_file(char const* file_name){
  FILE* file = std::fopen(file_name, "r");
  if(!file) return BufferError("cannot open file", file_name);

  std::fseek(file, 0, SEEK_END);
  auto size = std::ftell(file);
  if(size < 0){
    std::fclose(file);
    return BufferError("failed to get file size", file_name);
  }
  std::rewind(file);

  Buffer buffer(size + 1);
  auto read = std::fread(buffer.data, 1, size, file);

  std::fclose(file);
  if(read != static_cast<unsigned long>(size)){
    return BufferError("failed to read file", file_name);
  }

  *buffer.data = EOF;
  return buffer;
}

char const*
Buffer::get_start(){
  return data;
}

unsigned long
Buffer::get_length()const{
  return capacity - 1;
}

}