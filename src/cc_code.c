/*
Grupo Epsilon:
  - Douglas Flores
  - Filipe Santos
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cc_code.h"
#include "cc_misc.h"
#include "cc_ast.h"
#include "cc_tree.h"
#include "cc_semantic.h"
#include "cc_dict.h"

static FILE *cfp = NULL;
iloc_t** iloc_list;
int iloc_list_length;
char* main_label = NULL;
int main_scope;
comp_dict_t* function_labels;
char** reg_list;
int reg_list_length;
int var_scope_size;
comp_dict_t* function_registers_dict;

iloc_t** get_children_iloc_list(comp_tree_t* tree);
char* get_especial_reg(comp_tree_t* tree);
int ast_to_iloc (int type);
iloc_t* append_iloc_list(iloc_t* iloc_list[], int length);
void free_iloc_list();
static inline char *__iloc_instructions (int type);
char* get_char_address(comp_tree_t *tree);
iloc_t* vetor_indexando_iloc(comp_tree_t *tree, iloc_t **cc);
iloc_t* atribuicao_iloc(comp_tree_t* tree);
iloc_t* get_last_iloc(iloc_t* iloc);
char* get_literal_value(comp_tree_t *literal);
void add_rem_false(comp_tree_t *tree, char **remendo);
void add_rem_true(comp_tree_t *tree, char **remendo);
void patchup_true(comp_tree_t* tree, char *label);
void patchup_false(comp_tree_t* tree, char *label);
void concat_true(comp_tree_t* tree);
void concat_false(comp_tree_t* tree);
void short_circuit_literal(comp_tree_t* tree, iloc_t **iloc);
void short_circuit_variable(comp_tree_t *tree, iloc_t **iloc);
iloc_t *foreach_iloc(comp_tree_t *tree, iloc_t **cc);
iloc_t* call_sequence(comp_tree_t* tree);
iloc_t* call_epilogue(comp_tree_t* tree);
iloc_t* return_sequence(comp_tree_t* tree);
iloc_t* update_reg(char* reg, int op, int value);
int get_number_of_instructions(iloc_t* iloc);
char* create_reg();

void code_init(const char * filename){
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
    //Verifica se o arquivo de código foi aberto antes
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
    function_labels = dict_new();
    function_registers_dict = dict_new();
    reg_list = calloc(DICT_SIZE, sizeof(char));
    reg_list_length = 0;
    iloc_list = calloc(MAX_ILOC, sizeof(iloc_t*));
    iloc_list_length = 0;
    iloc_t *code;
    char rbss_address[10];
    code = code_generator(tree);    // Gera o código

    iloc_t* init_rarp = create_iloc(ILOC_LOADI,"1024", NULL, "rarp");
    iloc_t* init_rsp = create_iloc(ILOC_LOADI,"1024", NULL, "rsp");
    iloc_t* init_rbss = create_iloc(ILOC_LOADI, rbss_address, NULL, "rbss");
    iloc_t* jmp_main;

    if(main_label == NULL){
        main_label = "5";
        fprintf(stderr, "WARNING: main() function not found!\n");
    }
    jmp_main = create_iloc(ILOC_JUMPI, NULL, NULL, main_label);

    code =  append_iloc(
                init_rarp, append_iloc(
                    init_rsp, append_iloc(
                        init_rbss, append_iloc(
                            jmp_main, code))));

    sprintf(rbss_address, "%i", get_number_of_instructions(code));  // Converte o número de instruções para string

    print_iloc_list(invert_iloc_list(code));

    free_iloc_list();
    for (int i = 0; i < function_labels->size; i++)
        if (function_labels->data[i])
            dict_remove(function_labels, (function_labels->data[i])->key);
    dict_free(function_labels);
    for (int i = 0; i < function_registers_dict->size; i++)
        if (function_registers_dict->data[i])
            dict_remove(function_registers_dict, (function_registers_dict->data[i])->key);
    dict_free(function_registers_dict);
    free(reg_list);

}

iloc_t* create_iloc(int type, char *op1, char *op2, char *op3){
    iloc_t* iloc = (iloc_t*)calloc(1,sizeof(iloc_t));
    iloc->type = type;

    iloc->label = NULL;
    iloc->op1 = op1;
    iloc->op2 = op2;
    iloc->op3 = op3;
    iloc->prev = NULL;
    iloc->next = NULL;
    iloc->comment = NULL;

    iloc_list[iloc_list_length++] = iloc;

    return iloc;
}

iloc_t* code_generator(comp_tree_t *tree){
    if(!tree)
        return 0;
    if (tree->value->type == AST_FUNCAO_MAIN || tree->value->type == AST_FUNCAO){
        id_value_t* id_value = (id_value_t*)tree->value->symbol->value;
        var_scope_size = ((function_info_t*)id_value->decl_info[GLOBAL_SCOPE])->local_var_size*INT_SIZE;
        reg_list_length = 0;
    }

    iloc_t** cc = get_children_iloc_list(tree);
    iloc_t *ret = NULL, *aux1 = NULL, *aux2 = NULL, *aux3 = NULL;
    int func_reg_list_length;

    switch(tree->value->type){
        case AST_LITERAL:
            ret = create_iloc(
                    ILOC_LOADI,
                    get_literal_value(tree),
                    NULL,
                    create_reg());
            break;
        case AST_IDENTIFICADOR:
        case AST_ATRIBUTO:
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
            if (tree->first->value->type == AST_LITERAL)
                ret = create_iloc(
                        ILOC_LOADI,
                        insert_minus_in_str(get_literal_value(tree->first)),
                        NULL,
                        create_reg());
            else
                ret = append_iloc(
                        cc[0], create_iloc(ILOC_RSUBI, cc[0]->op3, "0", create_reg()));
            break;
        case AST_ARIM_SOMA:
        case AST_ARIM_MULTIPLICACAO:
        case AST_ARIM_SUBTRACAO:
        case AST_ARIM_DIVISAO:
            ret = append_iloc(append_iloc(cc[0], cc[1]),
                              create_iloc(ast_to_iloc(tree->value->type),
                                          cc[0] ? cc[0]->op3 : NULL,    //Operando 1
                                          cc[1] ? cc[1]->op3 : NULL,    //Operando 2
                                          create_reg()));               //Operando 3 - onde guarda o resultado
            break;
        case AST_LOGICO_COMP_L:
        case AST_LOGICO_COMP_G:
        case AST_LOGICO_COMP_DIF:
        case AST_LOGICO_COMP_LE:
        case AST_LOGICO_COMP_GE:
        case AST_LOGICO_COMP_IGUAL:
            aux1 = create_iloc(ast_to_iloc(tree->value->type),
                                           cc[0] ? cc[0]->op3 : NULL,
                                           cc[1] ? cc[1]->op3 : NULL,
                                           create_reg());

            aux2 = create_iloc(ILOC_CBR, aux1->op3, NULL, NULL);
            //Cria os remendos
            add_rem_true(tree, &aux2->op2);
            add_rem_false(tree, &aux2->op3);

            ret = append_iloc(append_iloc(append_iloc(cc[0], cc[1]),
                                          aux1), aux2), cc[2];
            break;
        case AST_LOGICO_COMP_NEGACAO:
            short_circuit_literal(tree->first, &cc[0]);
            short_circuit_variable(tree->first, &cc[0]);

            tree->value->rem_true = tree->first->value->rem_false;
            tree->value->rem_true_size = tree->first->value->rem_false_size;
            tree->value->rem_false = tree->first->value->rem_true;
            tree->value->rem_false_size = tree->first->value->rem_true_size;

            ret = cc[0];

            break;
        case AST_LOGICO_OU:
            short_circuit_literal(tree->first, &cc[0]);
            short_circuit_literal(tree->last, &cc[1]);
            short_circuit_variable(tree->first, &cc[0]);
            short_circuit_variable(tree->last, &cc[1]);

            aux1 = get_last_iloc(cc[1]);
            aux1->label = create_label();

            patchup_false(tree->first, aux1->label);

            concat_true(tree);
            tree->value->rem_false = tree->last->value->rem_false;
            tree->value->rem_false_size = tree->last->value->rem_false_size;

            ret = append_iloc(cc[0], cc[1]);
            break;
        case AST_LOGICO_E:
            short_circuit_literal(tree->first, &cc[0]);
            short_circuit_literal(tree->last, &cc[1]);
            short_circuit_variable(tree->first, &cc[0]);
            short_circuit_variable(tree->last, &cc[1]);

            aux1 = get_last_iloc(cc[1]);
            aux1->label = create_label();

            patchup_true(tree->first, aux1->label);

            concat_false(tree);
            tree->value->rem_true = tree->last->value->rem_true;
            tree->value->rem_true_size = tree->last->value->rem_true_size;

            ret = append_iloc(cc[0], cc[1]);
            break;
        case AST_SHIFT_LEFT:
        case AST_SHIFT_RIGHT:
            aux1 = create_iloc(ast_to_iloc(tree->value->type),
                               cc[0] ? cc[0]->op3 : NULL,
                               get_literal_value(tree->first->next),
                               create_reg());
            aux2 = create_iloc(ILOC_STOREAI,
                               aux1->op3,
                               get_especial_reg(tree->first),
                               get_char_address(tree->first));
            ret = append_iloc(append_iloc(append_iloc(
                    cc[0], aux1), aux2), cc[2]);
            break;
        case AST_ATRIBUICAO:
            ret = atribuicao_iloc(tree);
            break;
        case AST_IF_ELSE:
            cc[1] = append_iloc(
                    cc[1], create_iloc(ILOC_JUMPI, NULL, NULL, create_label()));
            if (!cc[3]) // nao existe prox comando
                cc[3] = create_iloc(ILOC_NOP, NULL, NULL, NULL);
            get_last_iloc(cc[3])->label = cc[1]->op3;
            aux2 = cc[3];
        case AST_IF:
            get_last_iloc(cc[0])->comment = "IF_EXP";
            get_last_iloc(cc[1])->comment = "IF_TRUE";

            aux1 = get_last_iloc(cc[1]);
            aux1->label = create_label();
            patchup_true(tree->first, aux1->label);
            if (!cc[2]) // Não existe próximo comando
                cc[2] = create_iloc(ILOC_NOP, NULL, NULL, NULL);
            aux3 = get_last_iloc(cc[2]);
            aux3->label = create_label();
            patchup_false(tree->first, aux3->label);

            ret = append_iloc(append_iloc(append_iloc(
                    cc[0], cc[1]), cc[2]), aux2);
            break;
        case AST_WHILE_DO:
            // adiciona um jump ate a exp
            aux3 = create_iloc(ILOC_JUMPI, NULL, NULL, create_label());
            get_last_iloc(cc[1])->label = aux3->op3;
        case AST_DO_WHILE:
            aux1 = get_last_iloc(cc[0]);
            aux1->label = create_label();
            patchup_true(tree->first->next, aux1->label);

            if (!cc[2]) // nao existe prox comando
                cc[2] = create_iloc(ILOC_NOP, NULL, NULL, NULL); //cria um nop para colocar o label
            aux2 = get_last_iloc(cc[2]);
            aux2->label = create_label();
            patchup_false(tree->first->next, aux2->label);

            ret = append_iloc(append_iloc(append_iloc(
                    aux3, cc[0]), cc[1]), cc[2]);
            break;
        case AST_FOR:
            // adiciona um JUMP para a exp
            aux1 = create_iloc(ILOC_JUMPI, NULL, NULL, create_label());
            get_last_iloc(cc[1])->label = aux1->op3;

            if (!cc[4]) // nao existe prox comando
                cc[4] = create_iloc(ILOC_NOP, NULL, NULL, NULL);

            aux2 = get_last_iloc(cc[3]);
            aux2->label = create_label(); // se true, vai para o inicio do bloco
            patchup_true(tree->first->next, aux2->label);

            aux3 = get_last_iloc(cc[4]);
            aux3->label = create_label(); // se false, vai para prox comando
            patchup_false(tree->first->next, aux3->label);

            // lista_comando1 -> JUMP exp -> bloco -> lista_comando2 -> exp bloco prox comando -> prox comando
            ret = append_iloc(append_iloc(append_iloc(append_iloc(append_iloc(
                    cc[0], aux1), cc[3]), cc[2]), cc[1]), cc[4]);
            break;
        case AST_FOREACH:
            ret = foreach_iloc(tree, cc);
            break;
        case AST_EXP_LIST:
            break;
        case AST_CHAMADA_DE_FUNCAO:
            ret = call_sequence(tree);
            break;
        case AST_FUNCAO_MAIN:
            ret = append_iloc_list(cc, tree->childnodes);

            // reserva espaço para as variáveis locais
            id_value_t* value = tree->value->symbol->value;
            function_info_t* func = value->decl_info[0];

            char* size_vars = malloc(20*sizeof(char));
            sprintf(size_vars, "%d", func->local_var_size);
            add_to_tmp_list(size_vars);

            iloc_t* reserve = create_iloc(ILOC_ADDI, "rsp", size_vars, "rsp");
            reserve->comment = "MAIN_FUNC";
            ret = append_iloc(reserve, ret);

            // como a ultima instrucao do da funcao main vai se um return,
            // sendo que o iloc do return_sequence sempre possui 7 ilocs,
            // substitui esses 6 ilocs por um halt
            ret = ret->prev->prev->prev->prev->prev->prev; //volta 6 ilocs
            ret->type = ILOC_HALT; //substitui por um halt
            ret->op1 = NULL;
            ret->op2 = NULL;
            ret->op3 = NULL;

            // Se não tem label, cria
            if(value->label[tree->value->var_scope] == NULL){
                get_last_iloc(ret)->label = create_label();
                value->label[tree->value->var_scope] = ret->label;
            }else
                get_last_iloc(ret)->label = value->label[tree->value->var_scope];

            main_label = get_last_iloc(ret)->label;
            break;
        case AST_FUNCAO:
            func_reg_list_length = reg_list_length;
            dict_put(function_registers_dict, tree->value->symbol->lexeme, &func_reg_list_length);

            ret = append_iloc_list(cc, tree->childnodes);
            // Constrói o epílogo da chamada da função
            get_last_iloc(ret)->comment = "FUNC";

            id_value_t* func_value = tree->value->symbol->value;
            // Se não tem label, cria
            if(func_value->label[tree->value->var_scope] == NULL){
                get_last_iloc(ret)->label = create_label();
                func_value->label[tree->value->var_scope] = ret->label;
            }else
                get_last_iloc(ret)->label = func_value->label[tree->value->var_scope];

            break;
        case AST_RETURN:
            ret = append_iloc_list(cc, tree->childnodes);
            get_last_iloc(ret)->comment = "RETURN_VALUE";
            ret->comment = "END_RETURN_VALUE";

            ret = append_iloc(ret, return_sequence(tree));

            break;
        default:
            ret = append_iloc_list(cc, tree->childnodes);
    }

    tree->value->iloc = ret;
    free(cc);
    return ret;
}

iloc_t* call_sequence(comp_tree_t* tree){
    iloc_t* seq;
    id_value_t* v = tree->first->value->symbol->value;
    function_info_t* func_info = v->decl_info[0];

    //calc do espaço necessário para os parametros
    int i = 0;
    int params_size = 0;
    while(func_info->param_type[i]){
        params_size += size_of(func_info->param_type[i]);
        i++;
    }

    // Guarda sp
    iloc_t* store_rsp = create_iloc(ILOC_STOREAI, "rsp", "rsp", "4");

    // Guarda rarp
    iloc_t* store_rarp = create_iloc(ILOC_STOREAI, "rarp", "rsp", "8");

    //empilhamento de parâmetros
    iloc_t* store_param = NULL;
    comp_tree_t* param_tree = tree->first;
    int top = 16;
    char* param_address = malloc(240*sizeof(char));
    add_to_tmp_list(param_address);
    for(i = 0; i < tree->childnodes-1; i++){
        param_tree = param_tree->next;
        int store_type = ILOC_STOREAI;
        sprintf(param_address+12*i, "%i", top);

        store_param = append_iloc(store_param, param_tree->value->iloc);
        store_param = append_iloc(store_param,
          create_iloc(store_type, store_param->op3, "rsp", param_address+12*i));
        store_param->comment = "STORE_PARAM";

        top += 4;//size_of(param_tree->value->value_type);
    }

    // salva e restaura os registradores
    iloc_t *store_registers = NULL, *restore_registers = NULL;;
    for(i = 0; i < reg_list_length; i++){
        char* endr = malloc(20*sizeof(char));
        add_to_tmp_list(endr);
        sprintf(endr, "%i", RA_SIZE + var_scope_size + i*4);
        store_registers = append_iloc(store_registers, create_iloc(ILOC_STOREAI, reg_list[i], "rsp", endr));
        restore_registers = append_iloc(restore_registers, create_iloc(ILOC_LOADAI, "rsp", endr, reg_list[i]));
    }
    if (store_registers){
        store_registers->comment = "STORE_REG";
        restore_registers->comment = "RESTORE_REG";
    }

    //calc do endereço de retorno
    char* reg_address = create_reg();
    iloc_t* address_calc = create_iloc(ILOC_ADDI, "rpc", "5", reg_address);
    address_calc->comment = "FUNC_CALL";

    // Guarda endereço de retorno
    seq = create_iloc(ILOC_STOREAI, reg_address, "rsp", "0");

    // Jump para a funcao
    id_value_t* value = tree->value->symbol->value;
    value->label[tree->value->var_scope] = (char*)dict_get(function_labels, tree->first->value->symbol->lexeme);
    if(value->label[tree->value->var_scope] == NULL){
        value->label[tree->value->var_scope] = create_label();
        dict_put(function_labels, tree->first->value->symbol->lexeme, value->label[tree->value->var_scope]);
    }
    iloc_t* jmp = create_iloc(ILOC_JUMPI, NULL, NULL, value->label[tree->value->var_scope]);

    // carrega o resultado da função
    iloc_t* load_result = create_iloc(ILOC_LOADAI, "rsp", "12", create_reg());
    load_result->comment = "LOAD_RESULT";

    seq = append_iloc(append_iloc(append_iloc(append_iloc(append_iloc(append_iloc(append_iloc(append_iloc(append_iloc(
              store_registers, store_rsp), store_rarp), store_param), address_calc),
              seq), call_epilogue(tree->first)), jmp), restore_registers), load_result);

    return seq;
}

iloc_t* call_epilogue(comp_tree_t* tree){
    iloc_t* call;

    // Move o rarp para a mesma posição do rsp
    call = create_iloc(ILOC_I2I, "rsp", NULL, "rarp");
    call->comment = "EPILOGUE";

    // Reserva espaço para as vars locais
    id_value_t* value = tree->value->symbol->value;
    function_info_t* func_info = value->decl_info[GLOBAL_SCOPE];

    //reg number
    int *reg_number = (int*)dict_get(function_registers_dict, tree->value->symbol->lexeme);
    if (!reg_number){
        reg_number = malloc(sizeof(int));
        *reg_number = reg_list_length;
        add_to_tmp_list(reg_number);
    }

    // Atualiza o rsp
    iloc_t* update_rsp = update_reg("rsp", REG_INC, func_info->local_var_size*INT_SIZE + RA_SIZE + *reg_number*INT_SIZE);

    call = append_iloc(call, update_rsp);

    return call;
}

iloc_t* return_sequence(comp_tree_t* tree){
    iloc_t* return_code;
    char* reg = create_reg();
    iloc_t *aux1, *aux2, *aux3, *aux4, *aux5, *aux6;
    aux1 = tree->last->value->iloc;

    return_code = create_iloc(ILOC_STOREAI, aux1->op3, "rarp", "12");

    // Carrega o endereço, o rsp e o rarp
    reg = create_reg();
    char* rA = create_reg();
    char* rB = create_reg();
    aux1 = create_iloc(ILOC_LOADAI, "rarp", "0", reg);
    aux2 = create_iloc(ILOC_LOADAI, "rarp", "4", rA);
    aux3 = create_iloc(ILOC_LOADAI, "rarp", "8", rB);
    aux4 = create_iloc(ILOC_I2I, rA, NULL, "rsp");
    aux5 = create_iloc(ILOC_I2I, rB, NULL, "rarp");
    aux6 = create_iloc(ILOC_JUMP, NULL, NULL, reg);

    return_code = append_iloc(
                    return_code, append_iloc(
                      aux1, append_iloc(
                        aux2, append_iloc(
                          aux3, append_iloc(
                            aux4, append_iloc(
                              aux5, aux6))))));

    return return_code;
}

iloc_t* update_reg(char* reg, int op, int value){
    iloc_t* ret;
    char* val = malloc(10*sizeof(char));
    sprintf(val, "%i", value);

    if(op == REG_INC)
        ret = create_iloc(ILOC_ADDI, reg, val, reg);
    else
        ret = create_iloc(ILOC_SUBI, reg, val, reg);

    add_to_tmp_list(val);

    return ret;
}
/*
 * Duplica o codigo iloc
 */
