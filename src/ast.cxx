#include "ast.hxx"
#include <cstdio>
#include "utils.hxx"

namespace niubcc{
namespace ast{

std::string
Program::print(){
  return utils::fmt(
    "Program(\n%s\n)",
    funcdef->print().c_str()
  );
}

std::string
FunctionDef::print(){
  return utils::fmt(
    "FunctionDef(\nname=%.*s\nbody=%s\n)",
    name_len, name,
    stmt->print().c_str()
  );
}

std::string
RetStmt::print(){
  return utils::fmt(
    "ReturnStmt(\n%s\n)",
    ret_val->print().c_str()
  );
}

std::string
Constant::print(){
  return utils::fmt(
    "Conatant(%.*s)",
    value_len, value
  );
}

}
}