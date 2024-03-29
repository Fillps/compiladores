/*
  Epsilon - Filipe Santos e Douglas Flores
*/
%{
#include "parser.h" //arquivo automaticamente gerado pelo bison
#include "cc_misc.h"
int line_number = 1;
%}

DIGIT       [0-9]
LETTER      [a-zA-Z]
ALFABETIC   [a-zA-Z_]
ALFANUMERIC [a-zA-Z0-9_]

INTEGER     {DIGIT}+
FLOAT       {INTEGER}\.{DIGIT}*((e|E)("+"|"-")?{INTEGER})?

%x IN_COMMENT
%%

int         return TK_PR_INT;
float       return TK_PR_FLOAT;
bool        return TK_PR_BOOL;
char        return TK_PR_CHAR;
string      return TK_PR_STRING;
if          return TK_PR_IF;
then        return TK_PR_THEN;
else        return TK_PR_ELSE;
while       return TK_PR_WHILE;
do          return TK_PR_DO;
input       return TK_PR_INPUT;
output      return TK_PR_OUTPUT;
return      return TK_PR_RETURN;
const       return TK_PR_CONST;
static      return TK_PR_STATIC;
foreach     return TK_PR_FOREACH;
for         return TK_PR_FOR;
switch      return TK_PR_SWITCH;
case        return TK_PR_CASE;
break       return TK_PR_BREAK;
continue    return TK_PR_CONTINUE;
class       return TK_PR_CLASS;
private     return TK_PR_PRIVATE;
public      return TK_PR_PUBLIC;
protected   return TK_PR_PROTECTED;

[,;:()\[\]{}+-/<>=!&%#\^\.\*]     return yytext[0];

"<="        return TK_OC_LE;
">="        return TK_OC_GE;
"=="        return TK_OC_EQ;
"!="        return TK_OC_NE;
"&&"        return TK_OC_AND;
"||"        return TK_OC_OR;
">>"        return TK_OC_SR;
"<<"        return TK_OC_SL;
"%|%"       return TK_OC_PIPE;
"%>%"       return TK_OC_PIPEG;

"\n"        line_number++;

"//".*|" "|"\t"

false       {yylval.valor_lexico = insert_symbol(TK_LIT_FALSE, yytext, yyleng); return TK_LIT_FALSE;}
true        {yylval.valor_lexico = insert_symbol(TK_LIT_TRUE, yytext, yyleng); return TK_LIT_TRUE;}

{ALFABETIC}{ALFANUMERIC}*   {yylval.valor_lexico = insert_symbol(TK_IDENTIFICADOR, yytext, yyleng); return TK_IDENTIFICADOR;}
\'.?\'                      {yylval.valor_lexico = insert_symbol(TK_LIT_CHAR, yytext, yyleng); return TK_LIT_CHAR;}
\"([^\"\n])*\"              {yylval.valor_lexico = insert_symbol(TK_LIT_STRING, yytext, yyleng); return TK_LIT_STRING;}

{INTEGER}                   {yylval.valor_lexico = insert_symbol(TK_LIT_INT, yytext, yyleng); return TK_LIT_INT;}
{FLOAT}                     {yylval.valor_lexico = insert_symbol(TK_LIT_FLOAT, yytext, yyleng); return TK_LIT_FLOAT;}

<INITIAL>{
"/*"              BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
"*/"      BEGIN(INITIAL);
[^*\n]+   // eat comment in chunks
"*"       // eat the lone star
\n        line_number++;
}

.           return TOKEN_ERRO;

%%
