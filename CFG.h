
#ifndef __CFG__H_
#define __CFG__H_

#include "lexer.h"
#include <vector>
#include <string>


class Rule {
public: 
  Rule(LexicalAnalyzer*);
  void Print();

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

    std::vector<Rule> rules;
};

#endif
