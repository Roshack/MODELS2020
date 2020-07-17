%{
#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <utility>
#include <memory>
#include <vector>
#include <algorithm>
#include "mel_classes.h"
using namespace std;
extern int yylex();
void yyerror(char *s) {
  fprintf(stderr, "%s\n", s);
}

extern std::string result;
extern vector<pair<string, melStmt>> rules;
extern vector<unique_ptr<melStmt>> *stmtsPtr;
vector<string> stmtIDs;

void checkID(string X) {
    bool found = false;
    for (auto &s: stmtIDs) {
        if (s == X) {
            found = true;
            break;
        }
    }
    if (!found) {
        cerr << "\033[31m" << "WARNING:" << "\033[0m" << " rule " << "\033[35m" << X << "\033[0m"  << " referenced before declaration." << endl;
        if (X == "parent") {
            cerr << "\t Did you maybe mean " << "\033[35m" << "mel.parent" << "\033[0m" << "?" << endl;
        } 
    }
}

string cleanEscapes(string &s) {
    stringstream ss;
    bool escaped = false;
    for (auto &c: s) {
        if (c == '\\' && !escaped) {
            escaped = true;
            continue;
        }
        bool foundEscaped =true;
        if (escaped) {
            switch (c) {
                case '\\':
                    ss << "\\";
                    break;
                case 'n':
                    ss << "\n";
                    break;
                case 'r':
                    ss << "\r";
                    break;
                case 't':
                    ss << "\t";
                    break;
                default:
                    foundEscaped = false;
                    break;
            }
        }
        if (!escaped || !foundEscaped) {
            ss << c;
        }
        escaped = false;
    }
    return ss.str();
    
}

%}


%union {
    std::string *string;
    pair<melParam, melParam> *paramPair;
    melRequirement *reqPair;
    vector<pair<melParam, melParam>> *paramPairVec;
    vector<melRequirement> *reqPairVec;
    pair<vector<pair<melParam, melParam>>, vector<melRequirement>> *totalPairVec;
    melCond * cond;
    vector<melCond*> * condVec;
    vector<melParam> *paramVec;
    melStmt * stmt;
    vector<unique_ptr<melStmt>> *stmtVec;
    int token;
};


%token <string> TID TINT TSTRINGLIT
%token <token> TEQUAL PARENTFN TNEQUAL TCONTENTS TNEXISTS TEXISTS TANCEST TOPTIONAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TLBRACK TRBRACK 
%token <token> TSEMICOLON TDOT TCOMMA TIMPLIES TCOLON TNODUP TNOASSOC


%type <string> ident stringlit leftHandBind
%type <string> modifier literal
%type <totalPairVec> params paramList

%type <paramVec> declType
%type <stmt> decl declWithMod
%type <stmtVec> decls

%type <cond> option functionCall fnParam
%type <condVec> options fnParamList fnParams
%type <paramPair> param
%type <reqPair> requirement

%token ID

%%

program : decls {
            stmtsPtr = $1;
            //cerr << " Final Pointer received is:" << stmtsPtr << endl;
        }
        ;

decls  : decls declWithMod {
            $1->emplace_back(unique_ptr<melStmt>{$2});
            //cerr << "Pointer received is: " << $1 << endl;
            $$ = $1;
            
        }
       | declWithMod {
            vector<unique_ptr<melStmt>> *stmts;
            stmts = new vector<unique_ptr<melStmt>>{};
            //cerr << "pointer is now" << stmts << endl;
            stmts->emplace_back(unique_ptr<melStmt>{$1});
            $$ = stmts;
        }
       ;

declWithMod  : TNODUP declWithMod {
                $2->setNoDup(1);
                $$ = $2;
             }
             | TNOASSOC declWithMod {
                $2->setNoAssoc(1);
                $$ = $2;
             }
             | TDOT declWithMod {
                 $2->hideOutput(1);
                 $$ = $2;
             }
             | decl { $$ = $1; }
             ;

decl     : TID TLPAREN declType TRPAREN TIMPLIES options TSEMICOLON
         { 
            melStmt *p = nullptr;
            vector<unique_ptr<melCond>> conds;
            for (auto &p : *$6) {
                conds.emplace_back(unique_ptr<melCond>{p});
            }
            reverse($3->begin(), $3->end()); 
            p = new melStmt{*$1, *$3, move(conds)};
            stmtIDs.emplace_back(*$1);
            delete $1;
            delete $3;
            delete $6;
            $$ = p;
         }
         ;
         
declType : TID { 
            $$ = new vector<melParam>{melParam{*$1, ""}};
            delete $1;
         }
         | TID TCOMMA declType { 
            
            $3->emplace_back(melParam{*$1, ""});
            $$ = $3;
            delete $1;
        }
         ;

options : option {
            vector<melCond*> *p = new vector<melCond*>{};
            p->emplace_back($1);
            $$ = p;
        }
        | options TCOMMA option {
            $1->emplace_back($3);
            $$ = $1;
        }
        | options TCOMMA TOPTIONAL option {
            $4->setOptional(true);
            $1->emplace_back($4);
            $$ = $1;
        }

option  : ident paramList { 
                melCond *p = new melXMLNode{*$1, std::move($2->first), std::move($2->second)};
                delete $2;
                delete $1;
                $$ = p;
        }
        | functionCall {
                $$ = $1;
          }
        ;

        
