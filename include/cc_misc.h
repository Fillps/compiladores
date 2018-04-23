#ifndef __MISC_H
#define __MISC_H
#include <stdio.h>
#include "cc_tree.h"

#define FALSE 0
#define TRUE 1

typedef struct symbol{
    int line;
    int token;
    void * value;
}symbol_t;

typedef struct nodeAST{
    int type;
    symbol_t *symbol;
}nodeAST;

int getLineNumber (void);
void yyerror (char const *mensagem);
void main_init (int argc, char **argv);
void main_finalize (void);
symbol_t* insert_symbol(int token);

comp_tree_t* createASTNode(int type, symbol_t* token);
comp_tree_t* createASTUnaryNode(int type, symbol_t* token, comp_tree_t* node);
comp_tree_t* createASTBinaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2);
comp_tree_t* createASTTernaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2, comp_tree_t* node3);
comp_tree_t* createASTQuaternaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2, comp_tree_t* node3, comp_tree_t* node4);

#endif
