#include "utils.hxx"
#include "symbol_table.hxx"
#include <cstring>

std::string
SymbolTable::make_tmp_name(char const* name, unsigned len){
  return niubcc::utils::fmt("%.*s.%u", len, name, tmp_num++);
}

char const*
SymbolTable::lookup_and_add(const char* name, unsigned len){
  std::string_view look_name(name, len);
  if(table.count(look_name)) return 0;
  table[look_name] = std::move(make_tmp_name(name, len));
  return table[look_name].c_str();
}

char const*
SymbolTable::lookup_and_get(const char* name, unsigned len){
  std::string_view look_name(name, len);
  if(!table.count(look_name)) return 0;
  return table[look_name].c_str();
}