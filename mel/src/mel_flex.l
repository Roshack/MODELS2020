%x COMMENT_SINGLE
%x COMMENT_MULTI
%{
char* commentStart;
%}

%{
#include <string>
#include <iostream>
#include <vector>
#include <utility>
using namespace std;
#include "mel_classes.h"
#include "../obj/mel_bison.hpp"

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

<INITIAL>"#" { 
  /* begin of single-line comment */ 
  commentStart = yytext; 
  BEGIN(COMMENT_SINGLE); 
}
<COMMENT_SINGLE>\n { 
  /* end of single-line comment */
  char* comment = strndup(commentStart, yytext - commentStart);
  //printf("'%s': was a single-line comment\n", comment);
  free(comment); 
  BEGIN(INITIAL); 
}
<COMMENT_SINGLE>[^\n]+ { 
  /* suppress whatever is in the comment */
}
[ \t\r\n]                 ;
"mel.parent"                  return TOKEN(PARENTFN);
[a-zA-Z_][a-zA-Z0-9_]*  SAVE_TOKEN; return TID;
[0-9]+                  SAVE_TOKEN; return TINT;
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"["                     return TOKEN(TLBRACK);
"]"                     return TOKEN(TRBRACK);
"{"                     return TOKEN(TLBRACE);
"}"                     return TOKEN(TRBRACE);
";"                     return TOKEN(TSEMICOLON);
"."                     return TOKEN(TDOT);
","                     return TOKEN(TCOMMA);
"|-"                    return TOKEN(TIMPLIES);
":"                     return TOKEN(TCOLON);
"="                     return TOKEN(TEQUAL);
"!="                    return TOKEN(TNEQUAL);
"$"                     return TOKEN(TNODUP);
"^"                     return TOKEN(TNOASSOC);
"@"                     return TOKEN(TOPTIONAL);
"mel.contents"          return TOKEN(TCONTENTS);
"mel.nexists"           return TOKEN(TNEXISTS);
"mel.exists"            return TOKEN(TEXISTS);
"mel.ancestor"          return TOKEN(TANCEST);
L?\"(\\.|[^\\"])*\"	    SAVE_TOKEN; return TSTRINGLIT;


.                       printf("Unknown token!n"); yyterminate();

%%
