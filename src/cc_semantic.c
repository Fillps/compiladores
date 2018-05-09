//
// Created by filipe on 5/9/18.
//
#include <stdlib.h>
#include "cc_semantic.h"
#include "cc_dict.h"

function_info_t* function_info;

int scope_stack[DICT_SIZE];
int *sp;
int current_scope;
int scope_stack_length;

#define push(sp, n) (*((sp)++) = (n))
#define pop(sp) (*--(sp))

void scope_init(){
    sp = scope_stack;
    current_scope = 0;
    push(sp, current_scope);
    scope_stack_length = 1;
}

void start_scope(){
    current_scope++;
    push(sp, current_scope);
    scope_stack_length++;
}

void end_scope(){
    pop(sp);
    scope_stack_length--;
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

void wrong_type_error(symbol_t* symbol, int correct_type, int wrong_type){
    fprintf (stderr, "IKS_ERROR_WRONG_TYPE(line: %d, id: %s) \'%s\' type was supposed to be \'%s\', but was found \'%s\'.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
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

void check_declared(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (value->type[scope_stack[i]] != UNDECLARED)
            return;

    undeclared_error(symbol);
}

void declare(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (value->type[scope_stack[i]] != UNDECLARED)
            declared_error(symbol);

    value->type[current_scope] = type;
}

void declare_non_primitive(symbol_t* symbol, int type, symbol_t* class_type){
    id_value_t* value = symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (value->type[scope_stack[i]] != UNDECLARED)
            declared_error(symbol);

    value->type[current_scope] = type;
    value->decl_info[current_scope] = class_type;
}

void declare_function(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (value->type[scope_stack[i]] != UNDECLARED)
            declared_error(symbol);

    value->type[current_scope] = DECL_FUNCTION;
    function_info->type = type;
    value->decl_info[current_scope] = function_info;
}

int check_usage(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (value->type[scope_stack[i]] == type)
            return TRUE;

    return FALSE;
}

void check_usage_variable(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (value->type[scope_stack[i]] != UNDECLARED)
            if (value->type[scope_stack[i]] < decl_variable(POA_LIT_INT) || value->type[scope_stack[i]] > decl_variable(POA_IDENT))
                variable_error(symbol);
            else if (value->type[scope_stack[i]] != type)
                wrong_type_error(symbol, value->type[scope_stack[i]], type);

}

void check_usage_vector(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (value->type[scope_stack[i]] != UNDECLARED)
            if (value->type[scope_stack[i]] < decl_vector(POA_LIT_INT) || value->type[scope_stack[i]] > decl_vector(POA_LIT_BOOL))
                vector_error(symbol);
            else if (value->type[scope_stack[i]] != type)
                wrong_type_error(symbol, value->type[scope_stack[i]], type);
}

void check_usage_function(comp_tree_t* tree){
    symbol_t* symbol = tree->first->value->symbol;
    id_value_t* id_value = (id_value_t *) symbol->value;

    for(int i = 0; i < scope_stack_length; i++)
        if (id_value->type[scope_stack[i]] == DECL_FUNCTION){
            function_info_t* func_info =  id_value->decl_info[scope_stack[i]];
            if (tree->childnodes - 1 < func_info->params_length)
                missing_args_error(symbol, func_info->params_length, tree->childnodes - 1);
            else if (tree->childnodes - 1 > func_info->params_length)
                excess_args_error(symbol, func_info->params_length, tree->childnodes - 1);
            else{

            }
            return;
        }
        else if (id_value->type[scope_stack[i]] != UNDECLARED)
            function_error(symbol);

    declared_error(symbol);
}

void check_usage_class(symbol_t* symbol){
    if (check_usage, DECL_CLASS == FALSE)
        class_error(symbol);
}

void create_params(){
    function_info = malloc(sizeof(function_info_t));
    function_info->params_length = 0;
}

void add_param(symbol_t* symbol, int type){
    function_info->param_id[function_info->params_length] = symbol;
    function_info->param_type[function_info->params_length] = type;
    function_info->params_length = function_info->params_length + 1;
}