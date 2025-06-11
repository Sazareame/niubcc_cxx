#pragma once
#include <memory>
#include <vector>
#include "tacky.hxx"

namespace niubcc{

namespace codegen{
enum class OperandType{
  Imm,
  Reg,
  Mem,
};

struct Operand{
  OperandType type;
  std::string repr;
  Operand(OperandType type, std::string const& repr): type(type), repr(repr){}
};

class AsmGenerator{
private:
  std::vector<std::string> codes{};
  unsigned stack_allocated{0};
  unsigned allocate_stack(unsigned tmp){
    unsigned stack_pos = (tmp + 1) * 4;
    stack_allocated = stack_allocated > stack_pos ? stack_allocated : stack_pos;
    return stack_pos;
  }

  Operand get_operand(Ptr<ir::Val>);

  void emit_mov(Operand const&, Operand const&);
  void emit_cmp(Operand const&, Operand const&);

  std::string alloc_tmp_reg(bool flag)const{
    if(flag) return "%%r10d";
    return "%%r11d";
  }

  void emit_bin_op(std::string const&, Operand const&, Operand const&);

  void gen_mul_inst(Ptr<ir::Binary>);
  void gen_div_inst(Ptr<ir::Binary>);
  void gen_bin_inst(Ptr<ir::Binary>, std::string const&);
  void gen_cond_inst(Ptr<ir::Binary>);
  void gen_cond_inst(Ptr<ir::Unary>);

public:
  void generate(Ptr<ir::Base>);
  void generate(Ptr<ir::Program>);
  void generate(Ptr<ir::FunctionDef>);
  void generate(Ptr<ir::Inst>);
  void generate(Ptr<ir::Unary>);
  void generate(Ptr<ir::Ret>);
  void generate(Ptr<ir::Binary>);
  void generate(Ptr<ir::Copy>);
  void generate(Ptr<ir::Jmp>);
  void generate(Ptr<ir::Jnz>);
  void generate(Ptr<ir::Jz>);
  void generate(Ptr<ir::Label>);
  std::string generate(Ptr<ir::Val>);
  std::string generate(Ptr<ir::Var>);
  std::string generate(Ptr<ir::Constant>);

  void emie_code(char const* filename)const;
};
}
}