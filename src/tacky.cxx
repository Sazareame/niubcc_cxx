#include "tacky.hxx"
#include "utils.hxx"
#include <iostream>

namespace niubcc{
namespace ir{

void
AstBuilder::append_cur_insts(Ptr<Inst> inst){
  if(!cur_insts){
    cur_insts_tail = cur_insts = inst;
    return;
  }
  cur_insts_tail->next = inst;
  cur_insts_tail = inst;
}

Ptr<Program>
AstBuilder::build(Ptr<ast::BaseNode> node){
  if(auto p = std::dynamic_pointer_cast<ast::Program>(node))
    return build(p);
  return 0;
}

Ptr<Program>
AstBuilder::build(Ptr<ast::Program> node){
  return std::make_shared<Program>(build(node->funcdef));
}

Ptr<FunctionDef>
AstBuilder::build(Ptr<ast::FunctionDef> node){
  build(node->stmt);
  return std::make_shared<FunctionDef>(node->name, node->name_len, cur_insts);
}

void
AstBuilder::build(Ptr<ast::RetStmt> node){
  auto val = build(node->ret_val);
  auto ret = std::make_shared<Ret>(val);
  append_cur_insts(ret);
}

Ptr<Val>
AstBuilder::build(Ptr<ast::Expr> node){
  if(auto p = std::dynamic_pointer_cast<ast::Constant>(node))
    return build(p);
  else if(auto p = std::dynamic_pointer_cast<ast::Unary>(node))
    return build(p);
  else if(auto p = std::dynamic_pointer_cast<ast::Binary>(node))
    return build(p);
  return 0;
}

Ptr<Constant>
AstBuilder::build(Ptr<ast::Constant> node){
  return std::make_shared<Constant>(node->value, node->value_len);
}

Ptr<Var>
AstBuilder::build(Ptr<ast::Unary> node){
  auto src = build(node->expr);
  auto dest = std::make_shared<Var>(get_tmp_val());
  auto inst = std::make_shared<Unary>(node->op_type, src, dest);
  append_cur_insts(inst);
  return dest;
}

Ptr<Var>
AstBuilder::build(Ptr<ast::Binary> node){
  auto src_1 = build(node->rhs);
  auto src_2 = build(node->lhs);
  auto dest = std::make_shared<Var>(get_tmp_val());
  auto inst = std::make_shared<Binary>(node->op_type, src_1, src_2, dest);
  append_cur_insts(inst);
  return dest;
}

void
Program::print(){
  std::cout << utils::fmt("Program:\n");
  funcdef->print();
}

void
FunctionDef::print(){
  std::cout << utils::fmt("Function %.*s:\n", name_len, name);
  auto p = instructions;
  while(p){
    p->print();
    p = p->next;
  }
}

void
Ret::print(){
  std::cout << utils::fmt("Ret(%s)\n", val->print().c_str());
}

std::string
Var::print(){
  return utils::fmt("Var(tmp.%u)", number);
}

std::string
Constant::print(){
  return utils::fmt("Constant(%.*s)", val_len, val);
}

void
Unary::print(){
  std::cout << utils::fmt("Unary(%s, %s, %s)\n",
    ast::map_op_name[static_cast<unsigned>(op)],
    src->print().c_str(), dst->print().c_str());
}

void
Binary::print(){
  std::cout << utils::fmt("Binary(%s, %s, %s, %s)\n",
    ast::map_op_name[static_cast<unsigned>(op)],
    src_1->print().c_str(),
    src_2->print().c_str(),
    dst->print().c_str());
}

}
}