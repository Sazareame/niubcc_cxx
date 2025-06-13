#include "lexer.h"
#include "utils.h"
#include <cstring>
#include <cstdio>

namespace niubcc{

#define TOK(X, S) S,
#define OP(X, S, P, B, U) S,
char const* Token::token_name_map[]{
#include "token.def"
};
#undef TOK
#undef OP

bool
Token::is_keyword()const{
  return (std::strncmp(
    token_name_map[static_cast<unsigned short>(type)], "Kw", 2) == 0);
}
bool
Token::is_literal()const{
  return (std::strncmp(
    token_name_map[static_cast<unsigned short>(type)], "Li", 2) == 0
  );
}

std::optional<std::string>
Token::fmt()const{
  if(type == TokenType::unknown){
    return std::nullopt;
  }
  if(is_keyword()) return utils::fmt(
    "Keyword@%s (%u, %u)",
    token_name_map[static_cast<unsigned short>(type)],
    line, col
  );
  else if(is_literal()) return utils::fmt(
    "Literal@%s@%.*s (%u, %u)",
    token_name_map[static_cast<unsigned short>(type)],
    addtional_len, raw_literal, line, col);
  else if(is_ident()) return utils::fmt(
    "Identifier@%s@%.*s (%u, %u)",
    token_name_map[static_cast<unsigned short>(type)],
    addtional_len, raw_indent, line, col);
  else return utils::fmt(
    "Token@%s (%u, %u)",
    token_name_map[static_cast<unsigned short>(type)],
    line, col
  );

}

void
Token::init(){
  type = TokenType::unknown;
  p_text = 0;
  len = 0;
  raw_literal = 0;
  addtional_len = 0;
}

}