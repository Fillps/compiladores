#include <stdlib.h>
#include <memory.h>
#include <main.h>
#include "cc_code.h"
#include "cc_dict.h"
#include "cc_misc.h"
#include "cc_tree.h"
#include "cc_gv.h"
#include "cc_semantic.h"

extern int line_number;
extern comp_tree_t* ast;

comp_dict_t* symbol_table;
char** tmp_list;
int tmp_list_length;

void create_int(symbol_t* symbol, char* key, char* lexeme, int length);
void create_float(symbol_t* symbol, char* key, char* lexeme, int length);
void create_char(symbol_t* symbol, char* key, char* lexeme, int length);
void create_string(symbol_t* symbol, char* key, char* lexeme, int length);
void create_bool(symbol_t* symbol, char* key, int bool, char* lexeme, int length);
void create_id(symbol_t* symbol, char* key, char* lexeme, int length);
void create_tmp(symbol_t* symbol, char* key, int type, char* lexeme, int length);

void scope_init();

int comp_get_line_number (void)
{
  return line_number;
}

void yyerror (char const *mensagem)
{
  fprintf (stderr, "[ERROR] Line %i - %s\n", line_number, mensagem);
}

void free_symbol(symbol_t* symbol){
    free(symbol->value);
    free(symbol->lexeme);
    free(symbol);
}

void main_init (int argc, char **argv)
{
    symbol_table = dict_new();
    scope_init();
    tmp_list = calloc(DICT_SIZE, sizeof(char));
    tmp_list_length = 0;
    gv_init(GV_OUTPUT);
    if(argc > 2)
        code_init(argv[2]);
    else
        code_init(NULL);
}

void main_finalize (void)
{
    for (int i = 0; i < symbol_table->size; i++)
        if (symbol_table->data[i])
            free_symbol(dict_remove(symbol_table, (symbol_table->data[i])->key));
    dict_free(symbol_table);
    tree_free(ast);
    for (int i = 0; i < tmp_list_length; i++)
        free(tmp_list[i]);
    gv_close();
    code_close();
}

void comp_print_table (void)
{
    for (int i = 0; i < symbol_table->size; i++)
        if (symbol_table->data[i]){
            symbol_t* symbol = (symbol_t*)symbol_table->data[i]->value;
            cc_dict_etapa_2_print_entrada(symbol_table->data[i]->key, symbol->line, symbol->token);
        }
}

symbol_t* insert_symbol(int token, char* lexeme, int length){

    symbol_t* symbol = (symbol_t*)malloc(sizeof(symbol_t));
    char* key = (char*)calloc(length + 1, sizeof(char));

    symbol->line = line_number;

    switch (token){
        case TK_LIT_INT:
            create_int(symbol, key, lexeme, length); break;
        case TK_LIT_FLOAT:
            create_float(symbol, key, lexeme, length); break;
        case TK_LIT_CHAR:
            create_char(symbol, key, lexeme, length); break;
        case TK_LIT_STRING:
            create_string(symbol, key, lexeme, length); break;
        case TK_LIT_FALSE:
            create_bool(symbol, key, FALSE, lexeme, length); break;
        case TK_LIT_TRUE:
            create_bool(symbol, key, TRUE, lexeme, length); break;
        case TK_IDENTIFICADOR:
            create_id(symbol, key, lexeme, length);
    }

    symbol_t* value = dict_put(symbol_table, key, symbol);

    if (value == NULL){
        perror("Erro ao inserir o símbolo!");
        free(key);
        free_symbol(symbol);
        exit(-1);
    }
    if(value != symbol) {
        value->line = line_number;
        free_symbol(symbol);
    }

    free(key);
    return value;
}

void create_int(symbol_t* symbol, char* key, char* lexeme, int length)
{
    symbol->lexeme = strdup(lexeme);
    symbol->value = (int*)malloc(sizeof(int));
    *(int*)symbol->value = atoi(lexeme);
    symbol->token = POA_LIT_INT;
    strcpy(key, lexeme);
    key[length] = POA_LIT_INT + '0';
}

void create_float(symbol_t* symbol, char* key, char* lexeme, int length)
{
    symbol->lexeme = strdup(lexeme);
    symbol->value = (float*)malloc(sizeof(float));
    *(float*)symbol->value = atof(lexeme);
    symbol->token = POA_LIT_FLOAT;
    strcpy(key, lexeme);
    key[length] = POA_LIT_FLOAT + '0';
}

void create_char(symbol_t* symbol, char* key, char* lexeme, int length)
{
    symbol->token = POA_LIT_CHAR;
    symbol->value = (char*)malloc(sizeof(char));
    if (strcmp(lexeme, "\'\'")==0) {
        *(char *)symbol->value = '\0';
        key[0] = POA_LIT_CHAR + '0';
    }
    else{
        *(char *)symbol->value = lexeme[1];
        strcpy(key, lexeme+1);
        key[1] = POA_LIT_CHAR + '0';
    }

    symbol->lexeme = (char*)calloc(2, sizeof(char));
    symbol->lexeme[0] = *(char *)symbol->value;
    symbol->lexeme[1] = '\0';
}

void create_string(symbol_t* symbol, char* key, char* lexeme, int length)
{
    symbol->token = POA_LIT_STRING;
    if (strcmp(lexeme, "\"\"")==0) {
        symbol->value = (char*)malloc(sizeof(char));
        ((char*)symbol->value)[0] = '\0';
        key[0] = POA_LIT_STRING + '0';
    }
    else {
        symbol->value = strdup(lexeme+1);
        ((char*)symbol->value)[length - 2] = '\0';
        strcpy(key, lexeme+1);
        key[length - 2] = POA_LIT_STRING + '0';
    }
    symbol->lexeme = strdup(symbol->value);
}

void create_bool(symbol_t* symbol, char* key, int bool, char* lexeme, int length)
{
    symbol->lexeme = strdup(lexeme);
    symbol->value = (int*)malloc(sizeof(int));
    *(int*)symbol->value = bool;
    symbol->token = POA_LIT_BOOL;
    strcpy(key, lexeme);
    key[length] = POA_LIT_BOOL + '0';
}

void create_id(symbol_t* symbol, char* key, char* lexeme, int length)
{
    symbol->lexeme = strdup(lexeme);
    symbol->token = POA_IDENT;
    strcpy(key, lexeme);
    key[length] = POA_IDENT + '0';

    id_value_t* value = calloc(1, sizeof(id_value_t));
    symbol->value = value;

}

char* create_reg(){
    static int regNumber = 0;
    char *regName;
    regName = (char *)calloc(256, sizeof(char));
    sprintf(regName, "r%d", regNumber++);

    tmp_list[tmp_list_length++] = regName;

    return regName;
}

char* crate_label(){
    static int labelNumber = 0;
    char *labelName;
    labelName = (char *)calloc(256, sizeof(char));
    sprintf(labelName, "label%d", labelNumber++);

    tmp_list[tmp_list_length++] = labelName;

    return labelName;
}

int get_global_address(int size){
    static int address = 0;
    int tmp = address;
    address += size;
    return tmp;
}

int get_local_address(int size){
    static int address = 0;
    int tmp = address;
    address += size;
    return tmp;
}



