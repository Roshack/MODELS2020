#ifndef RULES_CLASSES_H
#define RULES_CLASSES_H
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <sstream>
#include <list>
#include <utility>
#include "../lib/rapidxml/rapidxml.hpp"

#define CSVOUT 0

class Rule {
  protected:
    std::string relType;
    std::string lhs, rhs;
    std::string cond;
  public:
    Rule(std::string lhs, std::string rhs, std::string rel, std::string cond = "") : relType{rel}, lhs{lhs}, rhs{rhs}, cond{cond} {}
    virtual void printOut(std::ostream &out) const = 0;
    friend std::ostream& operator<<(std::ostream &, const Rule &);
    std::string toString() {
      std::string ret{lhs + " " + relType + " " + rhs};
      if (cond != "") {
        ret += " when " + cond;
      }
      return ret;
    }
};

class CSVRule : public Rule {
  public:
    CSVRule(std::string lhs, std::string rhs, std::string rel, std::string cond = "") : Rule{lhs,rhs,rel,cond} {}
    virtual void printOut(std::ostream &out) const override {
        out << lhs << "\t" << rhs << "\t" << relType;
        if (cond != "") {
          out << "\t" << cond;
        } else {
          out << "\t" << "true";
        }
    }
};

/*
">"                     { yylval.fn = 1; return CMP; }
"=>"                    { yylval.fn = 2; return CMP; }
">="                    { yylval.fn = 2; return CMP; }
"<"                     { yylval.fn = 3; return CMP; }
"<="                    { yylval.fn = 4; return CMP; }
"=<"                    { yylval.fn = 4; return CMP; }
"=="                    { yylval.fn = 5; return CMP; }
"!="                    { yylval.fn = 6; return CMP; }
*/

std::string CMPTokenToString(int cmp);

#endif

