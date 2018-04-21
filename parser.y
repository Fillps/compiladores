/*
  Epsilon - Douglas Flores e Filipe Santos
*/
%code requires{
#include "main.h"
#include "cc_misc.h"
#include "cc_tree.h"
#include "cc_ast.h"

}

%union {
    symbol_t* valor_lexico;
    compt_tree_t* ast;
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
%token TK_OC_PIPE
%token TK_OC_PIPEG
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

%type <ast>programa
%type <ast>comando
%type <ast>funcao
%type <ast>cabecalho
%type <ast>corpo

%type <ast>if
%type <ast>while
%type <ast>for
%type <ast>foreach
%type <ast>pipe
%type <ast>do_while
%type <ast>switch
%type <ast>case
%type <ast>while_exp

%type <ast>static_opc
%type <ast>tipo
%type <ast>lista
%type <ast>parametro
%type <ast>const_opc
%type <ast>corpo
%type <ast>bloco_comando
%type <ast>comando_simples
%type <ast>id

/* Declaração dos Não-Terminais */

%start programa

%%
/* Regras (e ações) da gramática */

programa:                
    comando programa         { $$ = createASTBinaryNode(AST_PROGRAMA,NULL,$1,$$); ast=$$; }
    | %empty                 { $$ = createASTNode(AST_PROGRAMA, NULL); ast = $$; };             

comando:
    novo_tipo                { $$ = NULL; }     
    | var_global             { $$ = NULL; }
    | funcao                 { $$ = $1; };

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
    static_opc tipo TK_IDENTIFICADOR '(' lista ')'
    | static_opc tipo TK_IDENTIFICADOR '(' ')';

lista:
    parametro
    | parametro ',' lista;

parametro:
    const_opc tipo TK_IDENTIFICADOR;

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
    | ret_break_cont ';'
    | controle_fluxo ';'
    | corpo ';'
    | case;

id:
    chamada_funcao pipe /* função */
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
    | chamada_funcao pipe;

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
    TK_IDENTIFICADOR '(' chamada_parametros ')'
    | TK_IDENTIFICADOR '(' ')';

chamada_parametros:
    exp ',' chamada_parametros
    | exp;

/* Comandos de Shift */
shift:
    TK_IDENTIFICADOR shift_operador TK_LIT_INT;

shift_operador:
    TK_OC_SL
    | TK_OC_SR;

/* Comandos de retorno, break, case e continue */
ret_break_cont:
    TK_PR_RETURN exp
    | TK_PR_BREAK
    | TK_PR_CONTINUE;

case:
    TK_PR_CASE TK_LIT_INT ':';

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

/* Pipes */
pipe:
    TK_OC_PIPE funcoes_encadeadas
    | TK_OC_PIPEG funcoes_encadeadas
    | %empty;

funcoes_encadeadas:
    funcao_encadeada
    | funcao_encadeada TK_OC_PIPE funcoes_encadeadas
    | funcao_encadeada TK_OC_PIPEG funcoes_encadeadas;

funcao_encadeada:
    TK_IDENTIFICADOR '(' '.' ')'
    | TK_IDENTIFICADOR '(' '.' ',' chamada_parametros ')';

%%
