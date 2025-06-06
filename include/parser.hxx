#pragma once
#include "lexer.hxx"
#include "ast.hxx"
#include "error.hxx"
#include <memory>

namespace niubcc{

class ParseError: Error{
  unsigned col;
  unsigned line;
public:
  ParseError(char const* msg, unsigned col, unsigned line)
  : Error(msg), col(col), line(line){};
  std::string to_string()const override;
};

class Parser{
private:
  std::vector<Token> tokens;
  unsigned tok_pos;

  static void error_handler(ParseError const& err);

  unsigned get_cur_tok_col()const{return tokens[tok_pos].get_col();};
  unsigned get_cur_tok_line()const{return tokens[tok_pos].get_line();};
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

  Expected<Ptr<ast::Program>, ParseError> parse_program();
  Expected<Ptr<ast::FunctionDef>, ParseError> parse_funcdef();
  Expected<Ptr<ast::RetStmt>, ParseError> parse_retstmt();
  Expected<Ptr<ast::Expr>, ParseError> parse_expr();
  Expected<Ptr<ast::Unary>, ParseError> parse_unary();
public:
  Parser(Lexer& lexer);
  Ptr<ast::Program> parse();
};

}