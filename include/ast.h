#pragma once
#include <memory>
#include "lexer.h"

namespace niubcc{
template<class T> using Ptr = std::shared_ptr<T>;

namespace ast{

#define TOK(X, S)
#define OP(X, S, P, B, U) X,
enum class OpType{
#include "token.def"
};
#undef TOK
#undef OP

#define TOK(X, STR)
#define OP(X, STR, P, B, U) STR,
inline char const* map_op_name[] = {
#include "token.def"
};
#undef TOK
#undef OP

struct FunctionDef;
struct Stmt;
struct RetStmt;
struct Constant;
struct Expr;

struct BaseNode{
  BaseNode() = default;
  virtual ~BaseNode() = default;
  virtual std::string print(unsigned) = 0;
};

struct Block: BaseNode{
  Ptr<Block> next;
  Block(Ptr<Block> next): next(next){};
  virtual std::string print(unsigned) = 0;
};

struct Stmt: Block{
  Stmt(Ptr<Block> next=0): Block(next){};
  virtual ~Stmt() = default;
  virtual std::string print(unsigned) = 0;
};

struct Decl: Block{
  char const* name; // C Style String
  Ptr<Expr> init;
  Decl(char const* name, Ptr<Expr> init=0, Ptr<Block> next=0)
  :Block(next), name(name), init(init){};
  std::string print(unsigned)override;
};

struct Expr: BaseNode{
  Expr() = default;
  virtual ~Expr() = default;
  virtual std::string print(unsigned) = 0;
};

struct Unary: Expr{
  OpType op_type;
  Ptr<Expr> expr;
  Unary(OpType op_type, Ptr<Expr> expr): op_type(op_type), expr(expr){}
  std::string print(unsigned)override;
};

struct Binary: Expr{
  OpType op_type;
  Ptr<Expr> lhs;
  Ptr<Expr> rhs;
  Binary(OpType op_type, Ptr<Expr> lhs, Ptr<Expr> rhs):
  op_type(op_type), lhs(lhs), rhs(rhs){}
  std::string print(unsigned)override;
};

struct Var: Expr{
  char const* name; // C Sylte String
  Var(char const* name): name(name){}
  std::string print(unsigned)override;
};

struct Assign: Expr{
  Ptr<Expr> src;
  Ptr<Expr> dst;
  Assign(Ptr<Expr> src, Ptr<Expr> dst): src(src), dst(dst){};
  std::string print(unsigned)override;
};

struct Program: BaseNode{
  Ptr<FunctionDef> funcdef;
  Program(Ptr<FunctionDef> funcdef): funcdef(funcdef){};
  std::string print(unsigned)override;
};

struct FunctionDef: BaseNode{
  char const* name;
  unsigned name_len;
  Ptr<Block> blocks;
  FunctionDef(char const* name, unsigned name_len, Ptr<Block> blocks)
  :name(name), name_len(name_len), blocks(blocks){};
  std::string print(unsigned)override;
};

struct RetStmt: Stmt{
  Ptr<Expr> ret_val;
  RetStmt(Ptr<Expr> ret_val): ret_val(ret_val){};
  std::string print(unsigned)override;
};

struct NullStmt: Stmt{
  NullStmt() = default;
  std::string print(unsigned)override;
};

struct ExprStmt: Stmt{
  Ptr<Expr> expr;
  ExprStmt(Ptr<Expr> expr): expr(expr){};
  std::string print(unsigned)override;
};

struct IfStmt: Stmt{
  Ptr<Expr> condition;
  Ptr<Stmt> then_stmt;
  Ptr<Stmt> else_stmt;
  IfStmt(Ptr<Expr> condition, Ptr<Stmt> then_stmt, Ptr<Stmt> else_stmt=0)
  :condition(condition), then_stmt(then_stmt), else_stmt(else_stmt){}
  std::string print(unsigned)override;
};

struct Constant: Expr{
  char const* value;
  unsigned value_len;
  Constant(char const* value, unsigned value_len)
  :value(value), value_len(value_len){};
  std::string print(unsigned)override;
};

struct Condition: Expr{
  Ptr<Expr> condition;
  Ptr<Expr> true_val;
  Ptr<Expr> false_val;
  Condition(Ptr<Expr> condition, Ptr<Expr> true_val, Ptr<Expr> false_val)
  :condition(condition), true_val(true_val), false_val(false_val){};
  std::string print(unsigned)override;
};

}
}