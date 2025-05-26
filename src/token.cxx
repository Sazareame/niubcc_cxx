#include "lexer.hxx"

namespace niubcc{

void
Token::init(){
  type = TokenType::unknown;
  p_text = 0;
  len = 0;
  raw_literal = 0;
  addtional_len = 0;
}

}