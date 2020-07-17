%{
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "rules_classes.h"
using namespace std;
extern int yylex();
void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

extern std::string result;
extern std::unordered_map<string,string> decls;
extern std::unordered_set<string> entities;
extern std::vector<std::unique_ptr<Rule>> rules;
%}


%union {
     int fn;
     int token;
     std::string *string;
};


%token <string> TID TINT TSTRINGLIT
%token <token> CMP
%token <token> TINFLUENCE TACTIVEIF TNOT TIS TNL
%token <token> TSEMICOLON TWHEN TLPAREN TRPAREN 


%type <string> ident id
%type <string> program decls decl rules rule influenceRule
%type <string> activeRule isRule cond 


%token ID

%%

program : decls rules 
        | rules
        ;

decls : decls decl 
      | decl
      ;

decl : ident TLPAREN id TRPAREN TNL {
          decls[*$3] = *$1;
          entities.insert(*$1);
          delete $1;
          delete $3;
      }
     ;


rules : rules rule
      | rule
      ;

rule : influenceRule TNL
     | activeRule TNL
     | isRule TNL
     | TNL { $$ = NULL; }
     ;

influenceRule : ident TINFLUENCE ident { 
                    std::string lhs = decls.find(*$1) != decls.end() ? decls[*$1] : *$1;
                    std::string rhs = decls.find(*$3) != decls.end() ? decls[*$3] : *$3;
                    rules.emplace_back(make_unique<CSVRule>(lhs, rhs, "influences"));
                    entities.insert(lhs);
                    entities.insert(rhs);
                    delete $1; delete $3;
               }
              | ident TINFLUENCE ident TWHEN ident {
                    std::string lhs = decls.find(*$1) != decls.end() ? decls[*$1] : *$1;
                    std::string rhs = decls.find(*$3) != decls.end() ? decls[*$3] : *$3;
                    std::string cond = decls.find(*$5) != decls.end() ? decls[*$5] : *$5;
                    rules.emplace_back(make_unique<CSVRule>(lhs, rhs, "influences", cond));
                    entities.insert(lhs);
                    entities.insert(rhs);
                    entities.insert(cond);
                    delete $1; delete $3; delete $5;
               }
              | ident TINFLUENCE ident TWHEN cond {
                    std::string lhs = decls.find(*$1) != decls.end() ? decls[*$1] : *$1;
                    std::string rhs = decls.find(*$3) != decls.end() ? decls[*$3] : *$3;
                    rules.emplace_back(make_unique<CSVRule>(lhs, rhs, "influences", *$5));
                    entities.insert(*$1);
                    entities.insert(*$3);
                    delete $1; delete $3; delete $5;
              }
              ;

activeRule : ident TACTIVEIF cond  {
                    std::string lhs = decls.find(*$1) != decls.end() ? decls[*$1] : *$1;
                    std::string rhs = decls.find(*$3) != decls.end() ? decls[*$3] : *$3;
                    rules.emplace_back(make_unique<CSVRule>(lhs, lhs, "active_if", rhs));
                    entities.insert(lhs);
                    delete $1; delete $3;
              }
           | ident TNOT TACTIVEIF cond {
                    std::string lhs = decls.find(*$1) != decls.end() ? decls[*$1] : *$1;
                    std::string rhs = decls.find(*$4) != decls.end() ? decls[*$4] : *$4;
                    rules.emplace_back(make_unique<CSVRule>(lhs, lhs, "not_active_if", rhs));
                    entities.insert(lhs);
                    delete $1; delete $4;
          }
          | ident TACTIVEIF ident {
               std::string lhs = decls.find(*$1) != decls.end() ? decls[*$1] : *$1;
               std::string rhs = decls.find(*$3) != decls.end() ? decls[*$3] : *$3;
               rules.emplace_back(make_unique<CSVRule>(lhs, lhs, "active_if", rhs));
               entities.insert(lhs);
               entities.insert(rhs);
               delete $1; delete $3;
          }
           ;

isRule : cond 
       | cond TWHEN cond 
       ;

cond : ident CMP ident {
          std::stringstream ss;
          ss << *$1 << CMPTokenToString($2) << *$3;
          $$ = new std::string{ss.str()};
          delete $1; delete $3;
     }
     | ident CMP TINT {
          std::stringstream ss;
          ss << *$1 << CMPTokenToString($2) << *$3;
          $$ = new std::string{ss.str()};
          delete $1; delete $3;
     }
     | TINT CMP ident {
          std::stringstream ss;
          ss << *$1 << CMPTokenToString($2) << *$3;
          $$ = new std::string{ss.str()};
          delete $3; delete $1;
     }
     ;



ident : ident id {
        $$ = new std::string{*$1 + " " + *$2};
        delete $1;
        delete $2;
    }
      | id { $$ = $1; }
      ;

id : TID { $$ = $1; }
   ;