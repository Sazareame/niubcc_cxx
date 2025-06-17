#pragma once
#include "lexer.h"
#include "ast.h"
#include "error.h"
#include "symbol_table.h"
#include <memory>

namespace niubcc{

class ParseError: Error{
  utils::Pos pos;
public:
  ParseError(char const* msg, utils::Pos pos)
  : Error(msg), pos(pos){};
  std::string to_string()const override;
};

class Parser{
private:
  SymbolTable symbol_table{};
  std::vector<Token> tokens;
  unsigned tok_pos;

  static void error_handler(ParseError const& err);

  static unsigned op_precedence[];

  ast::OpType convert_token_to_op(TokenType tokentype)const;

  utils::Pos get_cur_tok_pos()const{return tokens[tok_pos].get_pos();};
  TokenType get_cur_tok_type()const{return tokens[tok_pos].get_type();};

  bool match(TokenType type);

  template<class... Args>
  bool match(TokenType type, Args... types){
    return match(type) && match(types...);
  }

  bool next_is(TokenType type){return tokens[tok_pos].is(type);}

  template<class... Args>
  bool next_is(TokenType type, Args... types){
    return next_is(type) || next_is(types...);
  }

  unsigned get_op_precedence(TokenType tokentype)const{
    return op_precedence[static_cast<unsigned>(convert_token_to_op(tokentype))];
  }

  unsigned get_op_precedence(ast::OpType optype)const{
    return op_precedence[static_cast<unsigned>(optype)];
  }

  bool is_next_binary_op()const;
  bool is_next_unary_op()const;

  // Represent current loop depth, used for detecting bad break and continue.
  unsigned loop_depth{0};

  Expected<Ptr<ast::Program>, ParseError> parse_program();
  Expected<Ptr<ast::FunctionDef>, ParseError> parse_funcdef();
  Expected<Ptr<ast::Block>, ParseError> parse_block();
  Expected<Ptr<ast::Decl>, ParseError> parse_decl();
  Expected<Ptr<ast::Decl>, ParseError> parse_decl_init_list();
  Expected<Ptr<ast::Stmt>, ParseError> parse_stmt();
  Expected<Ptr<ast::CompoundStmt>, ParseError> parse_compoundstmt();
  Expected<Ptr<ast::ExprStmt>, ParseError> parse_exprstmt();
  Expected<Ptr<ast::RetStmt>, ParseError> parse_retstmt();
  Expected<Ptr<ast::IfStmt>, ParseError> parse_ifstmt();
  Expected<Ptr<ast::DoStmt>, ParseError> parse_dostmt();
  Expected<Ptr<ast::WhileStmt>, ParseError> parse_whilestmt();
  Expected<Ptr<ast::ForStmt>, ParseError> parse_forstmt();
  Expected<Ptr<ast::ForStmtInit>, ParseError> parse_forinit();
  Expected<Ptr<ast::GotoStmt>, ParseError> parse_gotostmt();
  Expected<Ptr<ast::Expr>, ParseError> parse_expr(unsigned precedence=0);
  Expected<Ptr<ast::Expr>, ParseError> parse_condition(unsigned precedence=0);
  Expected<Ptr<ast::Expr>, ParseError> parse_factor();
  Expected<Ptr<ast::Unary>, ParseError> parse_unary();
public:
  Parser(Lexer& lexer);
  Ptr<ast::Program> parse();
};

}