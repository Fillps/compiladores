/*
Grupo Epsilon:
  - Douglas Flores
  - Filipe Santos
*/

#ifndef COMPILADOR_CC_CODE_H
#define COMPILADOR_CC_CODE_H

// Tipos de operações
#define NORMAL        1
#define FLUX_CONTROL  2

// Tipos de operandos
#define REGISTER  1
#define NUMBER    2
#define LABEL     3

/* Código de Operações*/
#define NOP   0
//aritméticas
#define ADD     1
#define SUB     2
#define MULT    3
#define DIV     4
#define ADDI    5
#define SUBI    6
#define RSUBI   7
#define MULTI   8
#define DIVI    9
#define RDIVI   10
//shifts
#define LSHIFT    11
#define LSHIFTI   12
#define RSHIFT    13
#define RSHIFTI   14
//lógicas
#define AND   15
#define ANDI  16
#define OR    17
#define ORI   18
#define XOR   19
#define XORI  20
//carga de registradores
#define LOADI 21
//operações sobre a memória
#define LOAD      22
#define LOADAI    23
#define LOADA0    24
#define CLOAD     25
#define CLOADAI   26
#define CLOADA0   27
#define STORE     28
#define STOREAI   29
#define STOREA0   30
#define CSTORE    31
#define CSTOREAI  32
#define CSTOREA0  33
//cópia entre registradores
#define I2I   34
#define C2C   35
#define C2I   36
#define I2C   37
//saltos
#define JUMPI   38
#define JUMP    39
//fluxo de controle
#define CBR     40
#define CMP_LT  41
#define CMP_LE  42
#define CMP_EQ  43
#define CMP_GE  44
#define CMP_GT  45
#define CMP_NE  46

typedef struct iloc{
    char* label;        // Rótulo da instrução
    operation_t* ops;   // Lista de operações
    struct iloc *next, *prev;   //Lista de instruções
}iloc_t;

typedef struct operation{
    int opcode;      // Código da operação
    int type;        // sendo NORMAL ou FLUX_CONTROL
    operand_t* op_sources;    // Operadores fonte
    operand_t* op_targets;    // Operadores alvo
    struct operation *next;   // Próxima operação
}operation_t;

typedef struct operand{
    int type;       // sendo REGISTER, NUMBER ou LABEL
    char* lexeme;
    struct operand *first, *last;
    struct operand *next, *prev;
}operand_t;
