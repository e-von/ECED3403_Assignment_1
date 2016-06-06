#ifndef RECORDS_H
#define RECORDS_H

/*
  records.h
  Header file for records.c. Contains the structure for the double-linked list
  containing all the required information for the second pass code generation.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

#include "assembler.h"
#include "parser.h"

#define ORG2    0
#define BYTE2   1
#define WORD2   2
#define STRING2 3
#define BSS2    4

struct record_entry *head;

struct record_entry{
  /* Common for all instructions */
  unsigned int line;
  unsigned int LC;
  char* inst;
  enum INST_TYPE type;

  /* Operand Instructions */
  char* src_op;
  char* dst_op;
  enum ADDR_MODE src_mode;
  enum ADDR_MODE dst_mode;

  /* Jump Instructions */
  unsigned short offset;

  /* Storing for ASCII, WORD, BYTE, ORG, BSS */
  char* string;
  int value;
  unsigned char wbosb;

  /* Next Node */
  struct record_entry* next;
  struct record_entry* prev;
};

/* Declarations */
void add_inst_record(char*,enum INST_TYPE,char*,char*,enum ADDR_MODE,enum ADDR_MODE);
void double_linking(struct record_entry* , struct record_entry* );
void add_jump_record(char* , enum INST_TYPE, char* );
void add_string_record(char* , unsigned short );
void add_data_record(int , unsigned char );
void add_org_record(unsigned short);
void add_bss_record(unsigned short);
void print_records(void);
void clear_records(void);

#endif /* RECORDS_H */
