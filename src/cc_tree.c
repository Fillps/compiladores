// Copyright (c) 2016 Lucas Nodari
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <stdlib.h>
#include <cc_misc.h>
#include <cc_ast.h>
#include <cc_gv.h>
#include <memory.h>


#include "cc_tree.h"
#include "parser.h"

extern int line_number;
extern FILE *intfp;
void *comp_tree_last = NULL;

#define ERRO(MENSAGEM) { fprintf (stderr, "[cc_tree, %s] %s.\n", __FUNCTION__, MENSAGEM); abort(); }

comp_tree_t* tree_new(void){
	comp_tree_t *tree = tree_make_node(NULL);
	return tree;
}

void tree_free(comp_tree_t *tree){
	comp_tree_t *ptr = tree;
	do {
		if (ptr->first != NULL)
			tree_free(ptr->first);
		ptr = ptr->next;
		free(tree->value);
		free(tree);
		tree = ptr;
	} while(ptr != NULL);
}

comp_tree_t* tree_make_node(void *value){
	comp_tree_t *node = malloc(sizeof(comp_tree_t));
	if (!node)
		ERRO("Failed to allocate memory for tree node");

	node->value = value;
	node->childnodes = 0;
	node->first = NULL;
	node->last = NULL;
	node->next = NULL;
	node->prev = NULL;

	return node;
}

void tree_insert_node(comp_tree_t *tree, comp_tree_t *node){
	if (tree == NULL)
		ERRO("Cannot insert node, tree is null");
	if (node == NULL)
		ERRO("Cannot insert node, node is null");

	if (tree_has_child_nodes(tree)){
		tree->first = node;
		tree->last = node;
	} else {
		node->prev = tree->last;
		tree->last->next = node;
		tree->last = node;
	}
	++tree->childnodes;

	comp_tree_last = tree;
}

int tree_has_child_nodes(comp_tree_t *tree){
	if (tree != NULL){
		if (tree->childnodes == 0)
			return 1;
	}
	return 0;
}

comp_tree_t* tree_make_unary_node(void *value, comp_tree_t *node){
	comp_tree_t *newnode = tree_make_node(value);
	tree_insert_node(newnode,node);
	return newnode;
}

comp_tree_t* tree_make_binary_node(void *value, comp_tree_t *node1, comp_tree_t *node2){
	comp_tree_t *newnode = tree_make_node(value);
	tree_insert_node(newnode,node1);
	tree_insert_node(newnode,node2);
	return newnode;
}

comp_tree_t* tree_make_ternary_node(void *value, comp_tree_t *node1, comp_tree_t *node2, comp_tree_t *node3){
	comp_tree_t *newnode = tree_make_node(value);
	tree_insert_node(newnode,node1);
	tree_insert_node(newnode,node2);
	tree_insert_node(newnode,node3);
	return newnode;
}

static void print_spaces(int num){
	while (num-->0)
		putc(' ',stdout);
}

static void tree_debug_print_node(comp_tree_t *tree, int spacing){
	if (tree == NULL) return;
	print_spaces(spacing);
	printf("%p(%d): %p\n",tree,tree->childnodes,tree->value);
}

static void tree_debug_print_s(comp_tree_t *tree, int spacing){
	if (tree == NULL) return;

	comp_tree_t *ptr = tree;
	do {
		tree_debug_print_node(ptr,spacing);
		if (ptr->first != NULL)
			tree_debug_print_s(ptr->first,spacing+1);
		ptr = ptr->next;
	} while(ptr != NULL);
}

void tree_debug_print(comp_tree_t *tree){
	tree_debug_print_s(tree,0);
}

// Abstract Sintatic Tree

comp_tree_t* createASTNode(int type, symbol_t *token){
    nodeAST* nodeAST = calloc(1, sizeof(struct nodeAST));

    nodeAST->type = type;
    nodeAST->value_type = 0;
    nodeAST->symbol = token;
    nodeAST->rem_false = NULL;
    nodeAST->rem_true = NULL;
    nodeAST->rem_false_size = 0;
    nodeAST->rem_true_size = 0;

    return tree_make_node(nodeAST);
}

