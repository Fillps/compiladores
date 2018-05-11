//
// Created by filipe on 5/9/18.
//
#include <stdlib.h>
#include "cc_semantic.h"
#include "cc_dict.h"

function_info_t* function_info;
class_info_t* class_info;

int scope_stack[DICT_SIZE];
int *sp;
int current_scope;
int scope_counter;
int scope_stack_length;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))

static inline char *__type_description (int type)
{
    switch (type){
        case DECL_CLASS: return "class";
        case DECL_FUNCTION: return "function";
        case decl_variable(POA_LIT_INT): return "int";
        case decl_variable(POA_LIT_FLOAT): return "float";
        case decl_variable(POA_LIT_CHAR): return "char";
        case decl_variable(POA_LIT_STRING): return "string";
        case decl_variable(POA_LIT_BOOL): return "bool";
        case decl_vector(POA_LIT_INT): return "int[]";
        case decl_vector(POA_LIT_FLOAT): return "float[]";
        case decl_vector(POA_LIT_CHAR): return "char[]";
        case decl_vector(POA_LIT_STRING): return "string[]";
        case decl_vector(POA_LIT_BOOL): return "bool[]";

        default:
            fprintf (stderr, "%s: type provided is invalid here\n", __FUNCTION__);
            abort();
    }
}

void scope_init(){
    sp = scope_stack;
    current_scope = 0;
    scope_counter = 0;
    push(sp, scope_counter);
    scope_stack_length = 1;
}

void start_scope(){
    scope_counter++;
    push(sp, scope_counter);
    scope_stack_length++;
    current_scope = scope_counter;
}

void end_scope(){
    pop(sp);
    scope_stack_length--;
    current_scope = scope_stack[scope_stack_length - 1];
}

