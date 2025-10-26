#ifndef __CFG__H__
#define __CFG__H__

using namespace std;
static const bool DEBUGGING = false;

class Rule {
public: 
  Rule(string, vector<string>);
  Rule();
  void Print();
  bool isNull();
  bool hasPrefix(vector<string>);

  string LHS; 
  vector<string> RHS;

  int compare(const Rule *other) {

    if ((other->LHS .compare(this->LHS ) < 0)) {
      return -1;
    }
    else if ((other->LHS .compare(this->LHS ) > 0)) {
      return 1;
    }

    for (int i = 0; i < other->RHS.size() && i < this->RHS.size(); i++) {
      if ((other->RHS.at(i) .compare(this->RHS.at(i) ) < 0)) {
        return -1;
      }
      else if ((other->RHS.at(i) .compare(this->RHS.at(i) ) > 0)) {
        return 1;
      }
    }

    if (this->RHS.size() == other->RHS.size()) {
      return 0;
    }
    return (other->RHS.size() > this->RHS.size());
  }

  void cutBeginning(int cutLength) {
    //std::cout << "len: " << cutLength << std::endl;
    std::vector<string> newRHS;
    for (int i = cutLength; i < RHS.size(); i++) {
      newRHS.push_back(RHS[i]);
    }
    RHS = newRHS;
  }

  bool operator ==(const Rule& other) {
    return (this->compare(&other)) == 0;
  }

  std::vector<string> afterPrefix(int prefixLen) {
    std::vector<string> ret;
    for (int i = prefixLen; i < RHS.size(); i++) {
      ret.push_back(RHS[i]);
    }
    return ret;
  }

private:
  string tmp;
};

// will have its own lexical analyzer
class ContextFreeGrammar {
public:
  ContextFreeGrammar();
  void readGrammar();
  void init();

  void Print();
  void Printstrings();
  void PrintNullable();
  void PrintFirst();
  void PrintFollow();

  std::vector<Rule> rules;
  std::vector<string> nonterminals;
  std::vector<string> terminals;

  std::vector<string> nullable;

  std::map<std::string, std::vector<string>> first;
  std::map<std::string, std::vector<string>> follow;

  // helpers 

  std::vector<Rule> getRulesWith(string);
  std::vector<Rule> popRulesWith(string);
  std::vector<Rule> popRulesWithPrefix(string, std::vector<string>);

  static bool vecContains(std::vector<string> vec, string item) {
    for(int i = 0; i < vec.size(); i++) {
      if (vec.at(i)  == item )
        return true;
    }
    return false;
  }

//NOTE: USE THIS AS SET ADDITION, NOTHING ELSE
// returns changesMade
static int vecAddTo(std::vector<string>* from, std::vector<string>* to) {
  int changesMade = 0;

  // for term in termsToAdd
  for (string term : *from) {
    if (!vecContains(*to, term)) 
    {
      to->push_back(term);
      //first.at(rule.LHS ).push_back(term);
      changesMade++;
    }
  }

  return changesMade;
}

private:
  void initTokens();
  void initNullable();
  void initFirst();
  void initFollow();
};


#endif
