#ifndef __MISC_H
#define __MISC_H
#include <stdio.h>
#include "cc_dict.h"
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

#define SCOPE_SIZE 1000
#define PARAM_SIZE 100
#define FIELD_SIZE 100

#define INT_SIZE 4
#define INT_SIZE_c "4"

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
    int address[SCOPE_SIZE];
    int size;
    char* label[SCOPE_SIZE];
}id_value_t;

typedef struct tmp_value{
    int type;
    void* value;
}tmp_info_t;

typedef struct function_info{
    int type;
    int param_type[PARAM_SIZE];
    symbol_t* param_id[PARAM_SIZE];
    int params_length;
    int local_var_size;
}function_info_t;

typedef struct class_info{
    int field_type[FIELD_SIZE];
    symbol_t* field_id[FIELD_SIZE];
    int field_length;
}class_info_t;

int getLineNumber (void);
void yyerror (char const *mensagem);
void main_init (int argc, char **argv);
void main_finalize (void);
symbol_t* insert_symbol(int token, char* lexeme, int lengh);

char* create_register();
char* create_label();
int reset_local_address();
int get_global_address(int size);
int get_local_address(int size);
char* insert_minus_in_str(char* str);
void add_to_tmp_list(void *item);
void change_labels(comp_dict_t *dict, char **new_value_ptr, char* old_value);
void add_to_tmp_list(void *item);

#include "cc_tree.h"
int get_variable_type(comp_tree_t* tree);

int size_of(int type);
char* string_to_bool(char* str);

#endif
