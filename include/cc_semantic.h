//
// Created by filipe on 5/9/18.
//

#ifndef COMPILADOR_CC_SEMANTIC_H
#define COMPILADOR_CC_SEMANTIC_H

#include "cc_misc.h"
#include "cc_tree.h"

#define IKS_SUCCESS            0 //caso não houver nenhum tipo de erro

/* Verificação de declarações */
#define IKS_ERROR_UNDECLARED  1 //identificador não declarado
#define IKS_ERROR_DECLARED    2 //identificador já declarado
#define IKS_ERROR_ATTRIBUTE_UNDECLARED   16 //identificador deve ser declarado
#define IKS_ERROR_CLASS_UNDDECLARED 17
/* Uso correto de identificadores */
#define IKS_ERROR_VARIABLE    3 //identificador deve ser utilizado como variável
#define IKS_ERROR_VECTOR      4 //identificador deve ser utilizado como vetor
#define IKS_ERROR_FUNCTION    5 //identificador deve ser utilizado como função
#define IKS_ERROR_CLASS       15 //identificador deve ser utilizado como classe

/* Tipos e tamanho de dados */
#define IKS_ERROR_WRONG_TYPE  6 //tipos incompatíveis

/* Argumentos e parâmetros */
#define IKS_ERROR_MISSING_ARGS    9  //faltam argumentos
#define IKS_ERROR_EXCESS_ARGS     10 //sobram argumentos
#define IKS_ERROR_WRONG_TYPE_ARGS 11 //argumentos incompatívei

/* Tipos e atribuições */
#define IKS_ERROR_STRING_TO_X   7   //variável não string recebe string
#define IKS_ERROR_CHAR_TO_X     8   //variável não char recebe char

/* Expressões e tipos */
#define IKS_ERROR_INVALID_EXP   18  //expressões com tipos não compatíveis
#define IKS_ERROR_INVALID_CONDITION   19    // condições de comandos de controle de fluxo

/* Tipos de declaracao de indentificadores */
#define DECL_CLASS          1
#define DECL_FUNCTION       2
#define decl_variable(type) DECL_FUNCTION + type            // onde type = POA_LIT_INT, POA_LIT_FLOAT...
#define decl_vector(type)   decl_variable(POA_IDENT) + type

/* Tipos de operações com dois operandos */
#define CMP_ARITM       1   // ex: '>', '<=', '<', etc
#define CMP_BOOL        2   // ex: and, or
#define CMP_ARITM_BOOL  3   // ex: '==', que pode comparar tanto booleanos quanto números
#define ARITM           4   // expressões aritméticas

/* Tokens de operadores simples */
#define GREATER 1
#define LESSER  2
#define SUM     3
#define SUB     4
#define MULT    5
#define DIV     6
#define MOD     7
#define POT     8

#define SCOPE_SIZE 1000
#define PARAM_SIZE 100
#define FIELD_SIZE 100
#define UNDECLARED 0
#define GLOBAL_SCOPE 0

typedef struct id_value{
    void* value[SCOPE_SIZE];
    int type[SCOPE_SIZE];
    void* decl_info[SCOPE_SIZE];
    int scope_length;
}id_value_t;

typedef struct function_info{
    int type;
    int param_type[PARAM_SIZE];
    symbol_t* param_id[PARAM_SIZE];
    int params_length;
}function_info_t;

typedef struct class_info{
    int field_type[FIELD_SIZE];
    symbol_t* field_id[FIELD_SIZE];
    int field_length;
}class_info_t;

void scope_init();
void start_scope();
void end_scope();

void check_declared(symbol_t* symbol, int type);
void declare(symbol_t* symbol, int type);
void declare_function(symbol_t* symbol, int type);
void declare_non_primitive(symbol_t* symbol, int type, symbol_t* class_type);
void declare_class(symbol_t* symbol);

void create_params();
void add_param(symbol_t* symbol, int type);
void create_class_fields();
void class_add_field(symbol_t* symbol, int type);

void check_usage_variable(symbol_t* symbol);
void check_usage_vector(symbol_t* symbol);
void check_usage_function(comp_tree_t* tree);
void check_usage_attribute(symbol_t* class_var, symbol_t* attribute);
void check_var_assignment(symbol_t* var, comp_tree_t* exp);
void check_condition(comp_tree_t* exp, int token);

int get_var_type(symbol_t* var);
int get_func_type(comp_tree_t* tree);

void set_unary_node_value_type(comp_tree_t* node, int value_type);
void set_binary_node_value_type(comp_tree_t* node, int op_type, int op_token);

#endif //COMPILADOR_CC_SEMANTIC_H
