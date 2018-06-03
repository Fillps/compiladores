/*
Grupo Epsilon:
  - Douglas Flores
  - Filipe Santos
*/

#include <stdio.h>
#include <stdlib.h>
#include <cc_semantic.h>

#include "cc_code.h"
#include "cc_misc.h"
#include "cc_ast.h"
#include "cc_tree.h"

static FILE *cfp = NULL;
iloc_t** iloc_list;
int iloc_list_length;

iloc_t** get_children_iloc_list(comp_tree_t* tree);
char* get_especial_reg(comp_tree_t* tree);
int ast_to_iloc (int type);
iloc_t* create_iloc_if(iloc_t* code0, iloc_t* code1);
iloc_t* create_iloc_while(iloc_t* code0, iloc_t* code1);
iloc_t* makeFun(symbol_t* funSymbol, iloc_t* code3);
void updateFuncArgs(iloc_t* func, symbol_t* symbol);
iloc_t* append_iloc_list(iloc_t* iloc_list[], int length);
void free_iloc_list();
static inline char *__iloc_instructions (int type);
char* get_char_address(comp_tree_t *tree);
iloc_t* vetor_indexando_iloc(comp_tree_t *tree, iloc_t **cc);
iloc_t* atribuicao_iloc(comp_tree_t* tree, iloc_t** cc);


void code_init(const char *filename)
{
    //verificar se code_init já foi chamada
    if (cfp){
        fprintf (stderr, "%s:%d já foi chamada, abort()\n", __FUNCTION__, __LINE__);
        abort();
    }

    //se o nome do arquivo for válido, abre arquivo com esse nome para escrita
    if (filename){
        cfp = fopen (filename, "w");
        if (!cfp){
            fprintf (stderr, "%s:%d não conseguiu abrir o arquivo %s para escrita\n", __FUNCTION__, __LINE__, filename);
            abort();
        }
    }else{
        cfp = stdout;
    }
}

void code_close(){
    if (cfp){
        fclose(cfp);
    }
    else{
        fprintf (stderr, "%s:%i foi chamada, mas code_init não foi chamada antes, abort()\n", __FUNCTION__, __LINE__);
        abort();
    }
}

#define MAX_ILOC 10000

void build_iloc_code(comp_tree_t* tree){
    iloc_list = calloc(MAX_ILOC, sizeof(iloc_t*));
    iloc_list_length = 0;
    print_iloc_list(invert_iloc_list(code_generator(tree)));
    free_iloc_list();
}

iloc_t* create_iloc(int type, char *op1, char *op2, char *op3){
    iloc_t* iloc = (iloc_t*)calloc(1,sizeof(iloc_t));
    iloc->type = type;

    iloc->op1 = op1;
    iloc->op2 = op2;
    iloc->op3 = op3;
    iloc->prev = NULL;
    iloc->next = NULL;

    iloc_list[iloc_list_length++] = iloc;

    return iloc;
}

iloc_t* code_generator(comp_tree_t *tree){
    if(!tree) 
        return 0;

    iloc_t** cc = get_children_iloc_list(tree);
    iloc_t *ret, *aux1, *aux2;

    switch(tree->value->type){
        case AST_LITERAL:
            ret = create_iloc(
                    ILOC_LOADI,
                    tree->value->symbol->lexeme,
                    NULL,
                    create_reg());
            break;
        case AST_IDENTIFICADOR:
            ret = create_iloc(
                    ILOC_LOADAI,
                    get_especial_reg(tree),
                    get_char_address(tree),
                    create_reg());
            break;
        case AST_VETOR_INDEXADO:
            ret = vetor_indexando_iloc(tree, cc);
            break;
        case AST_ARIM_INVERSAO:
            ret = append_iloc(
                    cc[0], create_iloc(
                            ILOC_RSUBI,
                            cc[0]->op3,
                            "0",
                            create_reg()));
            break;
        case AST_ARIM_SOMA:
        case AST_ARIM_MULTIPLICACAO:
        case AST_ARIM_SUBTRACAO:
        case AST_ARIM_DIVISAO:
        case AST_LOGICO_OU:
        case AST_LOGICO_E:
        case AST_LOGICO_COMP_L:
        case AST_LOGICO_COMP_G:
        case AST_LOGICO_COMP_DIF:
        case AST_LOGICO_COMP_LE:
        case AST_LOGICO_COMP_GE:
        case AST_LOGICO_COMP_IGUAL:
            ret = append_iloc(append_iloc(
                    cc[0], cc[1]), create_iloc(ast_to_iloc(tree->value->type),
                                                cc[0] ? cc[0]->op3 : NULL,
                                                cc[1] ? cc[1]->op3 : NULL,
                                                create_reg()));
            break;
        case AST_SHIFT_LEFT:
        case AST_SHIFT_RIGHT:
            aux1 = create_iloc(ast_to_iloc(tree->value->type),
                               cc[0] ? cc[0]->op3 : NULL,
                               tree->first->next->value->symbol->lexeme,
                               create_reg());
            aux2 = create_iloc(ILOC_STOREAI,
                               aux1->op3,
                               get_especial_reg(tree->first),
                               get_char_address(tree->first));
            ret = append_iloc(append_iloc(append_iloc(
                    cc[0], aux1), aux2), cc[2]);
            break;
        case AST_ATRIBUICAO:
            ret = atribuicao_iloc(tree, cc);
            break;
        default:
            ret = append_iloc_list(cc, tree->childnodes);
    }

    free(cc);
    return ret;
}

