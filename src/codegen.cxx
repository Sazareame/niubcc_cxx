#include "codegen.hxx"
#include "utils.hxx"
#include <fstream>


namespace niubcc {
namespace codegen{

Operand
AsmGenerator::get_operand(Ptr<ir::Val> val){
  if(auto p = std::dynamic_pointer_cast<ir::Var>(val))
    return Operand(OperandType::Mem, generate(p));
  if(auto p = std::dynamic_pointer_cast<ir::Constant>(val))
    return Operand(OperandType::Imm, generate(p));
}

void
AsmGenerator::emit_mov(Operand const& src, Operand const& dst){
  if(src.type == OperandType::Mem && dst.type == OperandType::Mem){
    codes.emplace_back(utils::fmt("movl\t%s, %r10d\n", src.repr.c_str()));
    codes.emplace_back(utils::fmt("movl\t%r10d, %s\n", dst.repr.c_str()));
  }else{
    codes.emplace_back(utils::fmt("movl\t%s, %s\n", src.repr.c_str(), dst.repr.c_str()));
  }
}

void
AsmGenerator::emit_cmp(Operand const& src, Operand const& dst){
  if(dst.type == OperandType::Imm){
    codes.emplace_back(utils::fmt("movl\t%s, %r11d\n", dst.repr.c_str()));
    codes.emplace_back(utils::fmt("cmpl\t%s, %r11d\n", src.repr.c_str()));
  }else if(src.type == OperandType::Mem && dst.type == OperandType::Mem){
    codes.emplace_back(utils::fmt("movl\t%s, %r10d\n", src.repr.c_str()));
    codes.emplace_back(utils::fmt("cmpl\t%r10d, %s\n", dst.repr.c_str()));
  }else{
    codes.emplace_back(utils::fmt("cmpl\t%s, %s\n", src.repr.c_str(), dst.repr.c_str()));
  }
}

void
AsmGenerator::emit_bin_op(std::string const& op, Operand const& src, Operand const& dst){
  codes.emplace_back(
    utils::fmt("%s\t%s, %s\n", op.c_str(), src.repr.c_str(), dst.repr.c_str()));
}

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
  codes.emplace_back("pushq\t%rbp\n");
  codes.emplace_back("movq\t%rsp, %rbp\n");
  codes.emplace_back("");
  auto alloc_stack = codes.size() - 1;
  generate(node->instructions);
  codes[alloc_stack] = utils::fmt("subq\t$%u, %rsp\n", stack_allocated);
}
void 
AsmGenerator::generate(Ptr<ir::Inst> node){
  if(!node) return;
  if(auto p = std::dynamic_pointer_cast<ir::Unary>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Ret>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Binary>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Label>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Jz>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Jnz>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Jmp>(node))
    generate(p);
  if(auto p = std::dynamic_pointer_cast<ir::Copy>(node))
    generate(p);

  generate(node->next);
}

void 
AsmGenerator::generate(Ptr<ir::Unary> node){
  if(node->op == ast::OpType::op_not)
    return gen_cond_inst(node);
  auto src_op = get_operand(node->src);
  auto dst_op = get_operand(node->dst);

  emit_mov(src_op, dst_op);

  switch(node->op){
    case ast::OpType::op_bitnot: 
      codes.emplace_back(utils::fmt("notl\t%s\n", dst_op.repr.c_str())); break;
    case ast::OpType::op_minus:
      codes.emplace_back(utils::fmt("negl\t%s\n", dst_op.repr.c_str())); break;
    default: assert(0);
  }
}

void
AsmGenerator::gen_mul_inst(Ptr<ir::Binary> node){
  auto src1_op = get_operand(node->src_1);
  auto src2_op = get_operand(node->src_2);
  auto dst_op = get_operand(node->dst);
  
  Operand temp_reg_op(OperandType::Reg, "%r11d");
  
  emit_mov(src1_op, temp_reg_op);
  emit_bin_op("imul", src2_op, temp_reg_op);
  emit_mov(temp_reg_op, dst_op);
}

void
AsmGenerator::gen_div_inst(Ptr<ir::Binary> node){
  auto src1_op = get_operand(node->src_1);
  auto src2_op = get_operand(node->src_2);
  auto dst_op = get_operand(node->dst);
  
  emit_mov(src1_op, Operand(OperandType::Reg, "%eax"));
  codes.emplace_back("cdq\n");
  
  if(src2_op.type == OperandType::Imm){
    emit_mov(src2_op, Operand(OperandType::Reg, "%r10d"));
    codes.emplace_back("idivl\t %r10d\n");
  }else{
    codes.emplace_back(utils::fmt("idivl\t%s\n", src2_op.repr.c_str()));
  }
  
  if(node->op == ast::OpType::op_slash) {
    emit_mov(Operand(OperandType::Reg, "%eax"), dst_op);
  }else{
    emit_mov(Operand(OperandType::Reg, "%edx"), dst_op);
  }
}

void
AsmGenerator::gen_bin_inst(Ptr<ir::Binary> node, std::string const& op_name){
  auto src1_op = get_operand(node->src_1);
  auto src2_op = get_operand(node->src_2);
  auto dst_op = get_operand(node->dst);

  emit_mov(src1_op, dst_op);

  auto actual_src_op = src2_op;
  if(src2_op.type == OperandType::Mem && dst_op.type == OperandType::Mem){
    actual_src_op = Operand(OperandType::Reg, "%r10d");
    emit_mov(src2_op, actual_src_op);
  }

  emit_bin_op(op_name, actual_src_op, dst_op);
}

void
AsmGenerator::generate(Ptr<ir::Binary> node){
  std::string op_name;
  switch(node->op){
    case ast::OpType::op_asterisk: gen_mul_inst(node); return;
    case ast::OpType::op_slash:
    case ast::OpType::op_percent: gen_div_inst(node); return;
    case ast::OpType::op_plus:   op_name = "addl"; goto bin_inst;
    case ast::OpType::op_minus:  op_name = "subl"; goto bin_inst;
    case ast::OpType::op_bitand: op_name = "andl"; goto bin_inst;
    case ast::OpType::op_bitor:  op_name = "orl";  goto bin_inst;
    case ast::OpType::op_bitxor: op_name = "xorl"; goto bin_inst;
    case ast::OpType::op_lshift: op_name = "shll"; goto bin_inst;
    case ast::OpType::op_rshift: op_name = "sarl"; goto bin_inst;
    default: gen_cond_inst(node); return;
  }

bin_inst:
  gen_bin_inst(node, op_name);
}

void
AsmGenerator::gen_cond_inst(Ptr<ir::Binary> node){
  // cmpl src2, src1
  // movl $0, dst
  // setflag dst
  auto src1 = get_operand(node->src_1);
  auto src2 = get_operand(node->src_2);
  emit_cmp(src2, src1); // src1 and src2 could be both memory.
  auto dst = generate(node->dst);
  codes.emplace_back(utils::fmt("movl\t$0, %s\n", dst.c_str()));

  std::string instuction;
  switch(node->op){
    case ast::OpType::op_eq: instuction = "sete"; break;
    case ast::OpType::op_ne: instuction = "setne"; break;
    case ast::OpType::op_le: instuction = "setle"; break;
    case ast::OpType::op_ge: instuction = "setge"; break;
    case ast::OpType::op_lt: instuction = "setl"; break;
    case ast::OpType::op_gt: instuction = "setg"; break;
    default: assert(0 && "unreachabel");
  }

  codes.emplace_back(utils::fmt("%s\t%s\n", instuction.c_str(), dst.c_str()));
}

void
AsmGenerator::gen_cond_inst(Ptr<ir::Unary> node){
  // definitely logic not
  // cmpl $0, src
  // movel $0, dst
  // sete dst
  codes.emplace_back(utils::fmt("cmpl\t$0, %s\n", generate(node->src).c_str()));
  auto dst = generate(node->dst);
  codes.emplace_back(utils::fmt("movl\t$0, %s\n", dst.c_str()));
  codes.emplace_back(utils::fmt("sete\t%s\n", dst.c_str()));
}

void
AsmGenerator::generate(Ptr<ir::Jmp> node){
  codes.emplace_back(utils::fmt("jmp\t.L%u\n", node->label));
}

void
AsmGenerator::generate(Ptr<ir::Jnz> node){
  codes.emplace_back(utils::fmt("cmpl\t$0, %s\n", generate(node->cond).c_str()));
  codes.emplace_back(utils::fmt("jne\t.L%u\n", node->label));
}

void
AsmGenerator::generate(Ptr<ir::Jz> node){
  codes.emplace_back(utils::fmt("cmpl\t$0, %s\n", generate(node->cond).c_str()));
  codes.emplace_back(utils::fmt("je\t.L%u\n", node->label));
}

void
AsmGenerator::generate(Ptr<ir::Copy> node){
  auto src = get_operand(node->src);
  auto dst = get_operand(node->dst);
  emit_mov(src, dst);
}

void
AsmGenerator::generate(Ptr<ir::Label> node){
  codes.emplace_back(utils::fmt(".L%u:\n", node->number));
}

void 
AsmGenerator::generate(Ptr<ir::Ret> node){
  codes.emplace_back(utils::fmt("movl\t%s, %%eax\n", 
    generate(node->val).c_str()));
  codes.emplace_back("movq\t%rbp, %rsp\n");
  codes.emplace_back("popq\t%rbp\n");
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
  return utils::fmt("-%u(%rbp)", allocate_stack(node->number));
}

std::string
AsmGenerator::generate(Ptr<ir::Constant> node){
  return utils::fmt("$%.*s", node->val_len, node->val);
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