/*
  Epsilon - Douglas Flores e Filipe Santos
*/
%code requires{
#include "main.h"
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
%token TK_OC_XOR    // token de %|%
%token TK_OC_RR     // rotate right (%>%)
%token TK_LIT_INT
%token TK_LIT_FLOAT
%token TK_LIT_FALSE
%token TK_LIT_TRUE
%token TK_LIT_CHAR
%token TK_LIT_STRING
%token TK_IDENTIFICADOR
%token TOKEN_ERRO

/* Declaração dos Não-Terminais */

%start programa

%%
/* Regras (e ações) da gramática */

programa: novo_tipo | var_global | funcao;

/* Novo Tipo */
novo_tipo: TK_PR_CLASS TK_IDENTIFICADOR '[' campo ']' ';';
campo: encapsulamento tipo TK_IDENTIFICADOR | encapsulamento tipo TK_IDENTIFICADOR ':' campo;
encapsulamento: TK_PR_PRIVATE | TK_PR_PUBLIC | TK_PR_PROTECTED;
tipo: TK_PR_FLOAT | TK_PR_INT | TK_PR_BOOL | TK_PR_CHAR | TK_PR_STRING;

/* Variável Global */
var_global: static_opc var_tipo TK_IDENTIFICADOR vetor_global ';';
static_opc: %empty | TK_PR_STATIC;
var_tipo: tipo | TK_IDENTIFICADOR;

vetor_global: %empty | '[' TK_LIT_INT ']';

/* Função */
funcao: cabecalho corpo;
cabecalho: static_opc tipo TK_IDENTIFICADOR '(' lista ')';
lista: parametro | parametro ',' lista;
parametro: const_opc tipo TK_IDENTIFICADOR;
const_opc: %empty | TK_PR_CONST;
corpo: bloco_comando;
bloco_comando: %empty;

%%
