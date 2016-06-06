#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

/*
  instructions.h
  Header file for instructions.c. Inst structure provided by the ECED3403 class
  of Dalhousie University.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

#include "assembler.h"
#include "parser.h"

/* Definitions */
#define LASTINST "ZZZ"
#define MAX_BIT_VAL 65535
#define REG_SIZE 3
#define WORD_INC 2

unsigned char flag_const_immediate;

struct inst_el{
  char *inst;
  unsigned short opcode;
  enum INST_TYPE type;
  enum BYTE_COMB bw;
};

/* External Functions */
struct inst_el* get_inst(char*);
void analyzeinstruction(char* , struct firsttoken);
void operand_parser(char* , enum INST_TYPE, char* );
unsigned char tokenize_operands(char* , enum INST_TYPE , char** , char** );
void checkabsolute(char* , enum ADDR_MODE* );
void checkindirect(char* , enum ADDR_MODE* );
void checkimmediate(char* , enum ADDR_MODE* );
void checkdefault(char* , enum ADDR_MODE* );
void checkjump(char* , char* );
unsigned char checkjunkrecord(char* );
void incrementLC(enum ADDR_MODE* , enum ADDR_MODE* );
#endif /* INSTRUCTIONS_H */
