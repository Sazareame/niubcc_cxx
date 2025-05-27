#include <cctype>
#include <cstring>
#include <cstdio>
#include "lexer.hxx"
#include "utils.hxx"

namespace niubcc{

void
 Lexer::err_handler(LexerError const& err){
  std::string msg = err.to_string();
  std::fwrite(msg.data(), 1, msg.size(), stderr);
  std::fputc('\n', stderr);
  std::terminate();
 }

void
Lexer::tokenize(){
  while(1){
    auto res = lex_one_token();
    if(res.is_err())
      res.handle_err(Lexer::err_handler);
    if(!res.unwrap()) break;
  }
}

Expected<bool, LexerError>
Lexer::lex_one_token(){
  cur_ptr = text_ptr;
  if(!skip_whitespace()) return false;

  if(tok_pos == tok_max_len){
    tok_max_len *= 2;
    tokens.reserve(tok_max_len);
  }
  Token& token = tokens[tok_pos++];
  token.init();

  if(std::isdigit(*cur_ptr)) return lex_number(token);
  if(std::isalpha(*cur_ptr)) return lex_ident_or_kw(token);

  token.p_text = cur_ptr;
  token.len = 1;
  token.col = col;
  token.line = line;
  switch(*cur_ptr){
    case '(': token.type = TokenType::lparen; break;
    case ')': token.type = TokenType::rparen; break;
    case '{': token.type = TokenType::punct_lbrace; break;
    case '}': token.type = TokenType::punct_rbrace; break;
    case ';': token.type = TokenType::punct_semicol; break;
    default: return LexerError("Unexpected character", col, line);
  }
  ++text_ptr;
  ++col;
  return true;
}

bool
Lexer::skip_whitespace(){
  while(std::isspace(*cur_ptr)){
    if(*cur_ptr++ == '\n'){
      ++line;
      col = 1;
    }else{
      ++col;
    }
  }
  text_ptr = cur_ptr;
  return *cur_ptr == EOF ? false : true;
}

bool
Lexer::lex_number(Token& token){
  while(std::isdigit(*cur_ptr)) ++cur_ptr;
  token.p_text = text_ptr;
  token.len = static_cast<unsigned>(cur_ptr - text_ptr);
  token.col = col;
  token.line = line;
  token.type = TokenType::li_int;
  token.raw_literal = text_ptr;
  token.addtional_len = token.len;
  text_ptr = cur_ptr;
  col += token.len;
  return *cur_ptr == EOF ? false : true;
}

bool
Lexer::lex_ident_or_kw(Token& token){
  while(std::isalnum(*cur_ptr)) ++cur_ptr;
  token.p_text = text_ptr;
  token.len = static_cast<unsigned>(cur_ptr - text_ptr);
  token.col = col;
  token.line = line;
  if(std::strncmp(text_ptr, "int", token.len) == 0)
    token.type = TokenType::kw_int;
  else if(std::strncmp(text_ptr, "return", token.len) == 0)
    token.type = TokenType::kw_ret;
  else if(std::strncmp(text_ptr, "void", token.len) == 0)
    token.type = TokenType::kw_void;
  else{
    token.type = TokenType::ident;
    token.raw_indent = text_ptr;
    token.addtional_len = token.len;
  }
  col += token.len;
  text_ptr = cur_ptr;
  return *cur_ptr == EOF ? false : true;
}

std::vector<Token> const&
Lexer::get_tokens()const{
  return tokens;
}

std::string
LexerError::to_string()const{
  return utils::fmt("Lexical Error at line %u, col %d: %s\n", line, col, msg);
}

void 
Lexer::display_all_tokens()const{
  for(auto const& token : tokens){
    try{
      fprintf(stdout, "%s\n", token.fmt()->c_str());
    }catch(...){
      break;
    }
  }
}

}