#pragma once
#include <string>
#include <vector>
#include <optional>
#include "error.h"
#include "utils.h"

namespace niubcc{

class LexerError: Error{
private:
  utils::Pos pos;
public:
  LexerError(char const* msg, utils::Pos pos):
  Error(msg), pos(pos){};
  std::string to_string() const override;
};

#define TOK(X, S) X,
#define OP(X, S, P, B, U) X,
enum class TokenType: unsigned short{
#include "token.def"
};
#undef TOK
#undef OP

class Lexer;

class Token{
  friend class Lexer;
private:
  char const* p_text;
  TokenType type;
  unsigned len;
  utils::Pos pos;
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

  utils::Pos get_pos()const{return pos;}
  char const* get_name()const{
    if(type == TokenType::ident)
      return raw_indent;
    if(is_literal())
      return raw_literal;
    return 0;
  }
  unsigned get_name_len()const{
    if(is_ident() || is_literal())
      return addtional_len;
    return 0;
  }
  TokenType get_type()const{
    return type;
  } 
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
  std::vector<Token>& get_tokens_out();
  void display_all_tokens()const;

  unsigned get_token_vec_len()const{return tok_pos;}
  void reset_token_vec_len(){tok_pos = 0;}
};

}