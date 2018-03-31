#include <stdlib.h>
#include <memory.h>
#include <main.h>
#include "cc_dict.h"
#include "cc_misc.h"

extern int line_number;
extern char* yytext;
extern int yyleng;

typedef struct symbol{
    int line;
    int token;
    void * value;
}symbol_t;

comp_dict_t* symbol_table;

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
}

void comp_print_table (void)
{
  //para cada entrada na tabela de símbolos
  //Etapa 1: chame a função cc_dict_etapa_1_print_entrada
    for (int i = 0; i < symbol_table->size; i++)
        if (symbol_table->data[i]){
            symbol_t* symbol = (symbol_t*)symbol_table->data[i]->value;
            cc_dict_etapa_2_print_entrada(symbol_table->data[i]->key, symbol->line, symbol->token);
        }

}

void insert_symbol(int token){

    symbol_t* symbol = (symbol_t*)malloc(sizeof(symbol_t));
    char* key = (char*)malloc((yyleng + 1)*sizeof(char));

    symbol->line = line_number;

    switch (token){
        case TK_LIT_INT:
            symbol->value = (int*)malloc(sizeof(int));
            *(int*)symbol->value = atoi(yytext);
            symbol->token = POA_LIT_INT;
            strcpy(key, yytext);
            key[yyleng] = POA_LIT_INT + '0';
            key[yyleng + 1] = '\0';
            break;
        case TK_LIT_FLOAT:
            symbol->value = (float*)malloc(sizeof(float));
            *(float*)symbol->value = atof(yytext);
            symbol->token = POA_LIT_FLOAT;
            strcpy(key, yytext);
            key[yyleng] = POA_LIT_FLOAT + '0';
            key[yyleng + 1] = '\0';
            break;
        case TK_LIT_CHAR:
            if (strcmp(yytext, "\'\'")==0){
                free(symbol);
                free(key);
                return;
            }
            symbol->value = (char*)malloc(sizeof(char));
            *(int*)symbol->value = yytext[1];
            symbol->token = POA_LIT_CHAR;
            strcpy(key, yytext+1);
            key[1] = POA_LIT_CHAR + '0';
            key[2] = '\0';
            break;
        case TK_LIT_STRING:
            if (strcmp(yytext, "\"\"")==0) {
                free(symbol);
                free(key);
                return;
            }
            symbol->value = strdup(yytext+1);
            ((char*)symbol->value)[yyleng - 1] = '\0';
            symbol->token = POA_LIT_STRING;
            strcpy(key, yytext+1);
            key[yyleng - 2] = POA_LIT_STRING + '0';
            key[yyleng - 1] = '\0';
            break;
        case TK_IDENTIFICADOR:
            symbol->value = strdup(yytext);
            symbol->token = POA_IDENT;
            strcpy(key, yytext);
            key[yyleng] = POA_IDENT + '0';
            key[yyleng + 1] = '\0';
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
}