void undeclared_error(symbol_t* symbol){
    fprintf (stderr, "IKS_ERROR_UNDECLARED(line: %d, id: %s): \'%s\' was not declared.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
    exit(IKS_ERROR_UNDECLARED);
}

void declared_error(symbol_t* symbol){
    fprintf (stderr, "IKS_ERROR_DECLARED(line: %d, id: %s) \'%s\' already declared.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
    exit(IKS_ERROR_DECLARED);
}

void variable_error(symbol_t* symbol){
    fprintf (stderr, "IKS_ERROR_VARIABLE(line: %d, id: %s) \'%s\' was supposed to be a variable.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
    exit(IKS_ERROR_VARIABLE);
}

void vector_error(symbol_t* symbol){
    fprintf (stderr, "IKS_ERROR_VECTOR(line: %d, id: %s) \'%s\' was supposed to be a vector.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
    exit(IKS_ERROR_VECTOR);
}

void function_error(symbol_t* symbol){
    fprintf (stderr, "IKS_ERROR_FUNCTION(line: %d, id: %s) \'%s\' was supposed to be a function.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
    exit(IKS_ERROR_FUNCTION);
}

void class_error(symbol_t* symbol){
    fprintf (stderr, "IKS_ERROR_CLASS(line: %d, id: %s) \'%s\' was supposed to be a class.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
    exit(IKS_ERROR_CLASS);
}

void wrong_type_error(symbol_t* symbol, char* correct_type, char* wrong_type){
    fprintf (stderr, "IKS_ERROR_WRONG_TYPE(line: %d, id: %s) \'%s\' type was supposed to be \'%s\', but was found \'%s\'.\n",
             symbol->line, symbol->lexeme, symbol->lexeme, correct_type, wrong_type);
    exit(IKS_ERROR_WRONG_TYPE);
}

void missing_args_error(symbol_t* symbol, int expected, int found){
    fprintf (stderr, "IKS_ERROR_MISSING_ARGS(line: %d, id: %s) the function \'%s\' was expected to have %i parameters, but was found %i.\n",
             symbol->line, symbol->lexeme, symbol->lexeme, expected, found);
    exit(IKS_ERROR_MISSING_ARGS);
}

void excess_args_error(symbol_t* symbol, int expected, int found){
    fprintf (stderr, "IKS_ERROR_EXCESS_ARGS(line: %d, id: %s) the function \'%s\' was expected to have %i parameters, but was found %i.\n",
             symbol->line, symbol->lexeme, symbol->lexeme, expected, found);
    exit(IKS_ERROR_EXCESS_ARGS);
}

void attribute_undeclared_error(symbol_t* class, symbol_t* attribute){
    fprintf (stderr, "IKS_ERROR_ATTRIBUTE_UNDECLARED(line: %d, id: %s) \'%s\' is not a attribute of the class \'%s\'.\n",
             attribute->line, attribute->lexeme, attribute->lexeme, class->lexeme);
    exit(IKS_ERROR_ATTRIBUTE_UNDECLARED);
}

void check_declared(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = scope_stack_length - 1; i >= 0 ; i--)
        if (value->type[scope_stack[i]] != UNDECLARED)
            return;

    undeclared_error(symbol);
}

void declare(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    if (value->type[scope_stack[current_scope]] != UNDECLARED)
        declared_error(symbol);

    value->type[current_scope] = type;
}

void declare_non_primitive(symbol_t* symbol, int type, symbol_t* class_type){
    id_value_t* value = symbol->value;

    if (value->type[scope_stack[current_scope]] != UNDECLARED)
        declared_error(symbol);

    value->type[current_scope] = type;
    value->decl_info[current_scope] = class_type;
}

void declare_function(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    if (value->type[scope_stack[current_scope]] != UNDECLARED)
        declared_error(symbol);

    value->type[current_scope] = DECL_FUNCTION;
    function_info->type = type;
    value->decl_info[current_scope] = function_info;
}

void declare_class(symbol_t* symbol){
    id_value_t* value = symbol->value;

    if (value->type[scope_stack[current_scope]] != UNDECLARED)
        declared_error(symbol);

    value->type[current_scope] = DECL_CLASS;
    value->decl_info[current_scope] = class_info;
}

int check_usage(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if (value->type[scope_stack[i]] == type)
            return TRUE;

    return FALSE;
}

void check_usage_variable(symbol_t* symbol){
    id_value_t* value = symbol->value;

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if (value->type[scope_stack[i]] != UNDECLARED)
            if (value->type[scope_stack[i]] >= decl_vector(POA_LIT_INT) && value->type[scope_stack[i]] <= decl_vector(POA_LIT_BOOL))
                vector_error(symbol);
            else if (value->type[scope_stack[i]] == DECL_FUNCTION)
                function_error(symbol);
            else if (value->type[scope_stack[i]] == DECL_CLASS)
                class_error(symbol);
            else
                return;

    undeclared_error(symbol);
}

void check_usage_vector(symbol_t* symbol){
    id_value_t* value = symbol->value;

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if (value->type[scope_stack[i]] != UNDECLARED)
            if (value->type[scope_stack[i]] >= decl_variable(POA_LIT_INT) && value->type[scope_stack[i]] <= decl_variable(POA_IDENT))
                variable_error(symbol);
            else if (value->type[scope_stack[i]] == DECL_FUNCTION)
                function_error(symbol);
            else if (value->type[scope_stack[i]] == DECL_CLASS)
                class_error(symbol);
            else
                return;

    undeclared_error(symbol);
}

void check_usage_function(comp_tree_t* tree){
    symbol_t* symbol = tree->first->value->symbol;
    id_value_t* id_value = (id_value_t *) symbol->value;

    comp_tree_t* params_tree;
    symbol_t* param;
    id_value_t* param_value;

    if(tree->childnodes-1 > 0){
        params_tree = tree->first->next;
        for(int i = 0; i < tree->childnodes - 2; i++)
            params_tree = params_tree->next;
        param = params_tree->value->symbol;
        param_value = (id_value_t *) param->value;
    }

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if (id_value->type[scope_stack[i]] == DECL_FUNCTION){
            function_info_t* func_info =  id_value->decl_info[scope_stack[i]];
            if (tree->childnodes - 1 < func_info->params_length)
                missing_args_error(symbol, func_info->params_length, tree->childnodes - 1);
            else if (tree->childnodes - 1 > func_info->params_length)
                excess_args_error(symbol, func_info->params_length, tree->childnodes - 1);
            else{
                for(int c = tree->childnodes-2; c >= 0; c--){     //percorre todos os parâmetos
                    int j;
                    for(j = scope_stack_length - 1; j>=0; j--)    //busca a declaração do parametro fornecido no escopo mais próximo
                        if(param_value->type[scope_stack[j]] != UNDECLARED)
                            break;
                    int func_param_type = decl_variable(func_info->param_type[c]);
                    if(param_value->type[scope_stack[j]] != func_param_type)
                        wrong_type_error(param, __type_description(func_param_type), __type_description(param_value->type[scope_stack[j]]));

                    //passa para o parametro anterior fornecido na chamada
                    params_tree = params_tree->prev;
                    param = params_tree->value->symbol;
                    param_value = (id_value_t *) param->value;
                }
            }
            return;
        }
        else if (id_value->type[scope_stack[i]] != UNDECLARED)
            function_error(symbol);

    undeclared_error(symbol);
}

void check_usage_class(symbol_t* symbol){
    if (check_usage, DECL_CLASS == FALSE)
        class_error(symbol);
}

void check_usage_attribute(symbol_t* class_var, symbol_t* attribute){
    id_value_t* id_value = (id_value_t *) class_var->value;

    for(int i = scope_stack_length - 1; i >= 0; i--) //procura a declaracao da variavel de classe
        if (id_value->type[scope_stack[i]] == decl_variable(POA_IDENT)){
            symbol_t* class = ((symbol_t *)id_value->decl_info)->value;
            id_value_t* class_value = (id_value_t*)class->value;

            for(int j = scope_stack_length - 1; j >= 0; j--) // procura a declaracao de classe
                if (class_value->type[scope_stack[j]] == DECL_CLASS){
                    class_info_t* cl_info =  class_value->decl_info[scope_stack[j]];

                    for (int k = 0; k < cl_info->field_length; k++) // procura a declaracao do attibuto
                        if (attribute == cl_info->field_id[k])
                            return;
                    attribute_undeclared_error(class, attribute);
                }
                else if (class_value->type[scope_stack[j]] != UNDECLARED)
                    class_error(class);

        }
        else if (id_value->type[scope_stack[i]] != UNDECLARED)
            wrong_type_error(class_var, "class variable", __type_description(id_value->type[scope_stack[i]]));

    undeclared_error(class_var);
}

void create_params(){
    function_info = malloc(sizeof(function_info_t));
    function_info->params_length = 0;
}

void add_param(symbol_t* symbol, int type){
    function_info->param_id[function_info->params_length] = symbol;
    function_info->param_type[function_info->params_length] = type;
    function_info->params_length++;
}

void create_class_fields(){
    class_info = malloc(sizeof(class_info_t));
    class_info->field_length = 0;
}

void class_add_field(symbol_t* symbol, int type){
    class_info->field_id[class_info->field_length] = symbol;
    class_info->field_type[class_info->field_length] = type;
    class_info->field_length++;
}
