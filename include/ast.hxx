#pragma once
#include <memory>
#include "lexer.hxx"

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

struct BaseNode{
  BaseNode() = default;
  virtual ~BaseNode() = default;
  virtual std::string print() = 0;
};

struct Stmt: BaseNode{
  Stmt() = default;
  virtual ~Stmt() = default;
  virtual std::string print() = 0;
};

struct Expr: BaseNode{
  Expr() = default;
  virtual ~Expr() = default;
  virtual std::string print() = 0;
};

struct Unary: Expr{
  OpType op_type;
  Ptr<Expr> expr;
  Unary(OpType op_type, Ptr<Expr> expr): op_type(op_type), expr(expr){}
  std::string print()override;
};

struct Binary: Expr{
  OpType op_type;
  Ptr<Expr> lhs;
  Ptr<Expr> rhs;
  Binary(OpType op_type, Ptr<Expr> lhs, Ptr<Expr> rhs):
  op_type(op_type), lhs(lhs), rhs(rhs){}
  std::string print()override;
};

struct Program: BaseNode{
  Ptr<FunctionDef> funcdef;
  Program(Ptr<FunctionDef> funcdef): funcdef(funcdef){};
  std::string print()override;
};

struct FunctionDef: BaseNode{
  char const* name;
  unsigned name_len;
  Ptr<RetStmt> stmt;
  FunctionDef(char const* name, unsigned name_len, Ptr<RetStmt> stmt)
  :name(name), name_len(name_len), stmt(stmt){};
  std::string print()override;
};

struct RetStmt: Stmt{
  Ptr<Expr> ret_val;
  RetStmt(Ptr<Expr> ret_val): ret_val(ret_val){};
  std::string print()override;
};

struct Constant: Expr{
  char const* value;
  unsigned value_len;
  Constant(char const* value, unsigned value_len)
  :value(value), value_len(value_len){};
  std::string print()override;
};

}
}