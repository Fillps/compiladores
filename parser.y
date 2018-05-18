/*
  Epsilon - Douglas Flores e Filipe Santos
*/
%code requires{
#include "main.h"
#include "cc_misc.h"
#include "cc_tree.h"
#include "cc_ast.h"
#include "cc_semantic.h"

comp_tree_t* ast;
}

%union {
    symbol_t* valor_lexico;
    comp_tree_t* ast;
    int type;
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
%token <valor_lexico>TK_LIT_INT
%token <valor_lexico>TK_LIT_FLOAT
%token <valor_lexico>TK_LIT_FALSE
%token <valor_lexico>TK_LIT_TRUE
%token <valor_lexico>TK_LIT_CHAR
%token <valor_lexico>TK_LIT_STRING
%token <valor_lexico>TK_IDENTIFICADOR
%token TOKEN_ERRO

%left '>' '<' TK_OC_LE TK_OC_GE TK_OC_EQ TK_OC_NE TK_OC_AND TK_OC_OR
%left '+' '-'
%left '*' '/' '%'
%left '^'
%right UMINUS

%type <ast>programa
%type <ast>comandos
%type <ast>comando
%type <ast>identificador
%type <ast>literal
%type <ast>literal_int
%type <ast>literal_int_neg
%type <ast>literal_char
%type <ast>funcao
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
%type <ast>bloco_comando
%type <ast>comando_simples
%type <ast>var_declaracao_primitiva
%type <ast>var_valor
%type <ast>id
%type <ast>atribuicao
%type <ast>input
%type <ast>output
%type <ast>chamada_funcao
%type <ast>ret_break_cont
%type <ast>controle_fluxo
%type <ast>shift
%type <ast>exp
%type <ast>var
%type <ast>exp_lista
%type <ast>lista_comandos
%type <ast>for_comando
%type <ast>chamada_parametros
%type <ast>funcao_encadeada
%type <ast>funcoes_encadeadas

%type <type>tipo
%type <type>vetor_global
/* Declaração dos Não-Terminais */

%start programa

%%
/* Regras (e ações) da gramática */

programa:
    comandos    { if ($1 != NULL) $$ = createASTUnaryNode(AST_PROGRAMA, NULL, $1); else $$ = createASTNode(AST_PROGRAMA, NULL); ast = $$; }
    | %empty    { $$ = createASTNode(AST_PROGRAMA, NULL); ast = $$; };

comandos:
    comando  comandos        {
                                if ($1 != NULL && $2 != NULL) {
                                    tree_insert_node($1, $2);
                                    $$ = $1;
                                }
                                else if ($1 != NULL)
                                    $$ = $1;
                                else if ($2 != NULL)
                                    $$ = $2;
                                else
                                    $$ = NULL;
                             }
    | comando                {
                                if ($1 != NULL)
                                    $$ = $1;
                                else
                                    $$ = NULL;
                             };

comando:
    novo_tipo                { $$ = NULL; }
    | var_global             { $$ = NULL; }
    | funcao                 { $$ = $1; };

identificador:
    TK_IDENTIFICADOR { $$ = createASTNode(AST_IDENTIFICADOR, $1); };

literal:
    TK_LIT_TRUE     { $$ = createASTNode(AST_LITERAL, $1); set_unary_node_value_type($$, decl_variable(POA_LIT_BOOL)); }
    | TK_LIT_FALSE  { $$ = createASTNode(AST_LITERAL, $1); set_unary_node_value_type($$, decl_variable(POA_LIT_BOOL)); }
    | literal_int   { $$ = $1; set_unary_node_value_type($$, decl_variable(POA_LIT_INT)); }
    | TK_LIT_FLOAT  { $$ = createASTNode(AST_LITERAL, $1); set_unary_node_value_type($$, decl_variable(POA_LIT_FLOAT)); }
    | TK_LIT_STRING { $$ = createASTNode(AST_LITERAL, $1); set_unary_node_value_type($$, decl_variable(POA_LIT_STRING)); }
    | literal_char  { $$ = $1; set_unary_node_value_type($$, decl_variable(POA_LIT_CHAR)); };

literal_int:
    TK_LIT_INT      { $$ = createASTNode(AST_LITERAL, $1); };

literal_int_neg:
    '-' literal_int { $$ = createASTUnaryNode(AST_ARIM_INVERSAO, NULL, $2); };

literal_char:
    TK_LIT_CHAR     { $$ = createASTNode(AST_LITERAL, $1); };

/* Novo Tipo */
novo_tipo:
    TK_PR_CLASS TK_IDENTIFICADOR { create_class_fields(); }'[' campo ']' ';' { declare_class($2); };

campo:
    encapsulamento tipo TK_IDENTIFICADOR                { class_add_field($3, $2); }
    | encapsulamento tipo TK_IDENTIFICADOR ':' campo    { class_add_field($3, $2); };

encapsulamento:
    TK_PR_PRIVATE
    | TK_PR_PUBLIC
    | TK_PR_PROTECTED;

tipo:
    TK_PR_FLOAT     { $$ = POA_LIT_FLOAT; }
    | TK_PR_INT     { $$ = POA_LIT_INT; }
    | TK_PR_BOOL    { $$ = POA_LIT_BOOL; }
    | TK_PR_CHAR    { $$ = POA_LIT_CHAR; }
    | TK_PR_STRING  { $$ = POA_LIT_STRING; };

/* Variável Global */
var_global:
    static_opc tipo TK_IDENTIFICADOR vetor_global ';'               {
                                                                        if($4 == FALSE)
                                                                            declare($3, decl_variable($2));
                                                                        else
                                                                            declare($3, decl_vector($2));
                                                                    }
    | static_opc TK_IDENTIFICADOR TK_IDENTIFICADOR vetor_global ';' {
                                                                        check_declared($2, DECL_CLASS);
                                                                        if($4 == FALSE)
                                                                            declare($3, decl_variable($2->token));
                                                                        else
                                                                            declare($3, decl_vector($2->token));
                                                                    };

static_opc:
    %empty
    | TK_PR_STATIC;

vetor_global:
    %empty                  { $$ = FALSE; }
    | '[' TK_LIT_INT ']'    { $$ = POA_LIT_INT; };

/* Função */
funcao:
    static_opc tipo TK_IDENTIFICADOR { create_params(); } params { declare_function($3, $2); } corpo  { if ($7 != NULL)
                                                                                                            $$ = createASTUnaryNode(AST_FUNCAO, $3, $7);
                                                                                                        else
                                                                                                            $$ = createASTNode(AST_FUNCAO, $3);
                                                                                                      };

params:
     '(' param_lista ')'
    |'(' ')';

param_lista:
    parametro
    | parametro ',' param_lista;

parametro:
    const_opc tipo TK_IDENTIFICADOR { add_param($3, $2); };

const_opc:
    %empty
    | TK_PR_CONST;

/* Blocos de Comando */
corpo:
    { start_scope(); } '{' bloco_comando '}' { end_scope(); $$ = $3; };

bloco_comando:
    %empty                          { $$ = NULL; }
    | comando_simples bloco_comando {
                                        if ($1 && $2) {
                                            tree_insert_node($1, $2);
                                            $$ = $1;
                                        }
                                        else if ($1 != NULL)
                                            $$ = $1;
                                        else if ($2 != NULL)
                                            $$ = $2;
                                        else
                                            $$ = NULL;
                                    };

comando_simples:
    var_declaracao_primitiva ';'    { $$ = $1; }
    | atribuicao ';'                { $$ = $1; }
    | id ';'                        { $$ = $1; }
    | chamada_funcao ';'            { $$ = $1; }
    | input ';'                     { $$ = $1; }
    | output ';'                    { $$ = $1; }
    | ret_break_cont ';'            { $$ = $1; }
    | controle_fluxo ';'            { $$ = $1; }
    | corpo ';'                     { if ($1) $$ = createASTUnaryNode(AST_BLOCO, NULL, $1); else $$ = NULL; }
    | case                          { $$ = $1; }
    | pipe ';'                      { $$ = $1; }
    | shift ';'                     { $$ = $1; };

id:
    TK_IDENTIFICADOR TK_IDENTIFICADOR { $$ = NULL;  // variável de tipo não-primitivo
                                        check_declared($1, DECL_CLASS);
                                        declare_non_primitive($2, decl_variable($1->token), $1);
                                      };

var_declaracao_primitiva:
    static_opc const_opc tipo identificador TK_OC_LE var_valor   { $$ = createASTBinaryNode(AST_ATRIBUICAO, NULL, $4, $6); declare($4->value->symbol, decl_variable($3)); }
    | static_opc const_opc tipo TK_IDENTIFICADOR                 { $$ = NULL; declare($4, decl_variable($3)); };

var_valor:
    identificador   { $$ = $1; }
    | literal       { $$ = $1; };

/* Atribuição */
atribuicao:
    var '=' exp { $$ = createASTBinaryNode(AST_ATRIBUICAO, NULL, $1, $3); check_var_assignment($1->value->symbol, $3); };

var:
    identificador                       { $$ = $1; check_usage_variable($1->value->symbol); }
    | identificador '[' exp ']'         { $$ = createASTBinaryNode(AST_VETOR_INDEXADO, NULL, $1, $3); check_usage_vector($1->value->symbol); }
    | identificador '.' identificador   { $$ = createASTBinaryNode(AST_ATRIBUTO, NULL, $1, $3); check_usage_attribute($1->value->symbol, $3->value->symbol); };


exp:
    exp TK_OC_LE exp        { $$ = createASTBinaryNode(AST_LOGICO_COMP_LE, NULL, $1, $3); set_binary_node_value_type($$, CMP_ARITM); }
    | exp TK_OC_GE exp      { $$ = createASTBinaryNode(AST_LOGICO_COMP_GE, NULL, $1, $3); set_binary_node_value_type($$, CMP_ARITM); }
    | exp TK_OC_EQ exp      { $$ = createASTBinaryNode(AST_LOGICO_COMP_IGUAL, NULL, $1, $3); set_binary_node_value_type($$, CMP_ARITM_BOOL); }
    | exp TK_OC_NE exp      { $$ = createASTBinaryNode(AST_LOGICO_COMP_DIF, NULL, $1, $3); set_binary_node_value_type($$, CMP_ARITM_BOOL); }
    | exp TK_OC_AND exp     { $$ = createASTBinaryNode(AST_LOGICO_E, NULL, $1, $3); set_binary_node_value_type($$, CMP_BOOL); }
    | exp TK_OC_OR exp      { $$ = createASTBinaryNode(AST_LOGICO_OU, NULL, $1, $3); set_binary_node_value_type($$, CMP_BOOL); }
    | exp '>' exp           { $$ = createASTBinaryNode(AST_LOGICO_COMP_G, NULL, $1, $3); set_binary_node_value_type($$, CMP_ARITM); }
    | exp '<' exp           { $$ = createASTBinaryNode(AST_LOGICO_COMP_L, NULL, $1, $3); set_binary_node_value_type($$, CMP_ARITM); }
    | exp '+' exp           { $$ = createASTBinaryNode(AST_ARIM_SOMA, NULL, $1, $3); set_binary_node_value_type($$, ARITM); }
    | exp '-' exp           { $$ = createASTBinaryNode(AST_ARIM_SUBTRACAO, NULL, $1, $3); set_binary_node_value_type($$, ARITM); }
    | exp '*' exp           { $$ = createASTBinaryNode(AST_ARIM_MULTIPLICACAO, NULL, $1, $3); set_binary_node_value_type($$, ARITM); }
    | exp '/' exp           { $$ = createASTBinaryNode(AST_ARIM_DIVISAO, NULL, $1, $3); set_binary_node_value_type($$, ARITM); }
    | exp '%' exp           { $$ = createASTBinaryNode(AST_ARIM_MOD, NULL, $1, $3); set_binary_node_value_type($$, ARITM); }
    | exp '^' exp           { $$ = createASTBinaryNode(AST_ARIM_POT, NULL, $1, $3); set_binary_node_value_type($$, ARITM); }
    | '(' exp ')'           { $$ = $2; set_unary_node_value_type($$, $2->value->value_type); }
    | '+' exp %prec UMINUS  { $$ = $2; set_unary_node_value_type($$, $2->value->value_type); }
    | '-' exp %prec UMINUS  { $$ = createASTUnaryNode(AST_ARIM_INVERSAO, NULL, $2); set_unary_node_value_type($$, $2->value->value_type); }
    | '!' exp %prec UMINUS  { $$ = createASTUnaryNode(AST_LOGICO_COMP_NEGACAO, NULL, $2); set_unary_node_value_type($$, $2->value->value_type); }
    | var                   { $$ = $1; set_unary_node_value_type($$, get_var_type($1->value->symbol)); }
    | literal               { $$ = $1; set_unary_node_value_type($$, $1->value->value_type); }
    | chamada_funcao        { $$ = $1; set_unary_node_value_type($$, get_func_type($1)); }
    | pipe                  { $$ = $1; };

exp_lista:
    exp ',' exp_lista       { $$ = $1; tree_insert_node($1, $3); }
    | exp                   { $$ = $1; };

/* Input e Output */
output:
    TK_PR_OUTPUT exp_lista  { $$ = createASTUnaryNode(AST_OUTPUT, NULL, $2); };

input:
    TK_PR_INPUT exp         { $$ = createASTUnaryNode(AST_INPUT, NULL, $2); };

/* Chamada de função */
chamada_funcao:
    identificador '(' chamada_parametros ')' { $$ = createASTBinaryNode(AST_CHAMADA_DE_FUNCAO, NULL, $1, $3); update_childs($$); check_usage_function($$); }
    | identificador '(' ')'                  { $$ = createASTUnaryNode(AST_CHAMADA_DE_FUNCAO, NULL, $1); check_usage_function($$); };

chamada_parametros:
    exp ',' chamada_parametros  { $$ = $1; tree_make_next($1, $3); }
    | exp                       { $$ = $1; };

/* Comandos de Shift */
shift:
    identificador TK_OC_SL literal_int    { $$ = createASTBinaryNode(AST_SHIFT_LEFT, NULL, $1, $3); }
    | identificador TK_OC_SR literal_int  { $$ = createASTBinaryNode(AST_SHIFT_RIGHT, NULL, $1, $3); };

/* Comandos de retorno, break, case e continue */
ret_break_cont:
    TK_PR_RETURN exp    { $$ = createASTUnaryNode(AST_RETURN, NULL, $2); }
    | TK_PR_BREAK       { $$ = createASTNode(AST_BREAK, NULL); }
    | TK_PR_CONTINUE    { $$ = createASTNode(AST_CONTINUE, NULL); };

case:
    TK_PR_CASE literal_int ':'      { $$ = createASTUnaryNode(AST_CASE, NULL, $2); }
    | TK_PR_CASE literal_int_neg ':'{ $$ = createASTUnaryNode(AST_CASE, NULL, $2); }
    | TK_PR_CASE literal_char ':'   { $$ = createASTUnaryNode(AST_CASE, NULL, $2); }

/* Controle de Fluxo */
controle_fluxo:
    if          { $$ = $1; }
    | switch    { $$ = $1; }
    | while     { $$ = $1; }
    | do_while  { $$ = $1; }
    | foreach   { $$ = $1; }
    | for       { $$ = $1; };

/* IF */
if:
    TK_PR_IF '(' exp ')' TK_PR_THEN corpo                       {
                                                                    if ($6 != NULL)
                                                                        $$ = createASTBinaryNode(AST_IF_ELSE, NULL, $3, $6);
                                                                    else {
                                                                        tree_free($3);
                                                                        $$ = NULL;
                                                                    }
                                                                }
    | TK_PR_IF '(' exp ')' TK_PR_THEN corpo TK_PR_ELSE corpo    {
                                                                    if ($6 && $8)
                                                                        $$ = createASTTernaryNode(AST_IF_ELSE, NULL, $3, $6, $8);
                                                                    else if ($6)
                                                                        $$ = createASTBinaryNode(AST_IF_ELSE, NULL, $3, $6);
                                                                    else if ($8)
                                                                        $$ = createASTBinaryNode(AST_IF_ELSE, NULL, createASTUnaryNode(AST_LOGICO_COMP_NEGACAO, NULL, $3), $8);
                                                                    else
                                                                        $$ = NULL;
                                                                };
/* SWITCH */
switch:
    TK_PR_SWITCH '(' exp ')' corpo    {
                                        if ($5)
                                            $$ = createASTBinaryNode(AST_SWITCH, NULL, $3, $5);
                                        else{
                                            tree_free($3);
                                            $$ = NULL;
                                        }
                                      };

/* WHILE e DO-WHILE */
while_exp:
    TK_PR_WHILE '(' exp ')'    { $$ = $3; };

while:
    while_exp TK_PR_DO corpo    {
                                    if ($3)
                                        $$ = createASTBinaryNode(AST_WHILE_DO, NULL, $1, $3);
                                    else{
                                        tree_free($1);
                                        $$ = NULL;
                                    }
                                };

do_while:
    TK_PR_DO corpo while_exp    {
                                    if ($2)
                                        $$ = createASTBinaryNode(AST_DO_WHILE, NULL, $2, $3);
                                    else{
                                        tree_free($3);
                                        $$ = NULL;
                                    }
                                };

/* FOREACH */
foreach:
    TK_PR_FOREACH '(' identificador ':' exp_lista ')' corpo    {
                                                                if ($7)
                                                                    $$ = createASTTernaryNode(AST_FOREACH, NULL, $3, $5, $7);
                                                                else{
                                                                    tree_free($3);
                                                                    tree_free($5);
                                                                    $$ = NULL;
                                                                }
                                                                };

/* FOR */
for:
    TK_PR_FOR '(' lista_comandos ':' exp ':' lista_comandos ')' corpo   {
                                                                            if ($7)
                                                                                $$ = createASTQuaternaryNode(AST_FOR, NULL, $3, $5, $7, $9);
                                                                            else{
                                                                                tree_free($3);
                                                                                tree_free($5);
                                                                                tree_free($7);
                                                                                $$ = NULL;
                                                                            }
                                                                            };

for_comando:
    var_declaracao_primitiva    { $$ = $1; }
    | atribuicao                { $$ = $1; }
    | id                        { $$ = $1; }
    | chamada_funcao            { $$ = $1; }
    | pipe                      { $$ = $1; }
    | shift                     { $$ = $1; }
    | ret_break_cont            { $$ = $1; }
    | controle_fluxo            { $$ = $1; };

lista_comandos:
    for_comando                         { $$ = $1; }
    | for_comando ',' lista_comandos    { $$ = $1; tree_insert_node($1, $3); };

/* Pipes */
pipe:
    chamada_funcao TK_OC_PIPE funcoes_encadeadas      { $$ = tree_finish_pipe($1, $3, AST_ENCADEAMENTO_PIPE); }
    | chamada_funcao TK_OC_PIPEG funcoes_encadeadas   { $$ = tree_finish_pipe($1, $3, AST_ENCADEAMENTO_PIPEG); };

funcoes_encadeadas:
    funcao_encadeada                                    { $$ = $1; }
    | funcoes_encadeadas TK_OC_PIPE funcao_encadeada    {
                                                            tree_insert_node_as_second_child($3, $1);
                                                            $$ = createASTUnaryNode(AST_ENCADEAMENTO_PIPE, NULL, $3);
                                                        }
    | funcoes_encadeadas TK_OC_PIPEG funcao_encadeada   {
                                                            tree_insert_node_as_second_child($3, $1);
                                                            $$ = createASTUnaryNode(AST_ENCADEAMENTO_PIPEG, NULL, $3);
                                                        };

funcao_encadeada:
    identificador '(' '.' ')'                             { $$ = createASTUnaryNode(AST_CHAMADA_DE_FUNCAO, NULL, $1); }
    | identificador '(' '.' ',' chamada_parametros ')'    { $$ = createASTBinaryNode(AST_CHAMADA_DE_FUNCAO, NULL, $1, $5); };

%%