iloc_t* vetor_indexando_iloc(comp_tree_t *tree, iloc_t **cc){

    iloc_t *multI , *addI, *loadAO;

    multI = create_iloc(
            ILOC_MULTI,
            cc[1]->op3,
            INT_SIZE_c,
            create_reg());

    addI = create_iloc(
            ILOC_ADDI,
            multI->op3,
            get_char_address(tree->first),
            create_reg());

    loadAO = create_iloc(
            ILOC_LOADAO,
            get_especial_reg(tree),
            addI->op3,
            create_reg());

    return append_iloc(append_iloc(append_iloc(
            cc[1], multI), addI), loadAO);
}

iloc_t* atribuicao_iloc(comp_tree_t* tree, iloc_t** cc){
    int op;
    iloc_t *address_iloc, *store;
    char* address;
    if (tree->first->value->type == AST_VETOR_INDEXADO){
        cc[0] = cc[0]->prev; // o ultimo iloc de um vetor indexado é o LOADAO, sendo o penultimo o calculo do ender
        cc[0]->next = NULL;  // remove o LOADAO deixando o calculo do ender como o ultimo iloc
        address_iloc = cc[0];
        address = address_iloc->op3;
        op = ILOC_STOREAO;
    }
    else{
        address_iloc = NULL;
        address = get_char_address(tree->first);
        op = ILOC_STOREAI;
    }

    store = create_iloc(
            op,
            cc[1] ? cc[1]->op3 : NULL,
            get_especial_reg(tree->first),
            address);

    return append_iloc(append_iloc(append_iloc(
            address_iloc, cc[1]), store), cc[2]);
}

iloc_t** get_children_iloc_list(comp_tree_t* tree){
    iloc_t** cc = calloc(tree->childnodes, sizeof(iloc_t*));

    comp_tree_t* aux_tree = tree->first;
    for (int i = 0; i < tree->childnodes; i++){
        cc[i] = code_generator(aux_tree);
        aux_tree = aux_tree->next;
    }

    return cc;
}

char* get_especial_reg(comp_tree_t* tree){
    if (tree->value->var_scope == GLOBAL_SCOPE)
        return "rbss";
    else
        return "rarp";
}

int ast_to_iloc (int type)
{
    switch (type){
        case AST_ARIM_SOMA: return ILOC_ADD;
        case AST_ARIM_MULTIPLICACAO: return ILOC_MULT;
        case AST_ARIM_SUBTRACAO: return ILOC_SUB;
        case AST_ARIM_DIVISAO: return ILOC_DIV;
        case AST_SHIFT_LEFT: return ILOC_LSHIFTI;
        case AST_SHIFT_RIGHT: return ILOC_RSHIFTI;
        case AST_LOGICO_OU: return ILOC_OR;
        case AST_LOGICO_E: return ILOC_AND;
        case AST_LOGICO_COMP_L: return ILOC_CMP_LT;
        case AST_LOGICO_COMP_G: return ILOC_CMP_GT;
        case AST_LOGICO_COMP_DIF: return ILOC_CMP_NE;
        case AST_LOGICO_COMP_LE: return ILOC_CMP_LE;
        case AST_LOGICO_COMP_GE: return ILOC_CMP_GE;
        case AST_LOGICO_COMP_IGUAL: return ILOC_CMP_EQ;

        default:
            fprintf (stderr, "%s: type provided is invalid here\n", __FUNCTION__);
            abort();
    }
}

iloc_t* append_iloc_list(iloc_t* iloc_list[], int length){
    iloc_t* iloc = iloc_list[0];
    for (int i = 1; i < length; i++)
        iloc = append_iloc(iloc, iloc_list[i]);

    return iloc;
}

