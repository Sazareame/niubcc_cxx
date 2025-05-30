#include "parser.hxx"
#include "utils.hxx"

namespace niubcc{

std::string
ParseError::to_string()const{
  return utils::fmt("Parse Error at line %u, col %u: %s", line, col, msg);
}

void
Parser::error_handler(ParseError const& err){
  std::string msg = err.to_string();
  std::fwrite(msg.data(), 1, msg.size(), stderr);
  std::fputc('\n', stderr);
  std::terminate();
}

Parser::Parser(Lexer& lexer){
  tok_pos = 0;
  tokens.swap(lexer.get_tokens_out());
}

bool
Parser::match(TokenType type){
  if(!tokens[tok_pos].is(type)) return false;
  ++tok_pos;
  return true;
}

Ptr<ast::Program>
Parser::parse(){
  auto res = parse_program();
  if(res.is_err()) res.handle_err(Parser::error_handler);
  auto root = res.unwrap();
  return root;
}

Expected<Ptr<ast::Program>, ParseError>
Parser::parse_program(){
  auto res = parse_funcdef();
  if(res.is_err()) return res.unwrap_err();
  return std::make_shared<ast::Program>(res.unwrap());
}

Expected<Ptr<ast::FunctionDef>, ParseError>
Parser::parse_funcdef(){
  if(!match(TokenType::kw_int))
    return ParseError("Expected keyword int", 
      get_cur_tok_col(), get_cur_tok_line());

  if(!match(TokenType::ident))
    return ParseError("Expected function name",
      get_cur_tok_col(), get_cur_tok_line());
  char const* name = tokens[tok_pos - 1].get_name();
  unsigned name_len = tokens[tok_pos - 1].get_name_len();

  if(!match(TokenType::lparen, TokenType::rparen, TokenType::punct_lbrace))
    return ParseError("Syntax error", get_cur_tok_col(), get_cur_tok_line());

  auto body = parse_retstmt();
  if(body.is_err()) return body.unwrap_err();

  if(!match(TokenType::punct_rbrace))
    return ParseError("Expected }", get_cur_tok_col(), get_cur_tok_line());

  return std::make_shared<ast::FunctionDef>(name, name_len, body.unwrap());
}

Expected<Ptr<ast::RetStmt>, ParseError>
Parser::parse_retstmt(){
  if(!match(TokenType::kw_ret))
    return ParseError("Expected keyword return",
      get_cur_tok_col(), get_cur_tok_line());
  
  if(!match(TokenType::li_int))
    return ParseError("Expected integer literal",
      get_cur_tok_col(), get_cur_tok_line());
  
  char const* val = tokens[tok_pos - 1].get_name();
  unsigned val_len = tokens[tok_pos - 1].get_name_len();

  if(!match(TokenType::punct_semicol))
    return ParseError("Expcted semicolumn",
      get_cur_tok_col(), get_cur_tok_line());
  
  Ptr<ast::Constant> exp = std::make_shared<ast::Constant>(val, val_len);
  return std::make_shared<ast::RetStmt>(exp);
}

}