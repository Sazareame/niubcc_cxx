#include "parser.h"
#include "utils.h"
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
  return utils::fmt("Parse Error at line %u, col %u: %s", pos.line, pos.col, msg);
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
    return ParseError("Expected keyword int", get_cur_tok_pos());

  if(!match(TokenType::ident))
    return ParseError("Expected function name", get_cur_tok_pos());
  char const* name = tokens[tok_pos - 1].get_name();
  unsigned name_len = tokens[tok_pos - 1].get_name_len();

  if(!match(TokenType::lparen, TokenType::rparen, TokenType::punct_lbrace))
    return ParseError("Syntax error", get_cur_tok_pos());

  symbol_table.call_func();

  auto body = parse_compoundstmt();
  if(body.is_err()) return body.unwrap_err();

  if(!next_is(TokenType::unknown)){
    return ParseError("Only support one main function defination",
      get_cur_tok_pos());
  }

  auto check_label = symbol_table.resolve_all_labels();
  if(check_label.has_value()){
    return ParseError("Use of undefined label", check_label.value());
  }

  symbol_table.ret_func();

  return std::make_shared<ast::FunctionDef>(name, name_len, body.unwrap());
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
    return ParseError("Expected type specifier, for now it is int", get_cur_tok_pos());

  auto res = parse_decl_init_list();
  if(res.is_err()) return res.unwrap_err();
  auto decl = res.unwrap();
  Ptr<ast::Block> cur = decl;

  while(match(TokenType::punct_comma)){
    auto res = parse_decl_init_list();
    if(res.is_err()) return res.unwrap_err();
    cur->next = res.unwrap();
    cur = cur->next;
  }

  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicolumm after variable declatation", get_cur_tok_pos());

  return decl;
}

Expected<Ptr<ast::Decl>, ParseError>
Parser::parse_decl_init_list(){
  if(!match(TokenType::ident))
    return ParseError("Expected variable name", get_cur_tok_pos());

  char const* name = tokens[tok_pos - 1].get_name();
  unsigned len = tokens[tok_pos - 1].get_name_len();
  auto uniq_name = symbol_table.lookup_and_add(name, len);
  if(!uniq_name) return ParseError("Duplicate declaration", get_cur_tok_pos());

  auto decl = std::make_shared<ast::Decl>(uniq_name);
  if(match(TokenType::op_assign)){
    auto init = parse_expr();
    if(init.is_err()) return init.unwrap_err();
    decl->init = init.unwrap();
  }

  return decl;
}

