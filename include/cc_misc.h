#ifndef __MISC_H
#define __MISC_H
#include <stdio.h>

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