iloc_t * iloc_dup(iloc_t* iloc){
    iloc_t* dup = NULL;
    iloc_t* dup_aux;
    comp_dict_t *dict = dict_new();

    while (iloc){
        dup_aux = create_iloc(iloc->type, iloc->op1, iloc->op2, iloc->op3);
        dup_aux->comment = iloc->comment;
        if (iloc->type == ILOC_JUMPI)
            change_labels(dict, &dup_aux->op3, iloc->op3);
        else if (iloc->type == ILOC_CBR){
            change_labels(dict, &dup_aux->op2, iloc->op2);
            change_labels(dict, &dup_aux->op3, iloc->op3);
        }
        if (iloc->label)
            change_labels(dict, &dup_aux->label, iloc->label);

        dup = append_iloc(dup_aux, dup);
        iloc = iloc->prev;
    }
    for (int i = 0; i < dict->size; i++)
        if (dict->data[i])
            dict_remove(dict, (dict->data[i])->key);
    dict_free(dict);
    return dup;
}

iloc_t *foreach_iloc(comp_tree_t *tree, iloc_t **cc) {
    comp_tree_t *aux_tree = tree->first->next;
    comp_tree_t *atrib_aux_tree;
    iloc_t *ret = NULL, *block = tree->first->next->next->value->iloc, *id = tree->first->value->iloc;
    while (aux_tree->value->type == AST_EXP_LIST){
        tree->first->value->iloc = iloc_dup(id);
        atrib_aux_tree = createASTBinaryNode(AST_ATRIBUICAO, NULL, tree->first, aux_tree->first);
        ret = append_iloc(append_iloc(
                ret, atribuicao_iloc(atrib_aux_tree)), iloc_dup(block));
        aux_tree = aux_tree->last;
    }
    // append a ultima exp da lista
    atrib_aux_tree = createASTBinaryNode(AST_ATRIBUICAO, NULL, tree->first, aux_tree);
    return append_iloc(append_iloc(append_iloc(
            ret, atribuicao_iloc(atrib_aux_tree)), block), cc[3]);
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

iloc_t* atribuicao_iloc(comp_tree_t* tree){
    int op;
    iloc_t *address_iloc, *store, *load_true, *load_false, *jump_store, *iloc_id, *iloc_val, *iloc_next;
    iloc_id = tree->first->value->iloc;
    iloc_val = tree->first->next->value->iloc;
    iloc_next = tree->childnodes == 3 ? tree->last->value->iloc : NULL;
    char* address;
    if (tree->first->value->type == AST_VETOR_INDEXADO){
        iloc_id = iloc_id->prev; // o ultimo iloc de um vetor indexado é o LOADAO, sendo o penultimo o calculo do ender
        iloc_id->next = NULL;  // remove o LOADAO deixando o calculo do ender como o ultimo iloc
        address_iloc = iloc_id;
        address = address_iloc->op3;
        op = ILOC_STOREAO;
    }
    else if(tree->first->value->type == AST_ATRIBUTO){
        address_iloc = NULL;
        address = get_char_address(tree->first);
        op = ILOC_STOREAI;
    }
    else{
        address_iloc = NULL;
        address = get_char_address(tree->first);
        op = ILOC_STOREAI;
    }

    store = create_iloc(
            op,
            iloc_val->op3,
            get_especial_reg(tree->first),
            address);

    int var_type = tree->first->value->value_type;
    int ast_type = tree->first->next->value->type;
    int is_bool = var_type == decl_variable(POA_LIT_BOOL) || var_type == decl_vector(POA_LIT_BOOL);

    // aplica o curto circuito apenas se não for um literal ou negação de um literal
    if ((ast_type != AST_LITERAL) && is_bool){

        if (ast_type == AST_LOGICO_COMP_NEGACAO && tree->first->next->first->value->type == AST_LITERAL){ // verifica negação de um literal
            iloc_val = create_iloc(
                    ILOC_LOADAI,
                    get_especial_reg(tree->first->next->first),
                    get_char_address(tree->first->next->first),
                    create_reg());
        } else {
            load_true = create_iloc(ILOC_LOADI, "1", NULL, create_reg());
            load_true->label = create_label();
            patchup_true(tree->first->next, load_true->label);

            store->label = create_label();

            jump_store = create_iloc(ILOC_JUMPI, NULL, NULL, store->label);

            load_false = create_iloc(ILOC_LOADI, "0", NULL, load_true->op3);
            load_false->label = create_label();
            patchup_false(tree->first->next, load_false->label);

            iloc_val = append_iloc(append_iloc(append_iloc(
                    iloc_val, load_true), jump_store), load_false);

            store->op1 = load_true->op3;
        }
    }

    return append_iloc(append_iloc(append_iloc(
            address_iloc, iloc_val), store), iloc_next);
}

void set_attribute_address(comp_tree_t* tree, symbol_t* attribute){
    int scope = tree->value->var_scope;

    id_value_t* decl_value = (id_value_t*)tree->first->value->symbol->value;
    symbol_t* class = (symbol_t*) decl_value->decl_info[scope];
    id_value_t* class_value = (id_value_t*) class->value;
    class_info_t* class_info = (class_info_t*) class_value->decl_info[0];

    int end = decl_value->address[scope];
    for(int i = 0; i < class_info->field_length; i++){  // busca o atributo na lista de atributos da classe
        if(class_info->field_id[i] == attribute)
            break;
        end += size_of(class_info->field_type[i]);
    }
    tree->value->address = end;
}

iloc_t** get_children_iloc_list(comp_tree_t* tree){
    iloc_t** cc = calloc(tree->childnodes + 1, sizeof(iloc_t*)); // +1 porque há possibilidade de existir próximo comando

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
        //TODO case AST_LOGICO_COMP_NEGACAO: return ILOC_XORI;

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
        case ILOC_HALT: return "halt";

        default:
            fprintf (stderr, "%s: type provided is invalid here\n", __FUNCTION__);
            abort();
    }
}