Expected<Ptr<ast::Stmt>, ParseError>
Parser::parse_stmt(){
  if(next_is(TokenType::ident) && tokens[tok_pos + 1].get_type() == TokenType::punct_colon){
    if(!symbol_table.is_in_func())
      return ParseError("Can only define lable in functions", get_cur_tok_pos());
    char const* name = tokens[tok_pos].get_name();
    unsigned len = tokens[tok_pos].get_name_len();
    auto label_name = symbol_table.define_label(name, len, get_cur_tok_pos());
    if(!label_name) return ParseError("Redifine label", get_cur_tok_pos());
    tok_pos += 2;
    auto res = parse_stmt();
    if(res.is_err()) return res.unwrap_err();
    auto stmt = res.unwrap();
    stmt->label = label_name;
    return stmt;
  }
  if(match(TokenType::kw_ret)){
    auto res = parse_retstmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  if(match(TokenType::kw_if)){
    auto res = parse_ifstmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  if(match(TokenType::kw_do)){
    auto res = parse_dostmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  if(match(TokenType::kw_while)){
    auto res = parse_whilestmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  if(match(TokenType::kw_for)){
    auto res = parse_forstmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  if(match(TokenType::kw_goto)){
    auto res = parse_gotostmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  if(match(TokenType::kw_break)){
    if(!loop_depth) return ParseError("Break statement outside loop", get_cur_tok_pos());
    if(!match(TokenType::punct_semicol)) return ParseError("Expected semicolumn", get_cur_tok_pos());
    return std::shared_ptr<ast::Stmt>(std::make_shared<ast::Break>());
  }
  if(match(TokenType::kw_continue)){
    if(!loop_depth) return ParseError("Contiue statement outside loop", get_cur_tok_pos());
    if(!match(TokenType::punct_semicol)) return ParseError("Expected semicolumn", get_cur_tok_pos());
    return std::shared_ptr<ast::Stmt>(std::make_shared<ast::Continue>());
  }
  if(match(TokenType::punct_semicol))
    return std::shared_ptr<ast::Stmt>(std::make_shared<ast::NullStmt>());
  if(match(TokenType::punct_lbrace)){
    auto res = parse_compoundstmt();
    if(res.is_err()) return res.unwrap_err();
    return std::shared_ptr<ast::Stmt>(res.unwrap());
  }
  auto res = parse_exprstmt();
  if(res.is_err()) return res.unwrap_err();
  return std::shared_ptr<ast::Stmt>(res.unwrap());
}

Expected<Ptr<ast::DoStmt>, ParseError>
Parser::parse_dostmt(){
  ++loop_depth;
  auto stmt = parse_stmt();
  if(stmt.is_err()) return stmt.unwrap_err();

  if(!match(TokenType::kw_while))
    return ParseError("Expected keyword while after do-statement body", get_cur_tok_pos());

  if(!match(TokenType::lparen))
    return ParseError("Expected left paranthesis", get_cur_tok_pos());

  auto condition = parse_expr();
  if(condition.is_err()) return condition.unwrap_err();

  if(!match(TokenType::rparen))
    return ParseError("Expected right paranthesis", get_cur_tok_pos());

  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicoloum", get_cur_tok_pos());
  
  --loop_depth;
  return std::make_shared<ast::DoStmt>(stmt.unwrap(), condition.unwrap());
};

Expected<Ptr<ast::WhileStmt>, ParseError>
Parser::parse_whilestmt(){
  if(!match(TokenType::lparen))
    return ParseError("Expected left paranthesis", get_cur_tok_pos());

  auto condition = parse_expr();
  if(condition.is_err()) return condition.unwrap_err();

  if(!match(TokenType::rparen))
    return ParseError("Expected right paranthesis", get_cur_tok_pos());

  ++loop_depth;
  auto stmt = parse_stmt();
  if(stmt.is_err()) return stmt.unwrap_err();
  --loop_depth;
  return std::make_shared<ast::WhileStmt>(stmt.unwrap(), condition.unwrap());
};

Expected<Ptr<ast::ForStmt>, ParseError>
Parser::parse_forstmt(){
  if(!match(TokenType::lparen))
    return ParseError("Expected left paranthesis", get_cur_tok_pos());

  symbol_table.enter_scope();

  auto init = parse_forinit();
  if(init.is_err()) return init.unwrap_err();

  Ptr<ast::Expr> condition = 0;
  Ptr<ast::Expr> post = 0;

  if(!match(TokenType::punct_semicol)){
    auto res = parse_expr();
    if(res.is_err()) return res.unwrap_err();
    condition = res.unwrap();
    if(!match(TokenType::punct_semicol))
      return ParseError("Expected semicolumn", get_cur_tok_pos());
  }

  if(!match(TokenType::punct_semicol)){
    auto res = parse_expr();
    if(res.is_err()) return res.unwrap_err();
    post = res.unwrap();
  }

  if(!match(TokenType::rparen))
    return ParseError("Expected right paranthesis", get_cur_tok_pos());

  ++loop_depth;
  auto stmt = parse_stmt();
  if(stmt.is_err()) return stmt.unwrap_err();
  --loop_depth;

  symbol_table.leave_scope();
  return std::make_shared<ast::ForStmt>(init.unwrap(), condition, post, stmt.unwrap());
};

Expected<Ptr<ast::ForStmtInit>, ParseError>
Parser::parse_forinit(){
  if(next_is(TokenType::kw_int)){
    auto res = parse_decl();
    if(res.is_err()) return res.unwrap_err();
    return std::make_shared<ast::ForStmtInit>(res.unwrap());
  }
  if(next_is(TokenType::punct_semicol))
    return std::shared_ptr<ast::ForStmtInit>(0);
  auto res = parse_expr();
  if(res.is_err()) return res.unwrap_err();
  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicolumn", get_cur_tok_pos());
  return std::make_shared<ast::ForStmtInit>(res.unwrap());
}

Expected<Ptr<ast::GotoStmt>, ParseError>
Parser::parse_gotostmt(){
  if(!match(TokenType::ident))
    return ParseError("Expected label name", get_cur_tok_pos());
  char const* name = tokens[tok_pos - 1].get_name();
  unsigned len = tokens[tok_pos - 1].get_name_len();
  auto label_name = symbol_table.add_label(name, len, get_cur_tok_pos());
  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicolumn", get_cur_tok_pos());
  return std::make_shared<ast::GotoStmt>(label_name);
}

Expected<Ptr<ast::CompoundStmt>, ParseError>
Parser::parse_compoundstmt(){
  symbol_table.enter_scope();

  auto blocks_res = parse_block();
  // parse_block() return null ptr if the body is empty, e.g., int main(){}
  if(blocks_res.is_err()) return blocks_res.unwrap_err();
  auto blocks = blocks_res.unwrap();
  auto cur = blocks;
  while(cur->next) cur = cur->next;

  while(1){
    auto res = parse_block();
    if(res.is_err()) return res.unwrap_err();
    cur->next = res.unwrap(); 
    if(!cur->next) break;
    while(cur->next) cur = cur->next;
  }

  // Checking the closed right brace
  if(!match(TokenType::punct_rbrace))
    return ParseError("Expected right brace after function body", get_cur_tok_pos());

  symbol_table.leave_scope();
  return std::make_shared<ast::CompoundStmt>(blocks);
}

Expected<Ptr<ast::IfStmt>, ParseError>
Parser::parse_ifstmt(){
  if(!match(TokenType::lparen))
    return ParseError("Expected left paranthesis", get_cur_tok_pos());
  auto condition = parse_expr();

  if(condition.is_err()) return condition.unwrap_err();

  if(!match(TokenType::rparen))
    return ParseError("Expected right paranthesis", get_cur_tok_pos());

  auto then_stmt = parse_stmt();
  if(then_stmt.is_err()) return then_stmt.unwrap_err();
  Ptr<ast::Stmt> else_stmt = 0;
  if(match(TokenType::kw_else)){
    auto res = parse_stmt();
    if(res.is_err()) return res.unwrap_err();
    else_stmt = res.unwrap();
  }
  return std::make_shared<ast::IfStmt>(condition.unwrap(), then_stmt.unwrap(), else_stmt);
}

Expected<Ptr<ast::ExprStmt>, ParseError>
Parser::parse_exprstmt(){
  auto expr = parse_expr();
  if(expr.is_err()) return expr.unwrap_err();
  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicolumn", get_cur_tok_pos());
  return std::make_shared<ast::ExprStmt>(expr.unwrap());
}

// Stmt -> return Expr ;
Expected<Ptr<ast::RetStmt>, ParseError>
Parser::parse_retstmt(){
  auto expr = parse_expr();
  if(expr.is_err()) return expr.unwrap_err();

  if(!match(TokenType::punct_semicol))
    return ParseError("Expected semicolumn",
      get_cur_tok_pos());

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
      if(!std::dynamic_pointer_cast<ast::Var>(lhs))
        return ParseError("Cannot assign to a rvalue", get_cur_tok_pos());
      auto rhs = parse_expr(get_op_precedence(ast::OpType::op_assign));
      lhs = std::make_shared<ast::Assign>(rhs.unwrap(), lhs);
    }else if(match(TokenType::op_que)){
      auto mid = parse_condition();
      if(mid.is_err()) return mid.unwrap_err();
      auto rhs = parse_expr(get_op_precedence(ast::OpType::op_que));
      if(rhs.is_err()) return rhs.unwrap_err();
      lhs = std::make_shared<ast::Condition>(lhs, mid.unwrap(), rhs.unwrap());
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

Expected<Ptr<ast::Expr>, ParseError>
Parser::parse_condition(unsigned precedence){
  auto res = parse_expr();
  if(res.is_err()) return res.unwrap_err();
  if(!match(TokenType::punct_colon))
    return ParseError("Expected colon in condition expression", get_cur_tok_pos());
  return res.unwrap();
}

// Factor -> int | Unary | (Expr) | Var
Expected<Ptr<ast::Expr>, ParseError>
Parser::parse_factor(){
  if(next_is(TokenType::op_decre))
    return ParseError("We do not support decrement operator yet",
      get_cur_tok_pos());
  if(next_is(TokenType::op_incre))
    return ParseError("We do not support increment operator yet",
      get_cur_tok_pos());
  if(is_next_unary_op()){
    auto unary = parse_unary();
    if(unary.is_err()) return unary.unwrap_err();
    return std::shared_ptr<ast::Expr>(unary.unwrap());
  }

  if(match(TokenType::lparen)){
    auto expr = parse_expr();
    if(expr.is_err()) return expr.unwrap_err();
    if(!match(TokenType::rparen))
      return ParseError("Expected )", get_cur_tok_pos());
    return expr;
  }

  if(match(TokenType::ident)){
    auto uniq_name = symbol_table.lookup_and_get(
      tokens[tok_pos - 1].get_name(), tokens[tok_pos - 1].get_name_len());
    if(!uniq_name)
      return ParseError("Undefined Variable", get_cur_tok_pos());
    return std::shared_ptr<ast::Expr>(std::make_shared<ast::Var>(uniq_name));
  }

  if(!match(TokenType::li_int))
    return ParseError("Expected expression", get_cur_tok_pos());
  
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