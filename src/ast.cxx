#include "ast.hxx"
#include <cstdio>
#include "utils.hxx"

namespace niubcc{
namespace ast{

std::string
Unary::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt(
    "Unary(\n%soperator=%s\n%sexpr=%s\n%s)",
    indent.c_str(),
    map_op_name[static_cast<unsigned>(op_type)],
    indent.c_str(),
    expr->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
Binary::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt(
    "Binary(\n%soperator=%s\n%slhs=%s\n%srhs=%s\n%s)",
    indent.c_str(),
    map_op_name[static_cast<unsigned>(op_type)],
    indent.c_str(),
    lhs->print(depth + 1).c_str(),
    indent.c_str(),
    rhs->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
Program::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt(
    "Program(\n%s%s\n%s)",
    indent.c_str(),
    funcdef->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
FunctionDef::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  std::string res = utils::fmt(
    "FunctionDef(\n%sname=%.*s\n%sbody=[",
    indent.c_str(),
    name_len, name,
    indent.c_str()
  );
  auto cur_block = blocks;
  while(cur_block){
    res += std::move(cur_block->print(depth + 1)); // explictly choose move assignment
    cur_block = cur_block->next;
  }
  res += utils::fmt("]\n%s)", end_indent.c_str());
  return res;
}

std::string
RetStmt::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt(
    "ReturnStmt(\n%s%s\n%s)",
    indent.c_str(),
    ret_val->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
Constant::print(unsigned depth=0){
  return utils::fmt(
    "Conatant(%.*s)",
    value_len, value
  );
}

std::string
Var::print(unsigned depth=0){
  return utils::fmt("Var(%s)", name);
}

std::string
Assign::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt("Assign(\n%ssrc=%s\n%sdst=%s\n%s)",
    indent.c_str(),
    src->print(depth + 1).c_str(),
    indent.c_str(),
    dst->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
NullStmt::print(unsigned depth=0){
  return "NullStmt()";
}

std::string
ExprStmt::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt("ExprStmt(\n%sexpr=%s\n%s)",
    indent.c_str(),
    expr->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
Decl::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  std::string init_expr{"none"};
  if(init) init_expr = init->print(depth + 1);
  return utils::fmt("Declaration(\n%sname=%s\n%sinit=%s\n%s)",
    indent.c_str(),
    name,
    indent.c_str(),
    init_expr.c_str(),
    end_indent.c_str()
  );
}

}
}