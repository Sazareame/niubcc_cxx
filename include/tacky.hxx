#pragma once
#include "assert.h"
#include "ast.hxx"

namespace niubcc{
namespace ir{

struct FunctionDef;
struct Inst;
struct Ret;
struct Val;
struct Unary;
struct Constant;
struct Var;

struct Base{
  virtual ~Base() = default;
};

struct Program: Base{
  Ptr<FunctionDef> funcdef;
  Program(Ptr<FunctionDef> funcdef):funcdef(funcdef){};
  void print();
};

struct FunctionDef: Base{
  char const* name;
  unsigned name_len;
  Ptr<Inst> instructions;
  FunctionDef(char const* name, unsigned name_len, Ptr<Inst> instructions)
  :name(name), name_len(name_len), instructions(instructions){};
  void print();
};

struct Inst: Base{
  Ptr<Inst> next;
  virtual ~Inst() = default;
  Inst(Ptr<Inst> next){
    assert(next.get() != this);
    this->next = next;
  }
  virtual void print() = 0;
};

struct Ret: Inst{
  Ptr<Val> val;
  Ret(Ptr<Val> val, Ptr<Inst> next=0): Inst(next), val(val){};
  void print()override;
};

struct Val: Base{
  virtual ~Val() = default;
  virtual std::string print() = 0;
};

struct Unary: Inst{
  ast::OpType op;
  Ptr<Val> src;
  Ptr<Val> dst;
  Unary(ast::OpType op, Ptr<Val> src, Ptr<Val> dst, Ptr<Inst> next=0):
  Inst(next), op(op), src(src), dst(dst){}
  void print()override;
};

struct Var: Val{
  unsigned number;
  Var(unsigned number): number(number){};
  std::string print()override;
};

struct Constant: Val{
  char const* val;
  unsigned val_len;
  Constant(char const* val, unsigned val_len): val(val), val_len(val_len){}
  std::string print()override;
};

class AstBuilder{
  unsigned tmp_val{0};
  unsigned get_tmp_val(){
    return tmp_val++;
  }
  Ptr<Inst> cur_insts{0};
  Ptr<Inst> cur_insts_tail{0};
  void append_cur_insts(Ptr<Inst>);
public:
  Ptr<Program> build(Ptr<ast::BaseNode>);
  Ptr<Program> build(Ptr<ast::Program>);
  Ptr<FunctionDef> build(Ptr<ast::FunctionDef>);
  void build(Ptr<ast::RetStmt>);
  Ptr<Val> build(Ptr<ast::Expr>);
  Ptr<Var> build(Ptr<ast::Unary>);
  Ptr<Constant> build(Ptr<ast::Constant>);
};

}
}