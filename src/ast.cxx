#include "ast.hxx"
#include <cstdio>
#include "utils.hxx"

namespace niubcc{
namespace ast{

std::string
Unary::print(){
  return utils::fmt(
    "Unary(\noperator=%s\nexpr=%s\n)",
    map_op_name[static_cast<unsigned>(op_type)],
    expr->print().c_str()
  );
}

std::string
Binary::print(){
  return utils::fmt(
    "Binary(\noperator=%s\nlhs=%s\nrhs=%s\n)",
    map_op_name[static_cast<unsigned>(op_type)],
    lhs->print().c_str(),
    rhs->print().c_str()
  );
}

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