comp_tree_t* createASTUnaryNode(int type, symbol_t* token, comp_tree_t* node){
    comp_tree_t* newnode = createASTNode(type, token);
    tree_insert_node(newnode,node);

    return newnode;
}

comp_tree_t* createASTBinaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2){
    comp_tree_t* newnode = createASTNode(type, token);
    tree_insert_node(newnode,node1);
    tree_insert_node(newnode,node2);

    return newnode;
}

comp_tree_t* createASTTernaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2, comp_tree_t* node3){
    comp_tree_t* newnode = createASTNode(type, token);
    tree_insert_node(newnode,node1);
    tree_insert_node(newnode,node2);
    tree_insert_node(newnode,node3);

    return newnode;
}

comp_tree_t* createASTQuaternaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2, comp_tree_t* node3, comp_tree_t* node4){
    comp_tree_t* newnode = createASTNode(type, token);
    tree_insert_node(newnode,node1);
    tree_insert_node(newnode,node2);
    tree_insert_node(newnode,node3);
    tree_insert_node(newnode,node4);

    return newnode;
}

void connect_all_childs(comp_tree_t* tree){
    comp_tree_t* child = tree->first;
    for (int i = 0; i < tree->childnodes; i++){
        gv_connect(tree, child);
        child = child->next;
    }
}

void build_gv(comp_tree_t* tree){
    comp_tree_t *ptr = tree;
    do {
        nodeAST *node_info = (tree->value);

        if (node_info->symbol != NULL)
            gv_declare(node_info->type, tree, (node_info->symbol)->lexeme);
        else
            gv_declare(node_info->type, tree, NULL);

        if (ptr->first != NULL){
            build_gv(ptr->first);
            connect_all_childs(tree);
        }
        ptr = ptr->next;
        tree = ptr;
    } while(ptr != NULL);
}

void tree_insert_node_as_second_child(comp_tree_t *tree, comp_tree_t *node){
    if (tree == NULL)
    ERRO("Cannot insert node, tree is null");
    if (node == NULL)
    ERRO("Cannot insert node, node is null");

    if (tree->childnodes == 1){
        node->prev = tree->last;
        tree->first->next = node;
        tree->last = node;
    } else if (tree->childnodes > 1){
        node->prev = tree->first;
        node->next = tree->first->next;
        tree->first->next->prev = node;
        tree->first->next = node;
    } else {
        tree->first = node;
        tree->last = node;
    }
    ++tree->childnodes;

    comp_tree_last = tree;
}
/***
 * Insere o nodo da chamada de funcao no lugar correto da arvore.
 * Como esse nodo carrega o pipe, a segunda funcao da expressao "perdeu seu pipe",
 * entao ao inserir o nodo da chamada, tambem insere o pipe antes da segunda funcao.
 *
 * \param func: a AST da primeira funcao de uma exp pipe.
 * \param pipes: a AST de toda a exp pipe sem a primeira funcao e seu pipe
 * \param type: o pipe a ser inserido
 */
comp_tree_t* tree_finish_pipe(comp_tree_t* func, comp_tree_t* pipes, int type){
    comp_tree_t* tree = pipes;
    // procura o nodo que chama a segunda funcao da exp
    if (pipes->value->type != AST_CHAMADA_DE_FUNCAO) {
        tree = pipes->first->first->next;
        while (tree->value->type != AST_CHAMADA_DE_FUNCAO)
            tree = tree->first->first->next;
    }
    //cria o nodo com o pipe a ser inserido
    comp_tree_t* new_pipe = createASTUnaryNode(type, NULL, tree);

    //faz o swap entre o novo pipe e a segunda funcao
    comp_tree_t temp;
    temp = *tree;
    *tree = *new_pipe;
    *new_pipe = temp;

    tree->prev = new_pipe->prev;
    tree->next = new_pipe->next;
    new_pipe->prev = NULL;
    new_pipe->next = NULL;
    tree->first = new_pipe;
    tree->last = new_pipe;

    //insere a primeira funcao da exp como param da segunda funcao
    tree_insert_node_as_second_child(new_pipe, func);
    //retorna a AST inteira
    return pipes;
}

