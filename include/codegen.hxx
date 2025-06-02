#pragma once
#include <memory>
#include <vector>
#include "tacky.hxx"

namespace niubcc{

namespace codegen{

class AsmGenerator{
private:
  std::vector<std::string> codes{};
  unsigned stack_allocated{0};
  unsigned allocate_stack(unsigned tmp){
    unsigned stack_pos = (tmp + 1) * 4;
    stack_allocated = stack_allocated > stack_pos ? stack_allocated : stack_pos;
    return stack_pos;
  }
  bool gen_mov_if_need(Ptr<ir::Val>, Ptr<ir::Val>);
public:
  void generate(Ptr<ir::Base>);
  void generate(Ptr<ir::Program>);
  void generate(Ptr<ir::FunctionDef>);
  void generate(Ptr<ir::Inst>);
  void generate(Ptr<ir::Unary>);
  void generate(Ptr<ir::Ret>);
  std::string generate(Ptr<ir::Val>);
  std::string generate(Ptr<ir::Var>);
  std::string generate(Ptr<ir::Constant>);

  void emie_code(char const* filename)const;
};
}
}