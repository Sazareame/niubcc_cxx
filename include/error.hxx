#pragma once
#include <string>
#include <memory>
#include <type_traits>

namespace niubcc{

class Error{
protected:
  char const* msg;
public:
  Error(char const* msg): msg(msg){};
  Error(Error const&) = delete;
  Error(Error&& oth) noexcept = default;
  Error& operator=(Error const&) = delete;
  Error& operator=(Error&&) = delete;
  virtual ~Error() = default;

  virtual std::string to_string() const = 0;
};

template<class T, class E,
class=std::enable_if_t<std::is_base_of_v<Error, E> > >
class Expected{
private:
  bool has_error;
  union{
    T value;
    std::unique_ptr<E> error;
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
  Expected(Expected<T, E>&& oth) noexcept: has_error(oth.has_error){
    if(has_error) new(&error) std::unique_ptr(std::move(oth.error));
    else new(&value) T(std::move(oth.value));
  }
  Expected& operator=(Expected<T, E>&& oth){
    if(this == &oth) return *this;
    this->~Expected();
    new (this) Expected(std::move(oth));
    return  *this;
  }

  T unwrap(){
    if(has_error){
      fprintf(stderr, "Unwrap a Expected which contains Error");
      std::terminate();
    }
    return std::move(value);
  }

  bool is_ok() const{
    return !has_error;
  }

  bool is_err() const{
    return has_error;
  }

  template<class F, class=std::enable_if_t<std::is_invocable_v<F, E> > >
  void handle_err(F&& f){
    if(!has_error) return;
    f(*error);
  }

};

}