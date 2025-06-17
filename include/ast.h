#pragma once
#include <memory>
#include <string>
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
struct CompoundStmt;

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
  Ptr<std::string> label{0};
  Stmt(Ptr<Block> next=0): Block(next){};
  virtual ~Stmt() = default;
  virtual std::string print(unsigned) = 0;
};

struct Decl: Block{
  Ptr<std::string> name;
  Ptr<Expr> init;
  Decl(Ptr<std::string> name, Ptr<Expr> init=0, Ptr<Block> next=0)
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
  Ptr<std::string> name;
  Var(Ptr<std::string> name): name(name){}
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
  Ptr<CompoundStmt> blocks;
  FunctionDef(char const* name, unsigned name_len, Ptr<CompoundStmt> blocks)
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

struct DoStmt: Stmt{
  Ptr<Stmt> stmt;
  Ptr<Expr> condition;
  DoStmt(Ptr<Stmt> stmt, Ptr<Expr> condition): stmt(stmt), condition(condition){};
  std::string print(unsigned)override;
};

struct WhileStmt: Stmt{
  Ptr<Stmt> stmt;
  Ptr<Expr> condition;
  WhileStmt(Ptr<Stmt> stmt, Ptr<Expr> condition): stmt(stmt), condition(condition){};
  std::string print(unsigned)override;
};

struct ForStmtInit: BaseNode{
  Ptr<Decl> decls;
  Ptr<Expr> expr;
  ForStmtInit(Ptr<Decl> decls): decls(decls), expr(0){};
  ForStmtInit(Ptr<Expr> expr): decls(0), expr(expr){};
  std::string print(unsigned)override;
};

struct ForStmt: Stmt{
  Ptr<ForStmtInit> init;
  Ptr<Expr> condition;
  Ptr<Expr> post;
  Ptr<Stmt> stmt;
  ForStmt(Ptr<ForStmtInit> init, Ptr<Expr> condition, Ptr<Expr> post, Ptr<Stmt> stmt)
  : init(init), condition(condition), post(post), stmt(stmt){};
  std::string print(unsigned)override;
};

struct Break: Stmt{
  std::string print(unsigned)override;
};

struct Continue: Stmt{
  std::string print(unsigned)override;
};

struct GotoStmt: Stmt{
  Ptr<std::string> target;
  GotoStmt(Ptr<std::string> target): target(target){};
  std::string print(unsigned)override;
};

struct CompoundStmt: Stmt{
  Ptr<Block> blocks;
  CompoundStmt(Ptr<Block> blocks): blocks(blocks){};
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