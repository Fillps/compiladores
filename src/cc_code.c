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

iloc_t** get_children_iloc_list(comp_tree_t* tree);
char* get_especial_reg(comp_tree_t* tree);
int ast_to_iloc (int type);
iloc_t* create_iloc_if(iloc_t* code0, iloc_t* code1);
iloc_t* create_iloc_while(iloc_t* code0, iloc_t* code1);
iloc_t* makeFun(symbol_t* funSymbol, iloc_t* code3);
void updateFuncArgs(iloc_t* func, symbol_t* symbol);
iloc_t* append_iloc_list(iloc_t* iloc_list[], int length);

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

void build_iloc_code(comp_tree_t* tree){
    print_iloc_list(invert_iloc_list(code_generator(tree)));
}

iloc_t* create_iloc(int type, char *op1, char *op2, char *op3){
    iloc_t* iloc = (iloc_t*)calloc(1,sizeof(iloc_t));
    iloc->type = type;

    iloc->op1 = op1;
    iloc->op2 = op2;
    iloc->op3 = op3;
    iloc->prev = NULL;
    iloc->next = NULL;

    return iloc;
}

iloc_t* code_generator(comp_tree_t *tree){
    if(!tree) 
        return 0;

    iloc_t** cc = get_children_iloc_list(tree);
    iloc_t* ret;

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
                    tree->value->symbol->lexeme,
                    create_reg());
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
            ret = append_iloc(
                    append_iloc(cc[0], cc[1]),
                    create_iloc(ast_to_iloc(tree->value->type),
                                cc[0] ? cc[0]->op3 : NULL,
                                cc[1] ? cc[1]->op3 : NULL,
                                create_reg()));
            break;
        case AST_ATRIBUICAO:
            ret = append_iloc(
                    cc[1],
                    create_iloc(ILOC_STOREAI,
                                tree->first->value->symbol->lexeme,
                                cc[1] ? cc[1]->op3 : NULL,
                                get_especial_reg(tree->first)));
            break;
        default:
            ret = append_iloc_list(cc, tree->childnodes);
    }

    free(cc);
    return ret;
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
        return "fp";
}

int ast_to_iloc (int type)
{
    switch (type){
        case AST_ARIM_SOMA: return ILOC_ADD;
        case AST_ARIM_MULTIPLICACAO: return ILOC_MULT;
        case AST_ARIM_SUBTRACAO: return ILOC_SUB;
        case AST_ARIM_DIVISAO: return ILOC_DIV;
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
        case ILOC_NOP: return "NOP";
        case ILOC_ADD: return "ADD";
        case ILOC_SUB: return "SUB";
        case ILOC_MULT: return "MULT";
        case ILOC_DIV: return "DIV";
        case ILOC_ADDI: return "ADDI";
        case ILOC_SUBI: return "SUBI";
        case ILOC_RSUBI: return "RSUBI";
        case ILOC_MULTI: return "MULTI";
        case ILOC_DIVI: return "DIVI";
        case ILOC_RDIVI: return "RDIVI";
        case ILOC_LSHIFT: return "LSHIFT";
        case ILOC_LSHIFTI: return "LSHIFTI";
        case ILOC_RSHIFT: return "RSHIFT";
        case ILOC_RSHIFTI: return "RSHIFTI";
        case ILOC_AND: return "AND";
        case ILOC_ANDI: return "ANDI";
        case ILOC_OR: return "OR";
        case ILOC_ORI: return "ORI";
        case ILOC_XOR: return "XOR";
        case ILOC_XORI: return "XORI";
        case ILOC_LOADI: return "LOADI";
        case ILOC_LOAD: return "LOAD";
        case ILOC_LOADAI: return "LOADAI";
        case ILOC_LOADA0: return "LOADA0";
        case ILOC_CLOAD: return "CLOAD";
        case ILOC_CLOADAI: return "CLOADAI";
        case ILOC_CLOADA0: return "CLOADA0";
        case ILOC_STORE: return "STORE";
        case ILOC_STOREAI: return "STOREAI";
        case ILOC_STOREA0: return "STOREA0";
        case ILOC_CSTORE: return "CSTORE";
        case ILOC_CSTOREAI: return "CSTOREAI";
        case ILOC_CSTOREA0: return "CSTOREA0";
        case ILOC_I2I: return "I2I";
        case ILOC_C2I: return "C2I";
        case ILOC_C2C: return "C2C";
        case ILOC_I2C: return "I2C";
        case ILOC_JUMPI: return "JUMPI";
        case ILOC_JUMP: return "JUMP";
        case ILOC_CBR: return "CBR";
        case ILOC_CMP_LT: return "CMP_LT";
        case ILOC_CMP_LE: return "CMP_LE";
        case ILOC_CMP_EQ: return "CMP_EQ";
        case ILOC_CMP_GE: return "CMP_GE";
        case ILOC_CMP_GT: return "CMP_GT";
        case ILOC_CMP_NE: return "CMP_NE";

        default:
            fprintf (stderr, "%s: type provided is invalid here\n", __FUNCTION__);
            abort();
    }
}

void print_iloc(iloc_t* iloc){
    if(!iloc)
        return;

    fprintf(cfp, "%s ", __iloc_instructions(iloc->type));

    if(iloc->op1) fprintf(cfp, "%s ", iloc->op1);
    if(iloc->op2) fprintf(cfp, "%s ", iloc->op2);
    if(iloc->op3) fprintf(cfp, "%s ", iloc->op3);

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
    for(iloc_t* i = iloc; iloc != NULL; iloc = iloc->next)
        print_iloc(i);
}

