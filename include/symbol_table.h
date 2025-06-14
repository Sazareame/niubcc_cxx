#pragma once
#include <string_view>
#include <unordered_map>
#include <memory>

namespace niubcc{

class SymbolTable;

class Scope{
friend class SymbolTable;
  std::unordered_map<std::string_view, std::shared_ptr<std::string> > table{};
  std::shared_ptr<Scope> outer;
public:
  Scope(std::shared_ptr<Scope> outer=0): outer(outer){};
};

class SymbolTable{
  std::shared_ptr<Scope> cur_scope;
  unsigned tmp_num{0};
  std::string make_tmp_name(char const* name, unsigned len);
public:
  SymbolTable(){
    cur_scope = std::make_shared<Scope>();
  }
  std::shared_ptr<std::string> lookup_and_add(char const* name, unsigned len);
  std::shared_ptr<std::string> lookup_and_get(char const* name, unsigned len);
  void enter_scope();
  void leave_scope();
};

}
