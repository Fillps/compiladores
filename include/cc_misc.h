#ifndef __MISC_H
#define __MISC_H
#include <stdio.h>

/*
  Constantes a serem utilizadas para diferenciar os lexemas que estão
  registrados na tabela de símbolos.
*/
#define POA_LIT_INT    1
#define POA_LIT_FLOAT  2
#define POA_LIT_CHAR   3
#define POA_LIT_STRING 4
#define POA_LIT_BOOL   5
#define POA_IDENT      6

#define FALSE 0
#define TRUE 1

#define GV_OUTPUT "saida.dot"

#define IKS_SUCCESS            0 //caso não houver nenhum tipo de erro

/* Verificação de declarações */
#define IKS_ERROR_UNDECLARED  1 //identificador não declarado
#define IKS_ERROR_DECLARED    2 //identificador já declarado

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

/* Tipos de declaracao de indentificadores */
#define DECL_CLASS          1
#define DECL_FUNCTION       2
#define decl_variable(type) DECL_FUNCTION + type            // onde type = POA_LIT_INT, POA_LIT_FLOAT...
#define decl_vector(type)   decl_variable(POA_IDENT) + type

#define SCOPE_SIZE 1000
#define PARAM_SIZE 100
#define UNDECLARED 0

typedef struct symbol{
    int line;
    int token;
    void * value;
    char * lexeme;
}symbol_t;

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

int getLineNumber (void);
void yyerror (char const *mensagem);
void main_init (int argc, char **argv);
void main_finalize (void);
symbol_t* insert_symbol(int token);

void start_scope();
void end_scope();

void check_declared(symbol_t* symbol, int type);
void declare(symbol_t* symbol, int type);
void declare_function(symbol_t* symbol, int type);
void declare_non_primitive(symbol_t* symbol, int type, symbol_t* class_type);

void create_params();
void add_param(symbol_t* symbol, int type);


#endif
