#pragma once
#include <string>
#include <memory>

namespace niubcc{

class Error{
private:
  std::string msg;
public:
  Error(std::string msg): msg(std::move(msg)){};
  Error(Error const&) = delete;
  Error(Error&& oth) noexcept: msg(std::move(oth.msg)){};
  Error& operator=(Error const&) = delete;
  Error& operator=(Error&&) = delete;
  virtual ~Error() = default;

  virtual std::string to_string() const = 0;
};

template<class T, class E,
typename=std::enable_if_t<std::is_base_of_v<Error, E> > >
class Expected{
private:
  bool has_error;
  union{
    T value;
    std::unique_ptr<Error> error;
  };
public:
  Expected(T&& value):
  value(std::move(value)), has_error(false){};
  Expected(E err):
  error(std::make_unique<E>(std::move(err))), has_error(true){};
  ~Expected(){
    if(!has_error) value.~T();
  }
  Expected(Expected<T, E> const&) = delete;
  Expected& operator=(Expected<T, E> const&) = delete;
  Expected(Expected<T, E>&& oth) = delete;
  Expected& operator=(Expected<T, E>&&) = delete;

  T unwrap(){
    if(has_error){
      fprintf(stderr, "Unwrap a Expected which contains Error");
      std::terminate();
    }
    return std::move(value);
  }

  bool is_ok() const{
    return has_error;
  }

};

}