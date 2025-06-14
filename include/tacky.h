#pragma once
#include "assert.h"
#include "ast.h"
#include <unordered_map>

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

struct Binary: Inst{
  ast::OpType op;
  Ptr<Val> src_1;
  Ptr<Val> src_2;
  Ptr<Val> dst; 
  Binary(ast::OpType op, 
    Ptr<Val> src_1,
    Ptr<Val> src_2,
    Ptr<Val> dst,
    Ptr<Inst> next=0):Inst(next), op(op), src_1(src_1), src_2(src_2), dst(dst){}
  void print()override;
};

struct Label: Inst{
  unsigned number;
  Label(unsigned number, Ptr<Inst> next=0): Inst(next), number(number){};
  void print()override;
};

struct Jmp: Inst{
  unsigned label;
  Jmp(unsigned label, Ptr<Inst> next=0): Inst(next), label(label){};
  void print()override;
};

struct Jnz: Inst{
  unsigned label;
  Ptr<Val> cond;
  Jnz(unsigned label, Ptr<Val> cond, Ptr<Inst> next=0):Inst(next), label(label), cond(cond){};
  void print()override;
};

struct Jz: Inst{
  unsigned label;
  Ptr<Val> cond;
  Jz(unsigned label, Ptr<Val> cond, Ptr<Inst> next=0):Inst(next), label(label), cond(cond){};
  void print()override;
};

struct Copy: Inst{
  Ptr<Val> src;
  Ptr<Val> dst;
  Copy(Ptr<Val> src, Ptr<Val> dst, Ptr<Inst> next=0): Inst(next), src(src), dst(dst){};
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
  std::unordered_map<char const*, unsigned> var_tmp_map{};
  // for tmp var
  unsigned get_tmp_val(){
    return tmp_val++;
  }

  // for named var
  unsigned get_tmp_val(char const* name){
    if(var_tmp_map.count(name)) return var_tmp_map[name];
    var_tmp_map[name] = tmp_val;
    return tmp_val++;
  }

  unsigned label_number{0};
  unsigned get_label(){
    return label_number++;
  }

  Ptr<Inst> cur_insts{0};
  Ptr<Inst> cur_insts_tail{0};
  void append_cur_insts(Ptr<Inst>);

  Ptr<Var> build_logic_and(Ptr<ast::Binary>);
  Ptr<Var> build_logic_or(Ptr<ast::Binary>);
public:
  Ptr<Program> build(Ptr<ast::BaseNode>);
  Ptr<Program> build(Ptr<ast::Program>);
  Ptr<FunctionDef> build(Ptr<ast::FunctionDef>);
  void build(Ptr<ast::Block>);
  void build(Ptr<ast::Decl>);
  void build(Ptr<ast::Stmt>);
  void build(Ptr<ast::RetStmt>);
  void build(Ptr<ast::CompoundStmt>);
  void build(Ptr<ast::IfStmt>);
  void build(Ptr<ast::ExprStmt>);
  Ptr<Val> build(Ptr<ast::Expr>);
  Ptr<Var> build(Ptr<ast::Unary>);
  Ptr<Var> build(Ptr<ast::Binary>);
  Ptr<Val> build(Ptr<ast::Assign>);
  Ptr<Var> build(Ptr<ast::Condition>);
  Ptr<Var> build(Ptr<ast::Var>);
  Ptr<Constant> build(Ptr<ast::Constant>);
};

}
}