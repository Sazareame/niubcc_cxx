#pragma once
#include <cstdio>
#include <error.hxx>

namespace niubcc{

class BufferError: public Error{
private:
  char const* filename;
public:
  BufferError(char const* msg, char const* filename):
  Error(msg), filename(filename){};
  std::string to_string()const override;
};

class Buffer{
private:
  char* data;
  unsigned long capacity;
  Buffer(unsigned capacity): capacity(capacity), data(new char[capacity]){};
public:
  ~Buffer(){
    if(data) delete [] data;
  }
  Buffer(Buffer const& oth);
  Buffer(Buffer&& oth) noexcept;
  Buffer& operator=(Buffer const& oth);
  Buffer& operator=(Buffer&& oth) noexcept;
  
  static Expected<Buffer, BufferError> from_file(char const* file_name);
  char const* get_start();
  unsigned long get_length()const;
};

}