functionCall : PARENTFN TLPAREN fnParam TCOMMA fnParam TRPAREN {
                vector<unique_ptr<melCond>> conds;
                conds.emplace_back(unique_ptr<melCond>{$3});
                conds.emplace_back(unique_ptr<melCond>{$5});
                vector<pair<melParam,melParam>> params;
                /*for (auto &p : conds) {
                    for (auto &b : p->getParams()){
                        
                    }
                }*/
                melFunction *p = new melParent{move(conds), params};
                $$ = p;
             }
             | TANCEST TLPAREN fnParam TCOMMA fnParam TRPAREN {
                vector<unique_ptr<melCond>> conds;
                conds.emplace_back(unique_ptr<melCond>{$3});
                conds.emplace_back(unique_ptr<melCond>{$5});
                vector<pair<melParam,melParam>> params;
                melFunction *a = new melAncestor{move(conds), params};
                $$ = a;
             }
             | TID fnParamList {
                checkID(*$1);
                vector<unique_ptr<melCond>> conds;
                for (auto &p : *$2) {
                    conds.emplace_back(unique_ptr<melCond>{p->clone()});
                    delete p;
                }
                melFunction *p = new melFunction{*$1, move(conds), vector<pair<melParam,melParam>>{}};
                delete $2;
                delete $1;
                $$ = p;
             }
             | TID fnParamList paramList {
                checkID(*$1);
                vector<unique_ptr<melCond>> conds;
                for (auto &p : *$2) conds.emplace_back(unique_ptr<melCond>{p});
                melFunction *p = new melFunction{*$1, move(conds), $3->first};
                delete $2;
                delete $1;
                delete $3;
                $$ = p;
             }
             ;

             
fnParamList : TLPAREN fnParams TRPAREN { 
               //reverse($2->begin(), $2->end()); 
               $$ = $2; 
            }
         ;
         
fnParams : fnParam {
            vector<melCond*> *c = new vector<melCond*>{$1};
            $$ = c;
         }
         | fnParams TCOMMA fnParam { 
            $1->emplace_back($3);
            $$ = $1;
         }
         ;

fnParam : ident paramList {
            melCond *c  = new melXMLNode{*$1, $2->first, $2->second};
            delete $1;
            delete $2;
            $$ = c;
         }
         | ident {
            melCond *conds = new melIDCond{*$1};
            delete $1;
            $$ = conds;
         }
        ;
paramList : TLBRACE params TRBRACE 
            { 
                //string s;
                //for (auto &p : $2->second) {
                //    s = s + p.first.toString() + ":" + p.second.toString() + ", ";
                //}
                
                $$ = $2;
            }
          | TLBRACE TRBRACE {
                pair<vector<pair<melParam, melParam>>, vector<melRequirement>> *ret = new pair<vector<pair<melParam, melParam>>, vector<melRequirement>>;
                $$ = ret;
          }
          ;
          
params : param { 
            vector<pair<melParam, melParam>> p = vector<pair<melParam, melParam>>{*$1};
            vector<melRequirement> emptyReqs;
            pair<vector<pair<melParam, melParam>>, vector<melRequirement>> *ret = new pair<vector<pair<melParam, melParam>>, vector<melRequirement>>;
            ret->first = p;
            ret->second = emptyReqs;
            delete $1;
            $$ = ret;
        }
       | requirement {
            vector<pair<melParam, melParam>> p;
            vector<melRequirement> reqs{};
            reqs.emplace_back(*$1);
            pair<vector<pair<melParam, melParam>>, vector<melRequirement>> *ret = new pair<vector<pair<melParam, melParam>>, vector<melRequirement>>;
            ret->first = p;
            ret->second = reqs;
            delete $1;
            $$ = ret;
       }
       | params TCOMMA param { 
            $1->first.emplace_back(*$3);
            delete $3;
            $$ = $1;
       }
       | params TCOMMA requirement {
           $1->second.emplace_back(*$3);
           delete $3;
           $$ = $1;
       }
       ;

param : leftHandBind modifier TCOLON ident
        {
            pair<melParam, melParam> *p = new pair<melParam, melParam>{melParam{*$1, *$2}, melParam{*$4, ""}};
            delete $1;
            delete $2;
            delete $4;
            $$ = p;
        }
      ;

requirement : leftHandBind modifier TEQUAL literal
            {
                melRequirement *p = new melRequirement{"=", melParam{*$1, *$2}, *$4};
                delete $1;
                delete $2;
                delete $4;
                $$ = p;
            }
            | leftHandBind modifier TNEQUAL literal
            {
                melRequirement *p = new melRequirement{"!=", melParam{*$1, *$2}, *$4};
                delete $1;
                delete $2;
                delete $4;
                $$ = p;
            }
            | TNEXISTS TLPAREN ident TRPAREN
            {
                melRequirement *p = new melRequirement{"nexists", melParam{*$3,""}, *$3};
                delete $3;
                $$ = p;
            }
            | TEXISTS TLPAREN ident TRPAREN
            {
                melRequirement *p = new melRequirement{"exists", melParam{*$3,""}, *$3};
                delete $3;
                $$ = p;
            }
            ;

leftHandBind : ident { $$ = $1; }
               | TCONTENTS  { $$ = new string{"mel.contents"}; }
      
modifier : TLBRACK stringlit TRBRACK { $$ = $2; }
         | { $$ = new string{""}; }
         ;


literal : stringlit {$$ = $1; }
        | TINT {$$ = $1; }
        ;
         
stringlit : TSTRINGLIT {
            $1->erase($1->begin());
            $1->erase(--$1->end());
            string *ret = new string{cleanEscapes(*$1)};
            delete $1;
            $$ = ret;
          }
      
ident : TID { $$ = $1; }
      | stringlit {
            $$ = $1;
       }
      ;
       


