/*
Grupo Epsilon:
  - Douglas Flores
  - Filipe Silva
*/
#include <stdlib.h>
#include <string.h>
#include "cc_semantic.h"
#include "cc_dict.h"
#include "parser.h"

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

void class_undeclared_error(symbol_t* symbol){
    fprintf (stderr, "IKS_ERROR_CLASS_UNDDECLARED(line: %d, id: %s) The class \'%s\' was not declared.\n",
             symbol->line, symbol->lexeme, symbol->lexeme);
    exit(IKS_ERROR_CLASS_UNDDECLARED);
}

void wrong_type_error(symbol_t* symbol, char* correct_type, char* wrong_type){
    fprintf (stderr, "IKS_ERROR_WRONG_TYPE(line: %d, id: %s) \'%s\' type was supposed to be \'%s\', but was found \'%s\'.\n",
             symbol->line, symbol->lexeme, symbol->lexeme, correct_type, wrong_type);
    exit(IKS_ERROR_WRONG_TYPE);
}

void different_type_warning(symbol_t* symbol, char* correct_type, char* given_type){
    fprintf (stderr, "WARNING**(line: %d, id: %s) \'%s\' type was converted to \'%s\'.\n",
             symbol->line, symbol->lexeme, given_type, correct_type);
}

void wrong_type_args_error(symbol_t* var, int correct_type, int wrong_type){
    switch (wrong_type) {
        case decl_variable(POA_LIT_INT):
            if(correct_type == decl_variable(POA_LIT_FLOAT))
                different_type_warning(var, __type_description(correct_type), __type_description(wrong_type));
            break;
        case decl_variable(POA_LIT_FLOAT):
            if(correct_type == decl_variable(POA_LIT_INT))
                different_type_warning(var, __type_description(correct_type), __type_description(wrong_type));
            break;
        default:
            fprintf (stderr, "IKS_ERROR_WRONG_TYPE_ARGS(line: %d, id: %s) correct arg type is \'%s\', but was given \'%s\' \'%s\'.\n",
                     var->line, var->lexeme, __type_description(correct_type), __type_description(wrong_type), var->lexeme);
            exit(IKS_ERROR_WRONG_TYPE_ARGS);
    }
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

void wrong_type_assignment(symbol_t* var, int correct_type, int wrong_type){
    switch (wrong_type) {
        case decl_variable(POA_LIT_STRING):
            fprintf (stderr, "IKS_ERROR_CHAR_TO_X(line: %d, id: %s) correct type is \'%s\', but was given \'%s\'.\n",
                     var->line, var->lexeme, __type_description(correct_type), __type_description(wrong_type));
            exit(IKS_ERROR_STRING_TO_X);
        case decl_variable(POA_LIT_CHAR):
            fprintf (stderr, "IKS_ERROR_CHAR_TO_X(line: %d, id: %s) correct type is \'%s\', but was given \'%s\'.\n",
                     var->line, var->lexeme, __type_description(correct_type), __type_description(wrong_type));
            exit(IKS_ERROR_CHAR_TO_X);
        case decl_variable(POA_LIT_INT):
            if(correct_type == decl_variable(POA_LIT_FLOAT))
                different_type_warning(var, __type_description(correct_type), __type_description(wrong_type));
            else
                wrong_type_error(var, __type_description(correct_type), __type_description(wrong_type));
            break;
        case decl_variable(POA_LIT_FLOAT):
            if(correct_type == decl_variable(POA_LIT_INT))
                different_type_warning(var, __type_description(correct_type), __type_description(wrong_type));
            else
                wrong_type_error(var, __type_description(correct_type), __type_description(wrong_type));
            break;
        default:
            fprintf (stderr, "IKS_ERROR_WRONG_TYPE(line: %d, id: %s) \'%s\' type is \'%s\', but was given \'%s\'.\n",
                     var->line, var->lexeme, var->lexeme, __type_description(correct_type), __type_description(wrong_type));
            exit(IKS_ERROR_WRONG_TYPE);
    }
}

void invalid_exp(comp_tree_t* exp_tree, char* correct_type, int wrong_type, char* token){
    /* Busca a linha da expressão */
    int line = 0;
    while(exp_tree->first){
        exp_tree = exp_tree->first;
    }
    line = exp_tree->value->symbol->line;

    fprintf (stderr, "IKS_ERROR_INVALID_EXP(line: %d, id: %s) \'%s\' is a \'%s\' expression, but was found one \'%s\' operand.\n",
            line, token, token, correct_type, __type_description(wrong_type));
}

void invalid_condition_error(comp_tree_t* exp_tree, char* comand, char* correct_type, int wrong_type){
    /* Busca a linha da expressão */
    int line = 0;
    while(exp_tree->first){
        exp_tree = exp_tree->first;
    }
    line = exp_tree->value->symbol->line;

    /* Imprime o erro */
    fprintf (stderr, "IKS_ERROR_INVALID_CONDITION(line: %d, id: %s) \'%s\' condition must be a \'%s\' expression but was given a \'%s\' expression.\n",
             line, comand, comand, correct_type, __type_description(wrong_type));
    exit(IKS_ERROR_INVALID_CONDITION);
}

int check_declared(symbol_t* symbol){
    id_value_t* value = symbol->value;

    for(int i = scope_stack_length - 1; i >= 0 ; i--)
        if (value->type[scope_stack[i]] != UNDECLARED)
            return i;

    undeclared_error(symbol);
}

void check_usage_class(symbol_t* symbol){
    id_value_t* value = symbol->value;

    if (value->type[GLOBAL_SCOPE] == DECL_CLASS)
        return;
    else if (value->type[GLOBAL_SCOPE] != UNDECLARED)
        class_error(symbol);

    class_undeclared_error(symbol);
}

void declare(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    if (value->type[current_scope] != UNDECLARED)
        declared_error(symbol);

    if (type == decl_variable(POA_LIT_STRING))
        value->address[current_scope] = -1;
    else if (current_scope == GLOBAL_SCOPE)
        value->address[current_scope] = get_global_address(INT_SIZE);
    else
        value->address[current_scope] = get_local_address(INT_SIZE);

    value->type[current_scope] = type;
}

void declare_vector(symbol_t* symbol, int type, int* size){
    id_value_t* value = symbol->value;

    if (value->type[current_scope] != UNDECLARED)
        declared_error(symbol);

    if (type == decl_vector(POA_LIT_STRING))
        value->address[current_scope] = -1; //TODO pensar qual é a melhor forma de resolver a string(nao sabemos qual é o tamanho dela na declaracao)
    else if (current_scope == GLOBAL_SCOPE)
        value->address[current_scope] = get_global_address(*size*INT_SIZE);
    else
        value->address[current_scope] = get_local_address(*size*INT_SIZE);

    value->type[current_scope] = type;
    value->decl_info[current_scope] = size;
}

void declare_non_primitive(symbol_t* symbol, int type, symbol_t* class_type){

    check_usage_class(class_type);

    id_value_t* value = symbol->value;

    if (value->type[current_scope] != UNDECLARED)
        declared_error(symbol);

    //TODO value->address[current_scope] calculando o tamanho da classe

    value->type[current_scope] = type;
    value->decl_info[current_scope] = class_type;
}

//TODO criar nova funcao q declara vetores de nao primitivos.

void declare_function(symbol_t* symbol, int type){
    id_value_t* value = symbol->value;

    if (value->type[GLOBAL_SCOPE] != UNDECLARED)
        declared_error(symbol);

    value->type[GLOBAL_SCOPE] = DECL_FUNCTION;
    function_info->type = type;
    value->decl_info[GLOBAL_SCOPE] = function_info;
}

void declare_class(symbol_t* symbol){
    id_value_t* value = symbol->value;

    if (value->type[GLOBAL_SCOPE] != UNDECLARED)
        declared_error(symbol);

    value->type[GLOBAL_SCOPE] = DECL_CLASS;
    value->decl_info[GLOBAL_SCOPE] = class_info;
}

void check_usage_variable(comp_tree_t* tree){
    id_value_t* value = tree->value->symbol->value;

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if (value->type[scope_stack[i]] != UNDECLARED)
            if (value->type[scope_stack[i]] >= decl_vector(POA_LIT_INT) && value->type[scope_stack[i]] <= decl_vector(POA_LIT_BOOL))
                vector_error(tree->value->symbol);
            else if (value->type[scope_stack[i]] == DECL_FUNCTION)
                function_error(tree->value->symbol);
            else if (value->type[scope_stack[i]] == DECL_CLASS)
                class_error(tree->value->symbol);
            else{
                tree->value->var_scope = scope_stack[i];
                tree->value->address = value->address[scope_stack[i]];
                return;
            }

    undeclared_error(tree->value->symbol);
}

void check_usage_vector(comp_tree_t* tree){
    id_value_t* value = tree->value->symbol->value;

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if (value->type[scope_stack[i]] != UNDECLARED)
            if (value->type[scope_stack[i]] >= decl_variable(POA_LIT_INT) && value->type[scope_stack[i]] <= decl_variable(POA_IDENT))
                variable_error(tree->value->symbol);
            else if (value->type[scope_stack[i]] == DECL_FUNCTION)
                function_error(tree->value->symbol);
            else if (value->type[scope_stack[i]] == DECL_CLASS)
                class_error(tree->value->symbol);
            else{
                tree->value->var_scope = scope_stack[i];
                tree->value->address = value->address[scope_stack[i]];
                return;
            }

    undeclared_error(tree->value->symbol);
}

void check_usage_function(comp_tree_t* tree){
    symbol_t* symbol = tree->first->value->symbol;
    id_value_t* id_value = (id_value_t *) symbol->value;
    comp_tree_t* params_tree;
    symbol_t* param;
    int param_type;

    if(tree->childnodes-1 > 0){
        params_tree = tree->first->next;
        for(int i = 0; i < tree->childnodes - 2; i++)
            params_tree = params_tree->next;
        param = params_tree->value->symbol;
        param_type = params_tree->value->value_type;
    }


    if (id_value->type[GLOBAL_SCOPE] == DECL_FUNCTION){
        function_info_t* func_info =  id_value->decl_info[GLOBAL_SCOPE];
        if (tree->childnodes - 1 < func_info->params_length)
	        missing_args_error(symbol, func_info->params_length, tree->childnodes - 1);
        else if (tree->childnodes - 1 > func_info->params_length)
	        excess_args_error(symbol, func_info->params_length, tree->childnodes - 1);
        else{
	        for(int i = tree->childnodes-2; i >= 0; i--){     //percorre todos os parâmetos
	            if(param_type != func_info->param_type[i])
		              wrong_type_args_error(param, func_info->param_type[i], param_type);

	            //passa para o parametro anterior fornecido na chamada
	            params_tree = params_tree->prev;
	            param = params_tree->value->symbol;
	            param_type = params_tree->value->value_type;
	        }
        }
        return;
    }
    else if (id_value->type[GLOBAL_SCOPE] != UNDECLARED)
        function_error(symbol);

    undeclared_error(symbol);
}

void check_usage_attribute(comp_tree_t* tree){
    symbol_t* class_var = tree->first->value->symbol;
    symbol_t* attribute = tree->last->value->symbol;
    id_value_t* id_value = (id_value_t *) class_var->value;

    for(int i = scope_stack_length - 1; i >= 0; i--) //procura a declaracao da variavel de classe
        if (id_value->type[scope_stack[i]] == decl_variable(POA_IDENT)){
            symbol_t* class = ((symbol_t *)id_value->decl_info[scope_stack[i]]);
            id_value_t* class_value = (id_value_t*)class->value;
            tree->value->var_scope = scope_stack[i];
            tree->value->address = id_value->address[scope_stack[i]];

            if (class_value->type[GLOBAL_SCOPE] == DECL_CLASS){
                class_info_t* cl_info =  class_value->decl_info[GLOBAL_SCOPE];

                for (int j = 0; j < cl_info->field_length; j++) // procura a declaracao do attibuto
                    if (attribute == cl_info->field_id[j])
                        return;
                attribute_undeclared_error(class, attribute);
            }
            else if (class_value->type[GLOBAL_SCOPE] != UNDECLARED)
                class_error(class);
            else
                class_undeclared_error(class);

        }
        else if (id_value->type[scope_stack[i]] != UNDECLARED)
            wrong_type_error(class_var, "class variable", __type_description(id_value->type[scope_stack[i]]));

    undeclared_error(class_var);
}

void create_params(){
    function_info = calloc(1, sizeof(function_info_t));
    function_info->params_length = 0;
}

void add_param(symbol_t* symbol, int type){
    function_info->param_id[function_info->params_length] = symbol;
    function_info->param_type[function_info->params_length] = decl_variable(type);
    function_info->params_length++;
}

void create_class_fields(){
    class_info = malloc(sizeof(class_info_t));
    class_info->field_length = 0;
}

void class_add_field(symbol_t* symbol, int type){
    class_info->field_id[class_info->field_length] = symbol;
    class_info->field_type[class_info->field_length] = decl_variable(type);
    class_info->field_length++;
}

int get_var_type(symbol_t* var){
    id_value_t* id_var = var->value;
    int var_type;

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if(id_var->type[scope_stack[i]] != UNDECLARED){
            var_type = id_var->type[scope_stack[i]];
            break;
        }

    return var_type;
}

int get_vector_type(symbol_t* symbol){
    id_value_t* value = symbol->value;

    for(int i = scope_stack_length - 1; i >= 0; i--)
        if (value->type[scope_stack[i]] != UNDECLARED)
            return value->type[scope_stack[i]];

    return 0;
}

int get_attribute_type(symbol_t* class_var, symbol_t* attribute){
    id_value_t* id_value = (id_value_t *) class_var->value;

    for(int i = scope_stack_length - 1; i >= 0; i--) //procura a declaracao da variavel de classe
        if (id_value->type[scope_stack[i]] == decl_variable(POA_IDENT)){
            symbol_t* class = (symbol_t *)id_value->decl_info[scope_stack[i]];
            id_value_t* class_value = (id_value_t*)class->value;

            if (class_value->type[GLOBAL_SCOPE] == DECL_CLASS){
                class_info_t* cl_info =  class_value->decl_info[GLOBAL_SCOPE];

                for (int j = 0; j < cl_info->field_length; j++) // procura a declaracao do attibuto
                    if (attribute == cl_info->field_id[j])
                        return cl_info->field_type[j];
                attribute_undeclared_error(class, attribute);
            }
            else if (class_value->type[GLOBAL_SCOPE] != UNDECLARED)
                class_error(class);
            else
                class_undeclared_error(class);

        }
        else if (id_value->type[scope_stack[i]] != UNDECLARED)
            wrong_type_error(class_var, "class variable", __type_description(id_value->type[GLOBAL_SCOPE]));

    undeclared_error(class_var);
}

int get_func_type(comp_tree_t* tree){
    symbol_t* symbol = tree->first->value->symbol;
    id_value_t* id_value = (id_value_t *) symbol->value;
    int func_type = 0;

    if (id_value->type[GLOBAL_SCOPE] == DECL_FUNCTION){
        function_info_t* func_info = id_value->decl_info[GLOBAL_SCOPE];
        func_type = decl_variable(func_info->type);
    }
    else if (id_value->type[GLOBAL_SCOPE] != UNDECLARED)
        function_error(symbol);

    return func_type;
}

void check_var_assignment(comp_tree_t* var, int var_type, int exp_type){
    if(var_type != exp_type &&
       decl_variable(var_type) != decl_vector(exp_type) &&
       decl_vector(var_type) != decl_variable(exp_type))
       wrong_type_assignment(var->value->symbol, var_type, exp_type);

}

char* get_token_name(int token){
    char* name = "unknown token";

    switch (token) {
        case TK_OC_LE:
            name = "<=";
            break;
        case TK_OC_GE:
            name = ">=";
            break;
        case TK_OC_EQ:
            name = "==";
            break;
        case TK_OC_NE:
            name = "!=";
            break;
        case TK_OC_AND:
            name = "&&";
        case TK_OC_OR:
            name = "||";
            break;
        case SEM_GREATER:
            name = ">";
            break;
        case SEM_LESSER:
            name = "<";
            break;
        case SEM_SUM:
            name = "+";
            break;
        case SEM_SUB:
            name = "-";
            break;
        case SEM_MULT:
            name = "*";
            break;
        case SEM_DIV:
            name = "/";
            break;
        case SEM_MOD:
            name = "%";
            break;
        case SEM_POT:
            name = "^";
            break;
        case TK_PR_IF:
            name = "if";
            break;
        case TK_PR_DO:
            name = "do-while";
            break;
        case TK_PR_WHILE:
            name = "while";
            break;
        case TK_PR_FOR:
            name = "for";
            break;
        case TK_PR_SWITCH:
            name = "switch-case";
            break;
        case TK_PR_FOREACH:
            name = "foreach";
            break;
    }
    return name;
}

void check_condition(comp_tree_t* exp, int token){
    int exp_type = exp->value->value_type;
    char* comand = get_token_name(token);

    switch (token) {
        case TK_PR_IF:
        case TK_PR_DO:
        case TK_PR_WHILE:
        case TK_PR_FOR:
            if(exp_type != decl_variable(POA_LIT_BOOL))
                invalid_condition_error(exp, comand, "logic", exp_type);
            break;
        case TK_PR_SWITCH:
            if(exp_type != decl_variable(POA_LIT_INT) && exp_type != decl_variable(POA_LIT_CHAR))
                invalid_condition_error(exp, comand, "integer or char", exp_type);
            break;
    }
}

void set_unary_node_value_type(comp_tree_t* node, int value_type){
	node->value->value_type = value_type;
}

void set_binary_node_value_type(comp_tree_t* node, int op_type, int op_token){
    int type_left = node->first->value->value_type;
    int type_right = node->last->value->value_type;

    switch (op_type) {
				case CMP_BOOL:
            if(type_left != decl_variable(POA_LIT_BOOL)){
                node->value->value_type = type_left;
                invalid_exp(node, "boolean", type_left, get_token_name(op_token));
            }
            else if(type_right != decl_variable(POA_LIT_BOOL)){
                node->value->value_type = type_right;
                invalid_exp(node, "boolean", type_right, get_token_name(op_token));
            }
            else
                node->value->value_type = decl_variable(POA_LIT_BOOL);
            break;
        case CMP_ARITM_BOOL:
            if(type_left == decl_variable(POA_LIT_BOOL))
                if(type_right == decl_variable(POA_LIT_BOOL)){
                    node->value->value_type = decl_variable(POA_LIT_BOOL);
                    break;
                }
                else{
                    node->value->value_type = type_right;
                    invalid_exp(node, "boolean", type_right, get_token_name(op_token));
                }
        case CMP_ARITM:
            if(type_left != decl_variable(POA_LIT_INT) && type_left != decl_variable(POA_LIT_FLOAT)){
                node->value->value_type = type_left;
                invalid_exp(node, "aritmetic", type_left, get_token_name(op_token));
            }
            else if(type_right != decl_variable(POA_LIT_INT) && type_right != decl_variable(POA_LIT_FLOAT)){
                node->value->value_type = type_right;
                invalid_exp(node, "aritmetic", type_right, get_token_name(op_token));
            }
            else
                node->value->value_type = decl_variable(POA_LIT_BOOL);
            break;
        case ARITM:
            if(type_left != decl_variable(POA_LIT_INT) && type_left != decl_variable(POA_LIT_FLOAT)){
                node->value->value_type = type_left;
                invalid_exp(node, "aritmetic", type_left, get_token_name(op_token));
            }
            else if(type_right != decl_variable(POA_LIT_INT) && type_right != decl_variable(POA_LIT_FLOAT)){
                node->value->value_type = type_right;
                invalid_exp(node, "aritmetic", type_right, get_token_name(op_token));
            }
            else if(type_left == decl_variable(POA_LIT_FLOAT) || type_right == decl_variable(POA_LIT_FLOAT))
                node->value->value_type = decl_variable(POA_LIT_FLOAT);
            else
                node->value->value_type = decl_variable(POA_LIT_INT);
            break;
		}
}

void set_pipe_type(comp_tree_t* pipes){
    comp_tree_t* tree = pipes;

    while (tree->value->type == AST_ENCADEAMENTO_PIPE || tree->value->type == AST_ENCADEAMENTO_PIPEG) {
        set_unary_node_value_type(tree, tree->first->value->value_type);
        tree = tree->first->first->next;
    }
}

void check_pipe(comp_tree_t* pipes){

    set_pipe_type(pipes);

    comp_tree_t* tree = pipes->first;

    while (tree->value->type == AST_CHAMADA_DE_FUNCAO) {
        check_usage_function(tree);
        tree = tree->first->next->first;
    }
}
