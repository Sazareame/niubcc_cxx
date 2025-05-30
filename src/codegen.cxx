#include "codegen.hxx"
#include "utils.hxx"
#include <fstream>


namespace niubcc {
namespace codegen{

void 
AsmNodeBuilder::build(Ptr<ast::BaseNode> node){
  if(auto p = std::dynamic_pointer_cast<ast::Program>(node))
    build_programe(p);
  return;
}

void 
AsmNodeBuilder::build_programe(Ptr<ast::Program> node){
  build_funcdef(node->funcdef);
  codes.emplace_back("\t.section .note.GNU-stack,\"\",@progbits\n");
}

void 
AsmNodeBuilder::build_funcdef(Ptr<ast::FunctionDef> node){
  codes.emplace_back(utils::fmt(
    "\t.globl %.*s\n%.*s:\n",
    node->name_len, node->name, node->name_len, node->name));
  build_retstmt(node->stmt);
}

void 
AsmNodeBuilder::build_retstmt(Ptr<ast::RetStmt> node){
  codes.emplace_back("\tmovl\t");
  build_constant(node->ret_val);
  codes.emplace_back(", \%eax\n");
  codes.emplace_back(utils::fmt("\tret\n"));
}

void 
AsmNodeBuilder::build_constant(Ptr<ast::Constant> node){
  codes.emplace_back(utils::fmt("$%.*s", node->value_len, node->value));
}

void
AsmNodeBuilder::emie_code(char const* filename)const{
  std::ofstream file(filename);
  if(!file){
    fprintf(stderr, "cannot create file %s.\n", filename);
    std::terminate();
  }
  for(auto& code: codes)
    file << code;
}

}
}