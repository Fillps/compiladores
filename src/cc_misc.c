#include <stdlib.h>
#include <memory.h>
#include <main.h>
#include "cc_dict.h"
#include "cc_misc.h"

extern int line_number;
extern char* yytext;
extern int yyleng;

comp_dict_t* symbol_table;

int comp_get_line_number (void)
{
  return line_number;
}

void yyerror (char const *mensagem)
{
  fprintf (stderr, "%s\n", mensagem); //altere para que apareça a linha
}

void main_init (int argc, char **argv)
{
    symbol_table = dict_new();
}

void main_finalize (void)
{
    for (int i = 0; i < symbol_table->size; i++)
        if (symbol_table->data[i])
            free(dict_remove(symbol_table, (symbol_table->data[i])->key));
    dict_free(symbol_table);
}

void comp_print_table (void)
{
  //para cada entrada na tabela de símbolos
  //Etapa 1: chame a função cc_dict_etapa_1_print_entrada
    for (int i = 0; i < symbol_table->size; i++)
        if (symbol_table->data[i])
            cc_dict_etapa_1_print_entrada(symbol_table->data[i]->key, *((int*)symbol_table->data[i]->value));
}

void insert_symbol(void){
    if (strcmp(yytext, "\"\"")==0 || strcmp(yytext, "\'\'")==0)
        return;

    char* key = (char*)malloc(yyleng*sizeof(char));
    int* line = (int*)malloc(sizeof(int));

    *line = line_number;

    if(yytext[0]=='\"' || yytext[0]=='\'') {
        strncpy(key, yytext + 1, yyleng - 2);
        key[yyleng-2] = '\0';
    }
    else
        strcpy(key, yytext);

    int* value = dict_put(symbol_table, key, line);
    if(value != line){
        *value = line_number;
        free(line);
    }
    free(key);
}