void tree_make_next(comp_tree_t* tree, comp_tree_t* next){
    tree->next = next;
    next->prev = tree;
}

void update_childs(comp_tree_t* tree){
    comp_tree_t* last = NULL;
    comp_tree_t* child = tree->first;
    int n_childs = 0;
    while (child){
        last = child;
        child = child->next;
        n_childs++;
    }
    tree->childnodes = n_childs;
    tree->last = last;
}

char *_description_from_type (int tipo)
{
    switch (tipo){
        case AST_PROGRAMA: return "programa";
        case AST_IF_ELSE: return "ifelse";
        case AST_DO_WHILE: return "dowhile";
        case AST_WHILE_DO: return "whiledo";
        case AST_INPUT: return "input";
        case AST_OUTPUT: return "output";
        case AST_ATRIBUICAO: return "=";
        case AST_RETURN: return "return";
        case AST_BLOCO: return "block";
        case AST_ARIM_SOMA: return "+";
        case AST_ARIM_SUBTRACAO: return "-";
        case AST_ARIM_MULTIPLICACAO: return "*";
        case AST_ARIM_DIVISAO: return "/";
        case AST_ARIM_INVERSAO: return "-";
        case AST_LOGICO_E: return "&&";
        case AST_LOGICO_OU: return "||";
        case AST_LOGICO_COMP_DIF: return "!=";
        case AST_LOGICO_COMP_IGUAL: return "==";
        case AST_LOGICO_COMP_LE: return "<=";
        case AST_LOGICO_COMP_GE: return ">=";
        case AST_LOGICO_COMP_L: return "<";
        case AST_LOGICO_COMP_G: return ">";
        case AST_LOGICO_COMP_NEGACAO: return "!";
        case AST_VETOR_INDEXADO: return "[]";
        case AST_CHAMADA_DE_FUNCAO: return "call";
        case AST_SHIFT_LEFT: return "<<";
        case AST_SHIFT_RIGHT: return ">>";
        case AST_BREAK: return "break";
        case AST_CONTINUE: return "continue";
        case AST_CASE: return "case";
        case AST_FOR: return "for";
        case AST_FOREACH: return "foreach";
        case AST_SWITCH: return "switch";
        case AST_ATRIBUTO: return ".";
        case AST_ARIM_MOD: return "%";
        case AST_ARIM_POT: return "^";
        case AST_ENCADEAMENTO_PIPE: return "%|%";
        case AST_ENCADEAMENTO_PIPEG: return "%>%";
        case AST_IF: return "if";

        default:
            fprintf (stderr, "%s: tipo provided is invalid here\n", __FUNCTION__);
            abort();
    }
    fprintf (stderr, "%s: should not get here\n", __FUNCTION__);
    abort();
}

void build_symbol_rec(comp_tree_t* tree, symbol_t* symbol, int type){

    comp_tree_t *ptr = tree;
    do {
        if (tree->value->symbol) {
            strcat(symbol->lexeme, tree->value->symbol->lexeme);
        }
        else if (ptr->first != NULL){
            build_symbol_rec(ptr->first, symbol, tree->value->type);
        }
        strcat(symbol->lexeme, _description_from_type(type));
        ptr = ptr->next;
        tree = ptr;
    } while(ptr != NULL);
}

void build_symbol(comp_tree_t* tree){

    if (tree->value->symbol)
        return;

    tree->value->symbol = calloc(1, sizeof(symbol_t));
    tree->value->symbol->lexeme = malloc(sizeof(char)*1000);
    tree->value->symbol->line = line_number;

    build_symbol_rec(tree->first, tree->value->symbol, tree->value->type);

}