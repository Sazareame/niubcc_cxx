#pragma once
#include <string>
#include <vector>
#include "error.hxx"

namespace niubcc{

class LexerError: Error{
private:
  unsigned col;
  unsigned line;
public:
  LexerError(char const* msg, unsigned col, unsigned line):
  Error(msg), col(col), line(line){};
  std::string to_string() const override;
};

enum class TokenType: unsigned short{
  unknown,
  ident,
  kw_int,
  kw_void,
  kw_ret,
  lparen,
  rparen,
  punct_lbrace,
  punct_rbrace,
  punct_semicol,
  li_int,
};

class Lexer;

class Token{
  friend class Lexer;
private:
  char const* p_text;
  TokenType type;
  unsigned len;

  union{
    char const* raw_literal;
    char const* raw_indent;
  };

  unsigned addtional_len;

  void init(); 

public:
  Token() = default;
  bool is(TokenType _type) const{return type == _type;}

};

class Lexer{
private:
  unsigned col{1};
  unsigned line{0};
  unsigned tok_pos{0};
  unsigned tok_max_len;
  char const* text_ptr;
  char const* cur_ptr;
  std::vector<Token> tokens;

  Expected<bool, LexerError> lex_one_token();
  bool skip_whitespace();
  bool lex_number(Token& token);
  bool lex_ident_or_kw(Token& token);

  static void err_handler(LexerError const& err);

public:
  Lexer(char const* _ptr, unsigned _tok_max_len):
  text_ptr(_ptr), tok_max_len(_tok_max_len), tokens(_tok_max_len){};
  ~Lexer() = default;
  
  void tokenize();
};

}