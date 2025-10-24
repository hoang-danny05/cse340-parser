/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#ifndef __LEXER__H__
#define __LEXER__H__

#include <vector>
#include <string>

#include "inputbuf.h"

// Lexer modified for FIRST & FOLLOW project

typedef enum { END_OF_FILE = 0, ARROW, STAR, HASH, ID, ERROR, OR } TokenType;

class Token {
  public:
    void Print();

    std::string lexeme;
    TokenType token_type;
    int line_no;

  bool operator==(const Token& other) const{
    return other.lexeme == this->lexeme;
  }
  int compare(const Token& other) {
    return this->lexeme.compare(other.lexeme);
  } 
};

class LexicalAnalyzer {
  public:
    Token GetToken();
    Token peek(int);
    LexicalAnalyzer();


  private:
    std::vector<Token> tokenList;
    Token GetTokenMain();
    int line_no;
    int index;
    Token tmp;
    InputBuffer input;

    bool SkipSpace();
    Token ScanId();
};

#endif  //__LEXER__H__
