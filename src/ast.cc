#include "ast.h"
#include <cstdio>
#include "utils.h"

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
  return utils::fmt(
    "FunctionDef(\n%sname=%.*s\n%sbody=%s\n%s)",
    indent.c_str(),
    name_len, name,
    indent.c_str(),
    blocks->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
CompoundStmt::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  std::string res = utils::fmt(
    "Stmts(\n%s[",
    indent.c_str()
  );
  auto cur_block = blocks;
  while(cur_block){
    res += std::move(cur_block->print(depth + 1)); // explictly choose move assignment
    cur_block = cur_block->next;
  }
  res += utils::fmt("%s]\n%s)", indent.c_str(), end_indent.c_str());
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
IfStmt::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');

  std::string else_repr{"none"};
  if(else_stmt) else_repr = else_stmt->print(depth + 1);
  return utils::fmt(
    "IfStmt(\n%scondition=%s\n%sthen=%s\n%selse=%s\n%s)",
    indent.c_str(),
    condition->print(depth + 1).c_str(),
    indent.c_str(),
    then_stmt->print(depth + 1).c_str(),
    indent.c_str(),
    else_repr.c_str(),
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
Condition::print(unsigned depth=0){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt(
    "Condition(\n%scondition=%s\n%strue=%s\n%sfalse=%s\n%s)",
    indent.c_str(),
    condition->print(depth + 1).c_str(),
    indent.c_str(),
    true_val->print(depth + 1).c_str(),
    indent.c_str(),
    false_val->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
Var::print(unsigned depth=0){
  return utils::fmt("Var(%s)", name->c_str());
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
    name->c_str(),
    indent.c_str(),
    init_expr.c_str(),
    end_indent.c_str()
  );
}

std::string
DoStmt::print(unsigned depth){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt("Do(\n%sstmts=%s\n%scondition=%s\n%s)",
    indent.c_str(),
    stmt->print(depth + 1).c_str(),
    indent.c_str(),
    condition->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
WhileStmt::print(unsigned depth){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');
  return utils::fmt("While(\n%sstmts=%s\n%scondition=%s\n%s)",
    indent.c_str(),
    stmt->print(depth + 1).c_str(),
    indent.c_str(),
    condition->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
ForStmtInit::print(unsigned depth){
  if(decls){
    auto indent = std::string(depth + 1, '\t');
    auto end_indent = std::string(depth, '\t');
    std::string res = utils::fmt("Decls(\n%s[", indent.c_str());
    auto cur = decls;
    while(cur){
      res += std::move(cur->print(depth + 1));
      cur = std::dynamic_pointer_cast<Decl>(cur->next);
    }
    res += utils::fmt("%s]\n%s)", indent.c_str(), end_indent.c_str());
    return res;
  }

  return expr->print(depth + 1);
}

std::string
ForStmt::print(unsigned depth){
  auto indent = std::string(depth + 1, '\t');
  auto end_indent = std::string(depth, '\t');

  std::string init_repr{"none"};
  std::string cond_repr{"none"};
  std::string post_repr{"none"};
  if(init) init_repr = init->print(depth + 1);
  if(condition) cond_repr = condition->print(depth + 1);
  if(post) post_repr = post->print(depth + 1);

  return utils::fmt("For(\n%sinit=%s\n%scondition=%s\n%spost=%s\n%sstmt=%s\n%s)",
    indent.c_str(),
    init_repr.c_str(),
    indent.c_str(),
    cond_repr.c_str(),
    indent.c_str(),
    post_repr.c_str(),
    indent.c_str(),
    stmt->print(depth + 1).c_str(),
    end_indent.c_str()
  );
}

std::string
Break::print(unsigned depth){
  return utils::fmt("Break()\n");
}

std::string
Continue::print(unsigned depth){
  return utils::fmt("Continue()\n");
}

}
}