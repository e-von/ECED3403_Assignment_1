#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

/*
  symboltable.h
  Header file for symboltable.c. Structures provided by the ECED3403 course at
  Dalhousie University.

  Coder: Elias Vonapartis, with code from ECED3403
  Release Date: May 28, 2016
  Latest Updates: None
*/

#define MAX_LC 65535
#define MAX_NAME_LEN 32

#define CONGEN(x) ((x == -1)||(x == 0)||(x == 1)||(x == 2)||(x == 4)||(x == 8))

enum SYMBOLTYPES {REGTYPE, LBLTYPE, UNKTYPE};

struct symbol_entry{
  char name[MAX_NAME_LEN];  /* Name of Symbol */
  int value;                /* LC, Register   */
  enum SYMBOLTYPES type;    /* Type (Register)*/
  struct symbol_entry *next;/* Next Entry     */
};

/* Function Declarations */
void init_symboltable(void);
void print_symboltable(void);
void add_entry(char* , int , enum SYMBOLTYPES);
struct symbol_entry* get_entry(char* );
void update_entry(char* , int , enum SYMBOLTYPES);
void delete_entry(void);
void clear_table(void);
unsigned char checkunknown(void);

#endif /* SYMBOLTABLE_H */
