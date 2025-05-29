#pragma once
#include <string>
#include <vector>
#include <optional>
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

#define TOK(X, S) X,
enum class TokenType: unsigned short{
#include "token.def"
};
#undef TOK

class Lexer;

class Token{
  friend class Lexer;
private:
  char const* p_text;
  TokenType type;
  unsigned len;
  unsigned col;
  unsigned line;

  union{
    char const* raw_literal;
    char const* raw_indent;
  };

  unsigned addtional_len;

  static char const* token_name_map[];

  void init(); 

  std::optional<std::string> fmt()const;

public:
  Token() = default;
  bool is(TokenType _type) const{return type == _type;}
  bool is_ident()const{return type == TokenType::ident;};
  bool is_keyword()const;
  bool is_literal()const;

};

class Lexer{
private:
  unsigned col{1};
  unsigned line{1};
  unsigned tok_pos{0};
  unsigned tok_max_len;
  char const* text_ptr;
  char const* cur_ptr;
  std::vector<Token> tokens;

  Expected<bool, LexerError> lex_one_token();
  bool skip_whitespace();
  bool lex_number(Token& token);
  bool lex_ident_or_kw(Token& token);
  bool is_valid_ident_char(char c);

  static void err_handler(LexerError const& err);

public:
  Lexer(char const* _ptr, unsigned _tok_max_len):
  text_ptr(_ptr), tok_max_len(_tok_max_len), tokens(_tok_max_len){};
  ~Lexer() = default;
  
  void tokenize();

  std::vector<Token> const& get_tokens()const;
  void display_all_tokens()const;

  unsigned get_token_vec_len()const{return tok_pos;}
  void reset_token_vec_len(){tok_pos = 0;}
};

}