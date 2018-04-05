/*
  Epsilon - Douglas Flores e Filipe Santos
*/
%code requires{
#include "main.h"
#include "cc_misc.h"

}

%union {
    symbol_t* valor_lexico;
}

/* Declaração dos tokens da linguagem */
%token TK_PR_INT
%token TK_PR_FLOAT
%token TK_PR_BOOL
%token TK_PR_CHAR
%token TK_PR_STRING
%token TK_PR_IF
%token TK_PR_THEN
%token TK_PR_ELSE
%token TK_PR_WHILE
%token TK_PR_DO
%token TK_PR_INPUT
%token TK_PR_OUTPUT
%token TK_PR_RETURN
%token TK_PR_CONST
%token TK_PR_STATIC
%token TK_PR_FOREACH
%token TK_PR_FOR
%token TK_PR_SWITCH
%token TK_PR_CASE
%token TK_PR_BREAK
%token TK_PR_CONTINUE
%token TK_PR_CLASS
%token TK_PR_PRIVATE
%token TK_PR_PUBLIC
%token TK_PR_PROTECTED
%token TK_OC_LE
%token TK_OC_GE
%token TK_OC_EQ
%token TK_OC_NE
%token TK_OC_AND
%token TK_OC_OR
%token TK_OC_SL
%token TK_OC_SR
%token TK_OC_XOR
%token TK_OC_RR
%token TK_LIT_INT
%token TK_LIT_FLOAT
%token TK_LIT_FALSE
%token TK_LIT_TRUE
%token TK_LIT_CHAR
%token TK_LIT_STRING
%token TK_IDENTIFICADOR
%token TOKEN_ERRO

%left '>' '<' TK_OC_LE TK_OC_GE TK_OC_EQ TK_OC_NE TK_OC_AND TK_OC_OR
%left '+' '-'
%left '*' '/' '%'
%left '^'
%right UMINUS

%left '.'

/* Declaração dos Não-Terminais */

%start programa

%%
/* Regras (e ações) da gramática */

programa:
    comando programa
    | %empty;

comando:
    novo_tipo
    | var_global
    | funcao;

/* Novo Tipo */
novo_tipo:
    TK_PR_CLASS TK_IDENTIFICADOR '[' campo ']' ';';

campo:
    encapsulamento tipo TK_IDENTIFICADOR
    | encapsulamento tipo TK_IDENTIFICADOR ':' campo;

encapsulamento:
    TK_PR_PRIVATE
    | TK_PR_PUBLIC
    | TK_PR_PROTECTED;

tipo:
    TK_PR_FLOAT
    | TK_PR_INT
    | TK_PR_BOOL
    | TK_PR_CHAR
    | TK_PR_STRING;

/* Variável Global */
var_global:
    static_opc tipo TK_IDENTIFICADOR vetor_global ';'
    | static_opc TK_IDENTIFICADOR TK_IDENTIFICADOR vetor_global ';';

static_opc:
    %empty
    | TK_PR_STATIC;

vetor_global:
    %empty
    | '[' TK_LIT_INT ']';

/* Função */
funcao:
    cabecalho corpo;

cabecalho:
    static_opc tipo TK_IDENTIFICADOR '(' lista ')';

lista:
    parametro
    | parametro ',' lista;

parametro:
    %empty
    | const_opc tipo TK_IDENTIFICADOR;

const_opc:
    %empty
    | TK_PR_CONST;

/* Blocos de Comando */
corpo:
    '{' bloco_comando '}';

bloco_comando:
    %empty
    | comando_simples bloco_comando;

comando_simples:
    var_declaracao_primitiva ';'
    | atribuicao ';'
    | id ';'
    | input ';'
    | output ';'
    | retorno
    | controle_fluxo;

id:
    chamada_funcao /* função */
    | TK_IDENTIFICADOR TK_IDENTIFICADOR // variável de tipo não-primitivo
    | shift;

var_declaracao_primitiva:
    static_opc const_opc tipo TK_IDENTIFICADOR var_atribuicao;

var_atribuicao:
    %empty
    | TK_OC_LE var_valor;

var_valor:
    TK_IDENTIFICADOR
    | TK_LIT_INT
    | TK_LIT_FLOAT
    | TK_LIT_CHAR
    | TK_LIT_FALSE
    | TK_LIT_TRUE
    | TK_LIT_STRING;

/* Atribuição */
atribuicao:
    var '=' exp;

var:
    TK_IDENTIFICADOR
    | TK_IDENTIFICADOR '[' exp ']'
    | var '.' var;

exp: 
    exp TK_OC_LE exp
    | exp TK_OC_GE exp
    | exp TK_OC_EQ exp
    | exp TK_OC_NE exp
    | exp TK_OC_AND exp
    | exp TK_OC_OR exp
    | exp '+' exp
    | exp '-' exp
    | exp '*' exp
    | exp '/' exp
    | exp '%' exp
    | exp '^' exp
    | '(' exp ')'
    | '+' exp %prec UMINUS
    | '-' exp %prec UMINUS
    | '!' exp %prec UMINUS
    | var
    | TK_LIT_INT
    | TK_LIT_FLOAT
    | TK_LIT_STRING
    | TK_LIT_CHAR
    | TK_LIT_TRUE
    | TK_LIT_FALSE
    | chamada_funcao;

exp_lista:
    exp ',' exp_lista
    | exp;

/* Input e Output */
output:
    TK_PR_OUTPUT exp_lista;

input:
    TK_PR_INPUT exp;

/* Chamada de função */
chamada_funcao:
    TK_IDENTIFICADOR '(' chamada_parametros ')';

chamada_parametros:
    chamada_parametro
    | chamada_parametro ',' chamada_parametros;

chamada_parametro:
    %empty
    | var_valor
    | '.';

/* Comandos de Shift */
shift:
    TK_IDENTIFICADOR shift_operador TK_LIT_INT;

shift_operador:
    TK_OC_SL
    | TK_OC_SR;

/* Comandos de retorno, break, case e continue */
comando_retorno:
    TK_PR_RETURN exp ';';

retorno:
    comando_retorno
    | TK_PR_BREAK ';'
    | TK_PR_CONTINUE ';'
    | TK_PR_CASE TK_LIT_INT ':';

/* Controle de Fluxo */
controle_fluxo:
    if
    | switch
    | while
    | do_while
    | foreach
    | for;

/* IF */
if:
    TK_PR_IF '(' exp ')' TK_PR_THEN corpo
    | TK_PR_IF '(' exp ')' TK_PR_THEN corpo TK_PR_ELSE corpo;

/* SWITCH */
switch:
    TK_PR_SWITCH '(' exp ')' corpo;

/* WHILE e DO-WHILE */
while_exp:
    TK_PR_WHILE '(' exp ')';

while:
    while_exp TK_PR_DO corpo;

do_while:
    TK_PR_DO corpo while_exp;

/* FOREACH */
foreach:
    TK_PR_FOREACH '(' TK_IDENTIFICADOR ':' exp_lista ')' corpo;

/* FOR */
for:
    TK_PR_FOR '(' lista_comandos ':' exp ':' lista_comandos ')' corpo;

for_comando: 
    var_declaracao_primitiva
    | atribuicao
    | id
    | TK_PR_BREAK
    | TK_PR_CONTINUE
    | TK_PR_RETURN exp
    | controle_fluxo;

lista_comandos: 
    for_comando
    | for_comando ',' lista_comandos;


    
























%%
