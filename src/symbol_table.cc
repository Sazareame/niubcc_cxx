#include "utils.h"
#include "symbol_table.h"
#include <cstring>

namespace niubcc{

std::string
SymbolTable::make_label_name(char const* name, unsigned len){
  return niubcc::utils::fmt(".userdefl%.*s.%u", len, name, label_num++);
}

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

std::shared_ptr<std::string>
SymbolTable::define_label(const char* name, unsigned len, utils::Pos pos){
  std::string_view look_name(name, len);
  if(labels.count(look_name)){
    if(labels[look_name].is_defined) return 0;
    labels[look_name].is_defined = true;
  }else{
    labels[look_name] = LabelEntry{
      .pos = pos,
      .name = std::make_shared<std::string>(make_label_name(name, len)),
      .is_defined = true,
    };
  }
  return labels[look_name].name;
}

std::shared_ptr<std::string>
SymbolTable::add_label(char const* name, unsigned len, utils::Pos pos){
  std::string_view look_name(name, len);
  if(labels.count(look_name)) return labels[look_name].name;
  labels[look_name] = LabelEntry{
    .pos = pos,
    .name = std::make_shared<std::string>(make_label_name(name, len)),
    .is_defined = false,
  };
  return labels[look_name].name;
}

std::optional<utils::Pos>
SymbolTable::resolve_all_labels(){
  for(auto& entry: labels)
    if(!entry.second.is_defined) return entry.second.pos;
  return std::nullopt; 
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