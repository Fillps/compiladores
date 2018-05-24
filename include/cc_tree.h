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

#ifndef CC_TREE_H_
#define CC_TREE_H_
#define DECL_FUNCTION       2
#define decl_variable(type) DECL_FUNCTION + type            // onde type = POA_LIT_INT, POA_LIT_FLOAT...

#include "cc_misc.h"

typedef struct nodeAST{
    int type;
    int value_type;
    symbol_t *symbol;
}nodeAST;

/*
 * Tipo: comp_tree_t, value é um tipo genérico armazenado em cada
 * nó da árvore, childnodes é a quantidade de nós filhos de um nó,
 * first e last referem-se ao primeiro e ultimo nós filhos,
 * next e prev percorrem uma lista de nós filhos de um mesmo nó.
 * Para uma árvore com 2 filhos, first->next == last e first == last->prev
 */
typedef struct comp_tree {
	nodeAST *value;
	int childnodes;
	struct comp_tree *first,*last;
	struct comp_tree *next,*prev;
} comp_tree_t;

/* Funções: a seguir segue a lista de funções da API cc_tree */

/*
 * Função: tree_new, cria um nó raiz para a árvore. Retorna um
 * ponteiro para o nó criado ou aborta a execução do programa
 * caso algum erro de alocação de memória tenha ocorrido.
 * Equivalente à tree_make_node(NULL).
 */
comp_tree_t* tree_new(void);

/*
 * Função: tree_free, percorre recursivamente a estrutura da
 * árvore e libera todos comp_tree_t alocados, os elementos value
 * devem ser tratados externamente, previamente ao uso desta função.
 */
void tree_free(comp_tree_t *tree);

/*
 * Função: tree_make_node, cria um novo nó e retorna o ponteiro para
 * a estrutura comp_tree_t criada. O paramêtro value é uma estrutura
 * qualquer sob responsabilidade do usuário.
 */
comp_tree_t* tree_make_node(void *value);

/*
 * Função: tree_insert_node, insere node no encadeamento de filhos de
 * tree. Aborta o programa se tree ou node forem nulos.
 */
void tree_insert_node(comp_tree_t *tree, comp_tree_t *node);

/*
 * Função: tree_has_child_nodes, retorna 1 se existir algum nó no
 * encadeamento de nós filhos de tree, 0 caso contrário.
 */
int tree_has_child_nodes(comp_tree_t *tree);

/*
 * Função: tree_make_unary_node, cria um novo nó usando value, e
 * insere node no encadeamento de filhos do novo nó.
 */
comp_tree_t* tree_make_unary_node(void *value, comp_tree_t* node);

/*
 * Função: tree_make_binary_node, cria um novo nó usando value, e
 * insere node1 e node2 no encadeamento de filhos do novo nó.
 */
comp_tree_t* tree_make_binary_node(void *value, comp_tree_t* node1, comp_tree_t* node2);

/*
 * Função: tree_make_ternary_node, cria um novo nó usando value, e
 * insere node1, node2 e node3 no encadeamento de filhos do novo nó.
 */
comp_tree_t* tree_make_ternary_node(void *value, comp_tree_t* node1, comp_tree_t* node2, comp_tree_t* node3);

/*
 * Função: tree_debug_print, percorre a árvore recursivamente e
 * exibe os ponteiros dos atributos value.
 */
void tree_debug_print(comp_tree_t *tree);


comp_tree_t* createASTNode(int type, symbol_t* token);
comp_tree_t* createASTUnaryNode(int type, symbol_t* token, comp_tree_t* node);
comp_tree_t* createASTBinaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2);
comp_tree_t* createASTTernaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2, comp_tree_t* node3);
comp_tree_t* createASTQuaternaryNode(int type, symbol_t* token, comp_tree_t* node1, comp_tree_t* node2, comp_tree_t* node3, comp_tree_t* node4);

void build_gv(comp_tree_t* tree);
void tree_insert_node_as_second_child(comp_tree_t *tree, comp_tree_t *node);

comp_tree_t* tree_finish_pipe(comp_tree_t* func, comp_tree_t* pipes, int type);
void tree_make_next(comp_tree_t* tree, comp_tree_t* next);
void update_childs(comp_tree_t* tree);
void build_symbol(comp_tree_t* tree);

#endif //CC_TREE_H_
