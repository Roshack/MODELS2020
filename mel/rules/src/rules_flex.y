%{
#include <string>
#include <iostream>
#include <vector>
#include <utility>
using namespace std;
#include "rules_classes.h"
#include "../obj/rules_bison.hpp"

#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { }

//https://stackoverflow.com/questions/9628099/c-istream-with-lex
extern std::istream *lexer_ins_;

#define YY_INPUT(buf, result, max_size) \
    result = 0; \
    while (1) { \
        int c = lexer_ins_->get(); \
        if (lexer_ins_->eof()) { \
            break; \
        } \
        buf[result++] = c; \
        if (result == max_size) break; \
    }
%}

%%

[ \t\r]                 ;
"\n"                    return TOKEN(TNL);
"influences"            return TOKEN(TINFLUENCE);
"activeif"              return TOKEN(TACTIVEIF);
"when"                  return TOKEN(TWHEN);
"is"                    return TOKEN(TIS);
"!"                     return TOKEN(TNOT);
">"                     { yylval.fn = 1; return CMP; }
"=>"                    { yylval.fn = 2; return CMP; }
">="                    { yylval.fn = 2; return CMP; }
"<"                     { yylval.fn = 3; return CMP; }
"<="                    { yylval.fn = 4; return CMP; }
"=<"                    { yylval.fn = 4; return CMP; }
"=="                    { yylval.fn = 5; return CMP; }
"!="                    { yylval.fn = 6; return CMP; }
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TID;
[0-9]+                  SAVE_TOKEN; return TINT;
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
";"                     return TOKEN(TSEMICOLON);
L?\"(\\.|[^\\"])*\"	    SAVE_TOKEN; return TSTRINGLIT;

.                       {cout << "Unknown token: " << *yytext << " as int: " <<int(*yytext) << endl;} yyterminate();

%%
