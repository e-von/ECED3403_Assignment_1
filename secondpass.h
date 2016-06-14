#ifndef SECONDPASS_H
#define SECONDPASS_H

/*
  secondpass.h
  Header file for secondpass.c

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

#include "records.h"

#define debug2

#define MAX_POS_OFFSET  1024    // As per the inst manual
#define MAX_NEG_OFFSET  -1022
#define PC_REG    0
#define SR_REG    2
#define CG_REG    3
#define WORDSIZE  0
#define BYTESIZE  1

#define WORDINC       2
#define DOUBLEWORDINC 4
#define FOURWORDINC   6

#define half_value(x) (x >> 1)

/* Declarations */
void secondpass(void);
void type1_inst(struct record_entry* );
void type2_inst(struct record_entry* );
void type3_inst(struct record_entry* );
void numval_extractor(enum ADDR_MODE , char* , int* , unsigned char* , unsigned char* , int);
void opcode_printer(unsigned short, int, int, unsigned char);

#endif /* SECONDPASS_H */
