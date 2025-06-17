#pragma once
#include <string_view>
#include <unordered_map>
#include <memory>
#include <optional>
#include "utils.h"
#include "error.h"

namespace niubcc{

class SymbolTable;

class Scope{
friend class SymbolTable;
  std::unordered_map<std::string_view, std::shared_ptr<std::string> > table{};
  std::shared_ptr<Scope> outer;
public:
  Scope(std::shared_ptr<Scope> outer=0): outer(outer){};
};

struct LabelEntry{
  utils::Pos pos;
  std::shared_ptr<std::string> name;
  bool is_defined;
};

class SymbolTable{
  std::shared_ptr<Scope> cur_scope;
  // Only need single label map, because label is in function scope.
  std::unordered_map<std::string_view, LabelEntry> labels{};
  bool in_func{false}; // For judge a legal label.

  unsigned tmp_num{0};
  unsigned label_num{0};
  std::string make_tmp_name(char const* name, unsigned len);
  std::string make_label_name(char const* name, unsigned len);
public:
  SymbolTable(){
    cur_scope = std::make_shared<Scope>();
  }
  std::shared_ptr<std::string> lookup_and_add(char const* name, unsigned len);
  std::shared_ptr<std::string> lookup_and_get(char const* name, unsigned len);
  std::shared_ptr<std::string> define_label(char const* name, unsigned len, utils::Pos pos);
  std::shared_ptr<std::string> add_label(char const* name, unsigned len, utils::Pos pos);
  std::optional<utils::Pos> resolve_all_labels();
  void enter_scope();
  void leave_scope();
  void call_func(){in_func = true;}
  void ret_func(){
    in_func = false;
    labels.clear();
  }
  bool is_in_func()const{return in_func;}
};

}
