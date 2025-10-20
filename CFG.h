
#ifndef __CFG__H_
#define __CFG__H_

#include "lexer.h"
#include <vector>
#include <string>
#include <map>


static const bool DEBUGGING = true;

class Rule {
public: 
  Rule(Token, std::vector<Token>);
  void Print();
  bool isNull();

  Token LHS; 
  std::vector<Token> RHS;
private:
  Token tmp;
};

// will have its own lexical analyzer
class ContextFreeGrammar {
public:
  ContextFreeGrammar();
  void Print();
  void PrintTokens();
  void PrintNullable();
  void PrintFirst();
  void PrintFollow();

  std::vector<Rule> rules;
  std::vector<Token> nonterminals;
  std::vector<Token> terminals;

  std::vector<Token> nullable;

  std::map<std::string, std::vector<Token>> first;
  std::map<std::string, std::vector<Token>> follow;
private:
  void initTokens();
  void initNullable();
  void initFirst();
  void initFollow();
  bool vecContains(std::vector<Token>, Token);
};

#endif
