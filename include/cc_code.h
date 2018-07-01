/*
Grupo Epsilon:
  - Douglas Flores
  - Filipe Santos
*/

#ifndef COMPILADOR_CC_CODE_H
#define COMPILADOR_CC_CODE_H

#include "cc_misc.h"
#include "cc_tree.h"

// Tipos de operações
#define NORMAL        1
#define FLUX_CONTROL  2

// Tipos de operandos
#define REGISTER  1
#define NUMBER    2
#define LABEL     3

/* Código de Operações*/
#define ILOC_NOP   0
//aritméticas
#define ILOC_ADD     1
#define ILOC_SUB     2
#define ILOC_MULT    3
#define ILOC_DIV     4
#define ILOC_ADDI    5
#define ILOC_SUBI    6
#define ILOC_RSUBI   7
#define ILOC_MULTI   8
#define ILOC_DIVI    9
#define ILOC_RDIVI   10
//shifts
#define ILOC_LSHIFT    11
#define ILOC_LSHIFTI   12
#define ILOC_RSHIFT    13
#define ILOC_RSHIFTI   14
//lógicas
#define ILOC_AND   15
#define ILOC_ANDI  16
#define ILOC_OR    17
#define ILOC_ORI   18
#define ILOC_XOR   19
#define ILOC_XORI  20
//carga de registradores
#define ILOC_LOADI 21
//operações sobre a memória
#define ILOC_LOAD      22
#define ILOC_LOADAI    23
#define ILOC_LOADAO    24
#define ILOC_CLOAD     25
#define ILOC_CLOADAI   26
#define ILOC_CLOADAO   27
#define ILOC_STORE     28
#define ILOC_STOREAI   29
#define ILOC_STOREAO   30
#define ILOC_CSTORE    31
#define ILOC_CSTOREAI  32
#define ILOC_CSTOREAO  33
//cópia entre registradores
#define ILOC_I2I   34
#define ILOC_C2C   35
#define ILOC_C2I   36
#define ILOC_I2C   37
//saltos
#define ILOC_JUMPI   38
#define ILOC_JUMP    39
//fluxo de controle
#define ILOC_CBR     40
#define ILOC_CMP_LT  41
#define ILOC_CMP_LE  42
#define ILOC_CMP_EQ  43
#define ILOC_CMP_GE  44
#define ILOC_CMP_GT  45
#define ILOC_CMP_NE  46

// Operations with regs
#define REG_INC   0
#define REG_DEC   1

#define RA_SIZE   16

typedef struct iloc{
    int type;
    char* label;
    char* op1;
    char* op2;
    char* op3;
    struct iloc* prev;
    struct iloc* next;
}iloc_t;

void code_init(const char* filename);
void code_close();
void build_iloc_code(comp_tree_t* tree);
iloc_t* create_iloc(int type, char* op1, char* op2, char* op3);
iloc_t* code_generator(comp_tree_t* tree);
iloc_t* append_iloc(iloc_t* iloc1, iloc_t* iloc2);
void print_iloc(iloc_t* iloc);
void print_iloc_list(iloc_t* iloc);
iloc_t* invert_iloc_list(iloc_t* last);
void set_attribute_address(comp_tree_t* tree, symbol_t* attribute);
void set_main_scope(int scope);



#endif