void print_iloc(iloc_t* iloc){
    if(!iloc)
        return;

    if(iloc->label) fprintf(cfp, "%s: ", iloc->label);

    fprintf(cfp, "%s ", __iloc_instructions(iloc->type));

    switch (iloc->type){
        case ILOC_STORE:
        case ILOC_STOREAI:
        case ILOC_STOREAO:
        case ILOC_CBR:
            if(iloc->op1) fprintf(cfp, "%s => ", iloc->op1);
            if(iloc->op2) fprintf(cfp, "%s, ", iloc->op2);
            if(iloc->op3) fprintf(cfp, "%s ", iloc->op3);
            break;
        default:
            if(iloc->op1) fprintf(cfp, "%s", iloc->op1);
            if(iloc->op2) fprintf(cfp, ", %s", iloc->op2);
            if(iloc->op3) fprintf(cfp, " => %s ", iloc->op3);
    }
    if (iloc->comment) fprintf(cfp, "\t// %s", iloc->comment);

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
    if(tree->value->var_scope != main_scope && tree->value->var_scope != GLOBAL_SCOPE)
        sprintf(address, "%i", tree->value->address + RA_SIZE);
    else
        sprintf(address, "%i", tree->value->address);
    add_to_tmp_list(address);
    return address;
}

char* get_literal_value(comp_tree_t *literal){
    char* address = malloc(20*sizeof(char));
    sprintf(address, "%i", *(int*)literal->value->symbol->value);
    add_to_tmp_list(address);
    return address;
}

