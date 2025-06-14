#include "utils.h"
#include "symbol_table.h"
#include <cstring>

namespace niubcc{

std::string
SymbolTable::make_tmp_name(char const* name, unsigned len){
  return niubcc::utils::fmt("%.*s.%u", len, name, tmp_num++);
}

std::shared_ptr<std::string>
SymbolTable::lookup_and_add(const char* name, unsigned len){
  std::string_view look_name(name, len);
  if(cur_scope->table.count(look_name)) return 0;
  cur_scope->table[look_name] = std::make_shared<std::string>(make_tmp_name(name, len));
  return cur_scope->table[look_name];
}

std::shared_ptr<std::string>
SymbolTable::lookup_and_get(const char* name, unsigned len){
  std::string_view look_name(name, len);
  auto scope = cur_scope;
  while(scope){
    if(scope->table.count(look_name)) return scope->table[look_name];
    scope = scope->outer;
  }
  return 0;
}

void
SymbolTable::enter_scope(){
  cur_scope = std::make_shared<Scope>(cur_scope);
}

void
SymbolTable::leave_scope(){
  if(!cur_scope->outer) return;
  cur_scope = cur_scope->outer;
}

}