iloc_t* append_iloc(iloc_t* iloc1, iloc_t* iloc2){
    if(!iloc1)
        return iloc2;
    else if(!iloc2)
        return iloc1;

    iloc_t* iloc = iloc2;

    while(iloc->prev)
        iloc = iloc->prev;

    iloc->prev = iloc1;
    return iloc2;
}

static inline char *__iloc_instructions (int type)
{
    switch (type){
        case ILOC_NOP: return "nop";
        case ILOC_ADD: return "add";
        case ILOC_SUB: return "sub";
        case ILOC_MULT: return "mult";
        case ILOC_DIV: return "div";
        case ILOC_ADDI: return "addI";
        case ILOC_SUBI: return "subI";
        case ILOC_RSUBI: return "rsubI";
        case ILOC_MULTI: return "multI";
        case ILOC_DIVI: return "divI";
        case ILOC_RDIVI: return "rdivI";
        case ILOC_LSHIFT: return "lshift";
        case ILOC_LSHIFTI: return "lshiftI";
        case ILOC_RSHIFT: return "rshift";
        case ILOC_RSHIFTI: return "rshiftI";
        case ILOC_AND: return "and";
        case ILOC_ANDI: return "andI";
        case ILOC_OR: return "or";
        case ILOC_ORI: return "orI";
        case ILOC_XOR: return "xor";
        case ILOC_XORI: return "xorI";
        case ILOC_LOADI: return "loadI";
        case ILOC_LOAD: return "load";
        case ILOC_LOADAI: return "loadAI";
        case ILOC_LOADAO: return "loadAO";
        case ILOC_CLOAD: return "cload";
        case ILOC_CLOADAI: return "cloadAI";
        case ILOC_CLOADAO: return "cloadAO";
        case ILOC_STORE: return "store";
        case ILOC_STOREAI: return "storeAI";
        case ILOC_STOREAO: return "storeAO";
        case ILOC_CSTORE: return "cstore";
        case ILOC_CSTOREAI: return "cstoreAI";
        case ILOC_CSTOREAO: return "cstoreAO";
        case ILOC_I2I: return "i2i";
        case ILOC_C2I: return "c2i";
        case ILOC_C2C: return "c2c";
        case ILOC_I2C: return "i2c";
        case ILOC_JUMPI: return "jumpI";
        case ILOC_JUMP: return "jump";
        case ILOC_CBR: return "cbr";
        case ILOC_CMP_LT: return "cmp_LT";
        case ILOC_CMP_LE: return "cmp_LE";
        case ILOC_CMP_EQ: return "cmp_EQ";
        case ILOC_CMP_GE: return "cmp_GE";
        case ILOC_CMP_GT: return "cmp_GT";
        case ILOC_CMP_NE: return "cmp_NE";

        default:
            fprintf (stderr, "%s: type provided is invalid here\n", __FUNCTION__);
            abort();
    }
}

void print_iloc(iloc_t* iloc){
    if(!iloc)
        return;

    fprintf(cfp, "%s ", __iloc_instructions(iloc->type));

    switch (iloc->type){
        case ILOC_STORE:
        case ILOC_STOREAI:
        case ILOC_STOREAO:
            if(iloc->op1) fprintf(cfp, "%s => ", iloc->op1);
            if(iloc->op2) fprintf(cfp, "%s, ", iloc->op2);
            if(iloc->op3) fprintf(cfp, "%s ", iloc->op3);
            break;
        default:
            if(iloc->op1) fprintf(cfp, "%s", iloc->op1);
            if(iloc->op2) fprintf(cfp, ", %s", iloc->op2);
            if(iloc->op3) fprintf(cfp, " => %s ", iloc->op3);
    }

    fprintf(cfp, "\n");
}

iloc_t* invert_iloc_list(iloc_t* last){
    iloc_t* aux_iloc, *curr_iloc;

    if(!last) return last;

    aux_iloc = last;
    for(curr_iloc = last->prev; curr_iloc; curr_iloc = curr_iloc->prev){
        curr_iloc->next = aux_iloc;
        aux_iloc = aux_iloc->prev;
    }

    return aux_iloc;
}

void print_iloc_list(iloc_t* iloc){
    for(iloc_t* i = iloc; i != NULL; i = i->next)
        print_iloc(i);
}

void free_iloc_list(){
    for(int i = 0; i < iloc_list_length; i++)
        free(iloc_list[i]);
}

char* get_char_address(comp_tree_t *tree){
    char* address = malloc(20*sizeof(char));
    sprintf(address, "%i", tree->value->address);
    return address;
}