iloc_t* get_last_iloc(iloc_t* iloc){
    iloc_t* last = iloc;
    while (last->prev)
        last = last->prev;
    return last;
}

void concat_true(comp_tree_t* tree){
    tree->value->rem_true = calloc(MAX_ILOC, sizeof(char*));
    tree->value->rem_true_size = 0;

    for(int i = 0; i < tree->first->value->rem_true_size; i++)
        add_rem_true(tree, tree->first->value->rem_true[i]);

    for(int i = 0; i < tree->last->value->rem_true_size; i++)
        add_rem_true(tree, tree->last->value->rem_true[i]);
}

void concat_false(comp_tree_t* tree){
    tree->value->rem_false = calloc(MAX_ILOC, sizeof(char*));
    tree->value->rem_false_size = 0;

    for(int i = 0; i < tree->first->value->rem_false_size; i++)
        add_rem_false(tree, tree->first->value->rem_false[i]);

    for(int i = 0; i < tree->last->value->rem_false_size; i++)
        add_rem_false(tree, tree->last->value->rem_false[i]);
}

void add_rem_false(comp_tree_t *tree, char **remendo){
    if (!tree->value->rem_false){
        tree->value->rem_false = calloc(MAX_ILOC, sizeof(char*));
        tree->value->rem_false_size = 0;
        add_to_tmp_list(tree->value->rem_false);
    }
    tree->value->rem_false[tree->value->rem_false_size++] = remendo;
}

