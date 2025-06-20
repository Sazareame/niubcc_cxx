#include "tacky.h"
#include "utils.h"
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
  build(node->blocks);
  return std::make_shared<FunctionDef>(node->name, node->name_len, cur_insts);
}

void
AstBuilder::build(Ptr<ast::CompoundStmt> node){
  auto cur = node->blocks;
  while(cur){
    build(cur);
    cur = cur->next;
  }
}

void
AstBuilder::build(Ptr<ast::Block> node){
  if(auto p = std::dynamic_pointer_cast<ast::Stmt>(node))
    build(p);
  if(auto p = std::dynamic_pointer_cast<ast::Decl>(node))
    build(p);
  return;
}

void
AstBuilder::build(Ptr<ast::Decl> node){
  if(!node->init) return;
  auto decled = std::make_shared<Var>(get_tmp_val(node->name));
  auto init = build(node->init);
  auto copy = std::make_shared<Copy>(init, decled);
  append_cur_insts(copy);
}

void
AstBuilder::build(Ptr<ast::Stmt> node){
  if(node->label)
    append_cur_insts(std::make_shared<Label>(get_label(node->label)));

  if(auto p = std::dynamic_pointer_cast<ast::RetStmt>(node))
    build(p);
  if(auto p = std::dynamic_pointer_cast<ast::ExprStmt>(node))
    build(p);
  if(auto p = std::dynamic_pointer_cast<ast::IfStmt>(node))
    build(p);
  if(auto p = std::dynamic_pointer_cast<ast::CompoundStmt>(node))
    build(p);
  if(auto p = std::dynamic_pointer_cast<ast::GotoStmt>(node))
    build(p);
  return; 
}

void
AstBuilder::build(Ptr<ast::RetStmt> node){
  auto val = build(node->ret_val);
  auto ret = std::make_shared<Ret>(val);
  append_cur_insts(ret);
}

void
AstBuilder::build(Ptr<ast::IfStmt> node){
  auto cond_res = build(node->condition);
  auto end_l = std::make_shared<Label>(get_label());
  auto else_l = node->else_stmt ? std::make_shared<Label>(get_label()) : 0;

  if(else_l)
    append_cur_insts(std::make_shared<Jz>(else_l->number, cond_res));
  else
    append_cur_insts(std::make_shared<Jz>(end_l->number, cond_res));

  build(node->then_stmt);
  append_cur_insts(std::make_shared<Jmp>(end_l->number));

  if(else_l){
    append_cur_insts(else_l);
    build(node->else_stmt);
  }

  append_cur_insts(end_l);
}

void
AstBuilder::build(Ptr<ast::GotoStmt> node){
  append_cur_insts(std::make_shared<Jmp>(get_label(node->label)));
}

void
AstBuilder::build(Ptr<ast::ExprStmt> node){
  build(node->expr);
}

Ptr<Val>
AstBuilder::build(Ptr<ast::Expr> node){
  if(auto p = std::dynamic_pointer_cast<ast::Constant>(node))
    return build(p);
  if(auto p = std::dynamic_pointer_cast<ast::Var>(node))
    return build(p);
  if(auto p = std::dynamic_pointer_cast<ast::Unary>(node))
    return build(p);
  if(auto p = std::dynamic_pointer_cast<ast::Binary>(node))
    return build(p);
  if(auto p = std::dynamic_pointer_cast<ast::Assign>(node))
    return build(p);
  if(auto p = std::dynamic_pointer_cast<ast::Condition>(node))
    return build(p);
  return 0;
}

Ptr<Val>
AstBuilder::build(Ptr<ast::Assign> node){
  auto dst = build(node->dst);
  auto src = build(node->src);
  append_cur_insts(std::make_shared<Copy>(src, dst));
  return dst;
}

