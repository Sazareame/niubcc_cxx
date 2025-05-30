#pragma once
#include <memory>
#include <vector>
#include "ast.hxx"

namespace niubcc{

namespace codegen{

class AsmNodeBuilder{
private:
  std::vector<std::string> codes{};
public:
  void build(Ptr<ast::BaseNode> node);
  void build_programe(Ptr<ast::Program> node);
  void build_funcdef(Ptr<ast::FunctionDef> node);
  void build_retstmt(Ptr<ast::RetStmt> node);
  void build_constant(Ptr<ast::Constant> node);
  void emie_code(char const* filename)const;
};
}
}