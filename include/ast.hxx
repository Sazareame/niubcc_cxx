#pragma once
#include <memory>

namespace niubcc{
template<class T> using Ptr = std::shared_ptr<T>;

namespace ast{

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
  Ptr<Constant> ret_val;
  RetStmt(Ptr<Constant> ret_val): ret_val(ret_val){};
  std::string print()override;
};

struct Constant: BaseNode{
  char const* value;
  unsigned value_len;
  Constant(char const* value, unsigned value_len)
  :value(value), value_len(value_len){};
  std::string print()override;
};

inline void foo(std::shared_ptr<BaseNode> nd){

}

inline void foo2(std::shared_ptr<Program> nd){
  foo(nd);
}

}
}