Ptr<Var>
AstBuilder::build(Ptr<ast::Condition> node){
  auto res = std::make_shared<Var>(get_tmp_val());
  auto cond_res = build(node->condition);
  auto false_l = std::make_shared<Label>(get_label());
  auto end_l = std::make_shared<Label>(get_label());

  append_cur_insts(std::make_shared<Jz>(false_l->number, cond_res));

  auto true_val = build(node->true_val);
  append_cur_insts(std::make_shared<Copy>(true_val, res));
  append_cur_insts(std::make_shared<Jmp>(end_l->number));

  append_cur_insts(false_l);

  auto false_val = build(node->false_val);
  append_cur_insts(std::make_shared<Copy>(false_val, res));

  append_cur_insts(end_l);

  return res;
}

Ptr<Constant>
AstBuilder::build(Ptr<ast::Constant> node){
  return std::make_shared<Constant>(node->value, node->value_len);
}

Ptr<Var>
AstBuilder::build(Ptr<ast::Var> node){
  return std::make_shared<Var>(get_tmp_val(node->name));
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
AstBuilder::build_logic_and(Ptr<ast::Binary> node){
  auto dest = std::make_shared<Var>(get_tmp_val());
   
  auto false_val = std::make_shared<Constant>("0", 1);
  auto true_val = std::make_shared<Constant>("1", 1);
   
  auto false_l = std::make_shared<Label>(get_label());
  auto end_l = std::make_shared<Label>(get_label());

  auto src_1 = build(node->lhs);                                      // ins of e1
  append_cur_insts(std::make_shared<Jz>(false_l->number, src_1));     // jz (e1) false_l

  auto src_2 = build(node->rhs);                                      // ins of e2
  append_cur_insts(std::make_shared<Jz>(false_l->number, src_2));     // jz (e2) false_l

  append_cur_insts(std::make_shared<Copy>(true_val, dest));           // mov $1, dst
  append_cur_insts(std::make_shared<Jmp>(end_l->number));             // jmp end
  append_cur_insts(false_l);                                          // false_l:
  append_cur_insts(std::make_shared<Copy>(false_val, dest));          // mov $0, dst
  append_cur_insts(end_l);                                            // end:
  return dest;
}

Ptr<Var>
AstBuilder::build_logic_or(Ptr<ast::Binary> node){
  auto dest = std::make_shared<Var>(get_tmp_val());
   
  auto false_val = std::make_shared<Constant>("0", 1);
  auto true_val = std::make_shared<Constant>("1", 1);
   
  auto true_l = std::make_shared<Label>(get_label());
  auto end_l = std::make_shared<Label>(get_label());

  auto src_1 = build(node->lhs);                                      // ins of e1
  append_cur_insts(std::make_shared<Jnz>(true_l->number, src_1));     // jnz (e1) true_l

  auto src_2 = build(node->rhs);                                      // ins of e2
  append_cur_insts(std::make_shared<Jnz>(true_l->number, src_2));     // jz (e2) false_l

  append_cur_insts(std::make_shared<Copy>(false_val, dest));          // mov $0, dst
  append_cur_insts(std::make_shared<Jmp>(end_l->number));             // jmp end
  append_cur_insts(true_l);                                           // true_l:
  append_cur_insts(std::make_shared<Copy>(true_val, dest));           // mov $1, dst
  append_cur_insts(end_l);                                            // end:
  return dest;
}

Ptr<Var>
AstBuilder::build(Ptr<ast::Binary> node){
  if(node->op_type == ast::OpType::op_or)
    return build_logic_or(node);
  if(node->op_type == ast::OpType::op_and)
    return build_logic_and(node);
  // Note that we evaluate expr form right hand
  // but for some operations (such as sub and div), the lhs should come first.
  auto src_2 = build(node->rhs);
  auto src_1 = build(node->lhs);
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

void
Label::print(){
  std::cout << utils::fmt("Lable(.L%u)\n", number);
}

void
Jmp::print(){
  std::cout << utils::fmt("Jmp(.L%u)\n", label);
}

void
Jnz::print(){
  std::cout << utils::fmt("Jnz(.L%u, %s)\n", label, cond->print().c_str());
}

void
Jz::print(){
  std::cout << utils::fmt("Jz(.L%u, %s)\n", label, cond->print().c_str());
}

void
Copy::print(){
  std::cout << utils::fmt("Copy(%s, %s)\n", src->print().c_str(), dst->print().c_str());
}

}
}