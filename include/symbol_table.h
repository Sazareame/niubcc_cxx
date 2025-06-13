#pragma once
#include <string_view>
#include <unordered_map>


class SymbolTable{
  std::unordered_map<std::string_view, std::string> table{};
  unsigned tmp_num{0};
  std::string make_tmp_name(char const* name, unsigned len);
public:
  SymbolTable() = default;
  char const* lookup_and_add(char const* name, unsigned len);
  char const* lookup_and_get(char const* name, unsigned len);
};