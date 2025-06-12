#include "parser.hxx"
#include "utils.hxx"
#include <cassert>

namespace niubcc{

#define TOK(X, S) 
#define OP(X, S, P, B, U) P,
unsigned Parser::op_precedence[]{
#include "token.def"
};
#undef TOK
#undef OP

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

#define TOK(X, R) case TokenType::X: return false;
#define OP(X, R, P, B, U) case TokenType::X: return B;
bool
Parser::is_next_binary_op()const{
  switch(get_cur_tok_type()){
    #include "token.def"
  }
}
#undef OP
#undef TOK

#define TOK(X, R) case TokenType::X: return false;
#define OP(X, R, P, B, U) case TokenType::X: return U;
bool
Parser::is_next_unary_op()const{
  switch(get_cur_tok_type()){
    #include "token.def"
  }
}
#undef OP
#undef TOK

#define TOK(X, R)
#define OP(X, R, P, B, U) case TokenType::X: return ast::OpType::X;
ast::OpType
Parser::convert_token_to_op(TokenType tokentype)const{
  switch(tokentype){
    #include "token.def"  
  }
  assert("unreachable && 0");
}
#undef TOK
#undef OP

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

  auto blocks_res = parse_block();
  // parse_block() return null ptr if the body is empty, e.g., int main(){}
  if(blocks_res.is_err()) return blocks_res.unwrap_err();
  auto blocks = blocks_res.unwrap();
  auto cur = blocks;

  while(cur){
    auto res = parse_block();
    if(res.is_err()) return res.unwrap_err();
    cur->next = res.unwrap(); 
    cur = cur->next;
  }

  // Checking the closed right brace
  if(!match(TokenType::punct_rbrace))
    return ParseError("Expected right brace after function body", get_cur_tok_col(), get_cur_tok_line());

  if(!next_is(TokenType::unknown)){
    return ParseError("Only support one main function defination",
      get_cur_tok_col(), get_cur_tok_line());
  }

  return std::make_shared<ast::FunctionDef>(name, name_len, blocks);
}

Expected<Ptr<ast::Block>, ParseError>
Parser::parse_block(){
  // Declaration
  if(next_is(TokenType::kw_int)){
    auto decl = parse_decl();
    if(decl.is_err()) return decl.unwrap_err();
    return std::shared_ptr<ast::Block>(decl.unwrap());
  }
  // Why use std::shared_ptr<ast::Block>() instead of std::make_shared<ast::Block>() ?
  // 1. ast::Block is an abstract class, which could not be constructed.
  // 2. Even if ast::Block is not abtract, in some compiler or context it may not compile
  //    This is because decl.unwrap() is a rvalue and sometimes compiler will not choose to apply implict conversion.

  // Statement
  if(!next_is(TokenType::punct_rbrace)){
    auto stmt = parse_stmt();
    if(stmt.is_err()) return stmt.unwrap_err();
    return std::shared_ptr<ast::Block>(stmt.unwrap());
  }

  // Empty. e.g., int main(){}
  return std::shared_ptr<ast::Block>(0);
}

Expected<Ptr<ast::Decl>, ParseError>
Parser::parse_decl(){
  if(!match(TokenType::kw_int))
    return ParseError("Expected type specifier, for now it is int", get_cur_tok_col(), get_cur_tok_line());
  if(!match(TokenType::ident))
    return ParseError("Expected variable name", get_cur_tok_col(), get_cur_tok_line());
  auto decl = std::make_shared<ast::Decl>(tokens[tok_pos - 1].get_name(), tokens[tok_pos - 1].get_name_len());
  if(match(TokenType::op_assign)){
    auto init = parse_expr();
    if(init.is_err()) return init.unwrap_err();
    decl->init = init.unwrap();
  }
  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicolumm after variable declatation", get_cur_tok_col(), get_cur_tok_line());
  return decl;
}

