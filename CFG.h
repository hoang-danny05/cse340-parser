#ifndef __CFG__H__
#define __CFG__H__

#include <cctype>
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

  static int compareStrings(string before, string after) {
    for (int i = 0; i < before.size() && i < after.size(); i++) {
      if (tolower(before[i]) < tolower(after[i])) {
        return -1;
      }
      if (tolower(before[i]) > tolower(after[i])) {
        return 1;
      }
    }

    if (before.size() < after.size()) {
      return -1;
    }
    if (before.size() > after.size()) {
      return 1;
    }
    return 0;
  }

  int compare(const Rule *other) {
    if (compareStrings(other->LHS, this->LHS) < 0) {
      return -1;
    }
    if (compareStrings(other->LHS, this->LHS) > 0) {
      return 1;
    }

    for (int i = 0; i < other->RHS.size() && i < this->RHS.size(); i++) {
      if (compareStrings(other->RHS[i], this->RHS[i]) < 0) {
        return -1;
      }
      if (compareStrings(other->RHS[i], this->RHS[i]) > 0) {
        return 1;
      }
    }

    if (this->RHS.size() == other->RHS.size()) {
      return 0;
    }
    return (other->RHS.size() > this->RHS.size());
  }

  int compare2(const Rule *other) {
    if (other->LHS.compare(this->LHS) < 0) {
      return -1;
    }
    if (other->LHS.compare(this->LHS) > 0) {
      return 1;
    }

    for (int i = 0; i < other->RHS.size() && i < this->RHS.size(); i++) {
      if (other->RHS[i].compare(this->RHS[i]) < 0) {
        return -1;
      }
      if (other->RHS[i].compare(this->RHS[i]) > 0) {
        return 1;
      }
    }

    if (this->RHS.size() == other->RHS.size()) {
      return 0;
    }
    return (other->RHS.size() > this->RHS.size());
  }
  void cutBeginning(int cutLength) {
    //cout << "len: " << cutLength << endl;
    vector<string> newRHS;
    for (int i = cutLength; i < RHS.size(); i++) {
      newRHS.push_back(RHS[i]);
    }
    RHS = newRHS;
  }

  Rule substitutePrefix(int, vector<string>);

  bool operator ==(const Rule& other) {
    return (this->compare(&other)) == 0;
  }

  vector<string> afterPrefix(int prefixLen) {
    vector<string> ret;
    for (int i = prefixLen; i < RHS.size(); i++) {
      ret.push_back(RHS[i]);
    }
    return ret;
  }

  int lengthCommonPrefixWith(const Rule& other) {
    for (int i = 0; i < this->RHS.size() && i < other.RHS.size(); i++) {
      if (this->RHS[i] == other.RHS[i]) {
        continue;
      } else {
        return i;
      }
    }
    return 0;
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
  void PrintTokens();
  void PrintNullable();
  void PrintFirst();
  void PrintFollow();

  vector<Rule> rules;
  vector<string> nonterminals;
  vector<string> terminals;

  vector<string> nullable;

  map<string, vector<string>> first;
  map<string, vector<string>> follow;

  // helpers 

  vector<Rule> getRulesWith(string);
  vector<Rule> popRulesWith(string);
  vector<Rule> popRulesWithPrefix(string, vector<string>);

  static bool vecContains(vector<string> vec, string item) {
    for(int i = 0; i < vec.size(); i++) {
      if (vec.at(i)  == item )
        return true;
    }
    return false;
  }

//NOTE: USE THIS AS SET ADDITION, NOTHING ELSE
// returns changesMade
static int vecAddTo(vector<string>* from, vector<string>* to) {
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
