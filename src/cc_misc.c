#include <stdlib.h>
#include <memory.h>
#include <main.h>
#include "cc_dict.h"
#include "cc_misc.h"
#include "cc_tree.h"
#include "cc_gv.h"

extern int line_number;
extern char* yytext;
extern int yyleng;
extern comp_tree_t* ast;

comp_dict_t* symbol_table;


void create_int(symbol_t* symbol, char* key);
void create_float(symbol_t* symbol, char* key);
void create_char(symbol_t* symbol, char* key);
void create_string(symbol_t* symbol, char* key);
void create_bool(symbol_t* symbol, char* key, int bool);
void create_id(symbol_t* symbol, char* key);

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
}

void main_finalize (void)
{
    for (int i = 0; i < symbol_table->size; i++)
        if (symbol_table->data[i])
            free_symbol(dict_remove(symbol_table, (symbol_table->data[i])->key));
    dict_free(symbol_table);
    tree_free(ast);
}

void comp_print_table (void)
{
    for (int i = 0; i < symbol_table->size; i++)
        if (symbol_table->data[i]){
            symbol_t* symbol = (symbol_t*)symbol_table->data[i]->value;
            cc_dict_etapa_2_print_entrada(symbol_table->data[i]->key, symbol->line, symbol->token);
        }
}

symbol_t* insert_symbol(int token){

    symbol_t* symbol = (symbol_t*)malloc(sizeof(symbol_t));
    char* key = (char*)calloc(yyleng + 1, sizeof(char));

    symbol->line = line_number;

    switch (token){
        case TK_LIT_INT:
            create_int(symbol, key); break;
        case TK_LIT_FLOAT:
            create_float(symbol, key); break;
        case TK_LIT_CHAR:
            create_char(symbol, key); break;
        case TK_LIT_STRING:
            create_string(symbol, key); break;
        case TK_LIT_FALSE:
            create_bool(symbol, key, FALSE); break;
        case TK_LIT_TRUE:
            create_bool(symbol, key, TRUE); break;
        case TK_IDENTIFICADOR:
            create_id(symbol, key);
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

void create_int(symbol_t* symbol, char* key)
{
    symbol->lexeme = strdup(yytext);
    symbol->value = (int*)malloc(sizeof(int));
    *(int*)symbol->value = atoi(yytext);
    symbol->token = POA_LIT_INT;
    strcpy(key, yytext);
    key[yyleng] = POA_LIT_INT + '0';
}

void create_float(symbol_t* symbol, char* key)
{
    symbol->lexeme = strdup(yytext);
    symbol->value = (float*)malloc(sizeof(float));
    *(float*)symbol->value = atof(yytext);
    symbol->token = POA_LIT_FLOAT;
    strcpy(key, yytext);
    key[yyleng] = POA_LIT_FLOAT + '0';
}

void create_char(symbol_t* symbol, char* key)
{
    symbol->token = POA_LIT_CHAR;
    symbol->value = (char*)malloc(sizeof(char));
    if (strcmp(yytext, "\'\'")==0) {
        *(char *)symbol->value = '\0';
        key[0] = POA_LIT_CHAR + '0';
    }
    else{
        *(char *)symbol->value = yytext[1];
        strcpy(key, yytext+1);
        key[1] = POA_LIT_CHAR + '0';
    }
    symbol->lexeme = strdup(symbol->value);
}

void create_string(symbol_t* symbol, char* key)
{
    symbol->token = POA_LIT_STRING;
    if (strcmp(yytext, "\"\"")==0) {
        symbol->value = (char*)malloc(sizeof(char));
        ((char*)symbol->value)[0] = '\0';
        key[0] = POA_LIT_STRING + '0';
    }
    else {
        symbol->value = strdup(yytext+1);
        ((char*)symbol->value)[yyleng - 2] = '\0';
        strcpy(key, yytext+1);
        key[yyleng - 2] = POA_LIT_STRING + '0';
    }
    symbol->lexeme = strdup(symbol->value);
}

void create_bool(symbol_t* symbol, char* key, int bool)
{
    symbol->lexeme = strdup(yytext);
    symbol->value = (int*)malloc(sizeof(int));
    *(int*)symbol->value = bool;
    symbol->token = POA_LIT_BOOL;
    strcpy(key, yytext);
    key[yyleng] = POA_LIT_BOOL + '0';
}

void create_id(symbol_t* symbol, char* key)
{
    symbol->lexeme = strdup(yytext);
    symbol->value = strdup(yytext);
    symbol->token = POA_IDENT;
    strcpy(key, yytext);
    key[yyleng] = POA_IDENT + '0';
}



