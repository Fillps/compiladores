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

typedef struct symbol{
    int line;
    int token;
    void * value;
    char * lexeme;
}symbol_t;

int getLineNumber (void);
void yyerror (char const *mensagem);
void main_init (int argc, char **argv);
void main_finalize (void);
symbol_t* insert_symbol(int token);

#endif
