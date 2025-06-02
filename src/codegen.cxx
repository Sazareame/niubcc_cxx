#include "codegen.hxx"
#include "utils.hxx"
#include <fstream>


namespace niubcc {
namespace codegen{

void 
AsmGenerator::generate(Ptr<ir::Base> node){
  if(auto p = std::dynamic_pointer_cast<ir::Program>(node))
    generate(p);
}

void 
AsmGenerator::generate(Ptr<ir::Program> node){
  generate(node->funcdef);
  codes.emplace_back(".section .note.GNU-stack,\"\",@progbits");
}

void 
AsmGenerator::generate(Ptr<ir::FunctionDef> node){
  codes.emplace_back(utils::fmt("\t.globl %.*s\n", node->name_len, node->name));
  codes.emplace_back(utils::fmt("%.*s:\n", node->name_len, node->name));
  codes.emplace_back("pushq\t\%rbp\n");
  codes.emplace_back("movq\t\%rsp, \%rbp\n");
  codes.emplace_back("");
  auto alloc_stack = codes.size() - 1;
  generate(node->instructions);
  codes[alloc_stack] = utils::fmt("subq\t$%u, \%rsp\n", stack_allocated);
}
void 
AsmGenerator::generate(Ptr<ir::Inst> node){
  if(!node) return;
  if(auto p = std::dynamic_pointer_cast<ir::Unary>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Ret>(node))
    generate(p);
  generate(node->next);
}

void 
AsmGenerator::generate(Ptr<ir::Unary> node){
  auto dst = generate(node->src, node->dst);
  switch(node->op){
    case ast::OpType::op_bitnot: 
      codes.emplace_back(utils::fmt("notl\t%s\n", dst.c_str())); break;
    case ast::OpType::op_minus:
      codes.emplace_back(utils::fmt("negl\t%s\n", dst.c_str())); break;
    default: assert(0);
  }
}

void 
AsmGenerator::generate(Ptr<ir::Ret> node){
  codes.emplace_back(utils::fmt("movl\t%s, %%eax\n", 
    generate(node->val).c_str()));
  codes.emplace_back("movq\t\%rbp, \%rsp\n");
  codes.emplace_back("popq\t\%rbp\n");
  codes.emplace_back("ret\n");
}

std::string
AsmGenerator::generate(Ptr<ir::Val> node){
  if(auto p = std::dynamic_pointer_cast<ir::Var>(node))
    return generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Constant>(node))
    return generate(p);
  return 0;
}

std::string
AsmGenerator::generate(Ptr<ir::Var> node){
  return utils::fmt("-%u(\%rbp)", allocate_stack(node->number));
}

std::string
AsmGenerator::generate(Ptr<ir::Constant> node){
  return utils::fmt("$%.*s", node->val_len, node->val);
}

std::string
AsmGenerator::generate(Ptr<ir::Val> src, Ptr<ir::Val> dst){
  auto p = std::dynamic_pointer_cast<ir::Var>(src);
  auto q = std::dynamic_pointer_cast<ir::Var>(dst);
  if(p && q){
    codes.emplace_back(utils::fmt("movl\t%s, \%r10d\n", generate(p).c_str()));
    auto dst_val = generate(q);
    codes.emplace_back(utils::fmt("movl\t\%r10d, %s\n", dst_val.c_str()));
    return dst_val;
  }
  auto dst_val = generate(dst);
  codes.emplace_back(utils::fmt(
    "movl\t%s, %s\n", generate(src).c_str(), dst_val.c_str()));
  return dst_val;
}

void
AsmGenerator::emie_code(char const* filename)const{
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