Expected<Ptr<ast::Stmt>, ParseError>
Parser::parse_stmt(){
  if(next_is(TokenType::kw_ret)){
    auto res = parse_retstmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  if(match(TokenType::punct_semicol))
    return std::shared_ptr<ast::Stmt>(std::make_shared<ast::NullStmt>());
  auto res = parse_exprstmt();
  if(res.is_err()) return res.unwrap_err();
  return std::shared_ptr<ast::Stmt>(res.unwrap());
}

Expected<Ptr<ast::ExprStmt>, ParseError>
Parser::parse_exprstmt(){
  auto expr = parse_expr();
  if(expr.is_err()) return expr.unwrap_err();
  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicolumn", get_cur_tok_col(), get_cur_tok_line());
  return std::make_shared<ast::ExprStmt>(expr.unwrap());
}

// Stmt -> return Expr ;
Expected<Ptr<ast::RetStmt>, ParseError>
Parser::parse_retstmt(){
  if(!match(TokenType::kw_ret))
    return ParseError("Expected keyword return",
      get_cur_tok_col(), get_cur_tok_line());
  
  auto expr = parse_expr();
  if(expr.is_err()) return expr.unwrap_err();

  if(!match(TokenType::punct_semicol))
    return ParseError("Expcted semicolumn",
      get_cur_tok_col(), get_cur_tok_line());

  return std::make_shared<ast::RetStmt>(expr.unwrap());
}

// Expr -> Factor | Expr op Expr
Expected<Ptr<ast::Expr>, ParseError>
Parser::parse_expr(unsigned precedence){
  auto lhs_res = parse_factor();
  if(lhs_res.is_err()) return lhs_res.unwrap_err();

  ast::OpType op;
  Ptr<ast::Expr> lhs = lhs_res.unwrap();

  while(is_next_binary_op() && get_op_precedence(get_cur_tok_type()) >= precedence){
    if(match(TokenType::op_assign)){
      auto rhs = parse_expr(get_op_precedence(ast::OpType::op_assign));
      lhs = std::make_shared<ast::Assign>(lhs, rhs.unwrap());
    }else{
      op = convert_token_to_op(get_cur_tok_type());
      ++tok_pos;
      auto rhs = parse_expr(get_op_precedence(op) + 1);
      if(rhs.is_err()) return rhs.unwrap_err();
      lhs = std::make_shared<ast::Binary>(op, lhs, rhs.unwrap());
    }
  }
  return lhs;
}

// Factor -> int | Unary | (Expr) | Var
Expected<Ptr<ast::Expr>, ParseError>
Parser::parse_factor(){
  if(next_is(TokenType::op_decre))
    return ParseError("We do not support decrement operator yet",
      get_cur_tok_col(), get_cur_tok_line());
  if(next_is(TokenType::op_incre))
    return ParseError("We do not support increment operator yet",
      get_cur_tok_col(), get_cur_tok_line());
  if(is_next_unary_op()){
    auto unary = parse_unary();
    if(unary.is_err()) return unary.unwrap_err();
    return std::shared_ptr<ast::Expr>(unary.unwrap());
  }

  if(match(TokenType::lparen)){
    auto expr = parse_expr();
    if(expr.is_err()) return expr.unwrap_err();
    if(!match(TokenType::rparen))
      return ParseError("Expected )", get_cur_tok_col(), get_cur_tok_line());
    return expr;
  }

  if(match(TokenType::ident))
    return std::shared_ptr<ast::Expr>(
      std::make_shared<ast::Var>(tokens[tok_pos - 1].get_name(), tokens[tok_pos - 1].get_name_len()));

  if(!match(TokenType::li_int))
    return ParseError("Expected expression",
      get_cur_tok_col(), get_cur_tok_line());
  
  char const* val = tokens[tok_pos - 1].get_name();
  unsigned val_len = tokens[tok_pos - 1].get_name_len();

  return 
    std::shared_ptr<ast::Expr>(std::make_shared<ast::Constant>(val, val_len));
}

// Unary -> - | ~ Factor
Expected<Ptr<ast::Unary>, ParseError>
Parser::parse_unary(){
  auto op_type = convert_token_to_op(get_cur_tok_type());
  ++tok_pos; //NOTE
  auto expr = parse_factor();
  if(expr.is_err()) return expr.unwrap_err();
  return std::make_shared<ast::Unary>(op_type, expr.unwrap());
}

}