void add_rem_true(comp_tree_t *tree, char **remendo){
    if (!tree->value->rem_true){
        tree->value->rem_true = calloc(MAX_ILOC, sizeof(char*));
        tree->value->rem_true_size = 0;
        add_to_tmp_list(tree->value->rem_true);
    }
    tree->value->rem_true[tree->value->rem_true_size++] = remendo;
}

void patchup_false(comp_tree_t* tree, char* label){
    if (!tree->value->rem_false)
        return;
    for(int i = 0; i < tree->value->rem_false_size; i++)
        *tree->value->rem_false[i] = label;
}

void patchup_true(comp_tree_t* tree, char* label){
    if (!tree->value->rem_true)
        return;
    for(int i = 0; i < tree->value->rem_true_size; i++)
        *tree->value->rem_true[i] = label;
}

void short_circuit_literal(comp_tree_t *tree, iloc_t **iloc){
    if (tree->value->type == AST_LITERAL){
        //Cria um iloc de jumpi com um remendo
        *iloc = create_iloc(ILOC_JUMPI, NULL, NULL, NULL);
        if (*(int*)tree->value->symbol->value == TRUE)
            add_rem_true(tree, &(*iloc)->op3);
        else
            add_rem_false(tree, &(*iloc)->op3);
    }
}

void short_circuit_variable(comp_tree_t *tree, iloc_t **iloc){
    if (tree->value->type == AST_IDENTIFICADOR || tree->value->type == AST_VETOR_INDEXADO){
        //Cria um iloc de cbr com dois remendos
        iloc_t * cbr = create_iloc(ILOC_CBR, (*iloc)->op3, NULL, NULL);
        add_rem_true(tree, &cbr->op2);
        add_rem_false(tree, &cbr->op3);
        *iloc = append_iloc(*iloc, cbr);
    }
}

void set_main_scope(int scope){
    main_scope = scope;
}

int get_number_of_instructions(iloc_t* iloc){
    iloc_t* tmp = iloc;
    int i = 1;
    while (tmp->prev){
        tmp = tmp->prev;
        i++;
    }
    return i;
}

char* create_reg(){
    char* reg = create_register();
    reg_list[reg_list_length++] = reg;
    return reg;
}
