/*
  symboltable.c
  Code regarding the symbol table structure and its manipulation is contained
  in this module. Based on code provided by the ECED3403 course at Dalhousie
  University.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "symboltable.h"
#include "parser.h"
#include "errors.h"

/* Symbol table list pointer - iunitially list is empty */
struct symbol_entry *entry = NULL;

void init_symboltable(void){
  /*
   - Initialize the symbol table with register values
   - Treat registers as labels with type of REGTYPE
  */
  add_entry("R0",  0,  REGTYPE);
  add_entry("PC",  0,  REGTYPE); /* Alias for R0 */
  add_entry("R1",  1,  REGTYPE);
  add_entry("SP",  1,  REGTYPE); /* Alias for R1 */
  add_entry("R2",  2,  REGTYPE);
  add_entry("SR",  2,  REGTYPE); /* Alias for R2 */
  add_entry("R3",  3,  REGTYPE);
  add_entry("R4",  4,  REGTYPE);
  add_entry("R5",  5,  REGTYPE);
  add_entry("R6",  6,  REGTYPE);
  add_entry("R7",  7,  REGTYPE);
  add_entry("R8",  8,  REGTYPE);
  add_entry("R9",  9,  REGTYPE);
  add_entry("R10", 10, REGTYPE);
  add_entry("R11", 11, REGTYPE);
  add_entry("R12", 12, REGTYPE);
  add_entry("R13", 13, REGTYPE);
  add_entry("R14", 14, REGTYPE);
  add_entry("R15", 15, REGTYPE);
  return;
}

/*
  Assumes the entry is unique (caller must do a get_entry() beforehand
  Adds new entry to end of list
*/
void add_entry(char *name, int value, enum SYMBOLTYPES type){
  struct symbol_entry *newentry;
  newentry = malloc(sizeof(struct symbol_entry));
  if(value <= MAX_LC){
    strcpy(newentry -> name, name);
    newentry -> value = value;
    newentry -> type = type;
    newentry -> next = entry;
    entry = newentry;
  }
  else{
  error_count("ERROR: Value added to table is out of bounds", NULL);
  }
  return;
}

/*
  This function searches the symbol table for a label name.
  Returns the entry of NULL if none matching.
*/
struct symbol_entry *get_entry(char *name){
  struct symbol_entry *stptr = entry;

  if(name){
    while (stptr){
      if (strcmp(stptr -> name, name) == 0){  //Case snstv string comparison
        return stptr;
      }
      else{
        stptr = stptr -> next;
      }
    }
  }
  return NULL;  /* Not in symtbl */
}

/*
  This function updates entries in the Symbol Table, specifically their values
  and types. Names cannot be changed.
*/
void update_entry(char* name, int value, enum SYMBOLTYPES type){
  struct symbol_entry* updatentry = get_entry(name);

  if((updatentry) && (value <= MAX_LC)){
    updatentry->value = value;
    updatentry->type = type;
  }
  else{
    printf("INTERNAL ERROR: Symbol Table update error.\n");
  }
}

/*
  This function prints the symbol table in the terminal and to the diagnostics
  file.
*/
void print_symboltable(void){
  struct symbol_entry* printentry = entry;
  if(printentry != NULL){
    printf("\n--------------    Symbol Table    --------------\n");
    fprintf(fout, "\n--------------    Symbol Table    --------------\n");
    while(printentry != NULL){
      printf("Name: %s \t Value: %d \t Type: ", printentry -> name,
              printentry -> value);
      fprintf(fout, "Name: %s\t\tValue: %d\t\tType: ", printentry -> name,
                      printentry -> value);
      switch (printentry->type) {
        case 0:
        printf("Register\n");
        fprintf(fout, "Register\n");
        break;
        case 1:
        printf("Label\n");
        fprintf(fout, "Label\n");
        break;
        case 2:
        printf("Unknown\n");
        fprintf(fout, "Unknown\n");
        break;
        default:
        printf("Boys we have a problem\n");
        fprintf(fout, "Illegal Type\n");
        break;
      }
      printentry = printentry -> next;
    }
  }
  else{
    printf("The Symbol Table has no entries.\n");
    fprintf(fout, "The Symbol Table has no entries.\n");
  }
}

/* This function deletes the latest entry in the symbol table */
void delete_entry(void){
  struct symbol_entry *oldentry = entry;
  free(oldentry);         // Release memory
  entry = entry -> next;  // Update to the next existing node
}

/* This function clears the whole symbol table */
void clear_table(void){
  struct symbol_entry *oldentry = entry;
  while(oldentry){
    delete_entry();
    oldentry = entry;     // Update to the next existing node
  }
}

unsigned char checkunknown(void){
  struct symbol_entry *stptr = entry;
  unsigned char res = FALSE;

  while(stptr != NULL){
    if(stptr->type == UNKTYPE){
      printf("ERROR: Undeclared Label %s\n", stptr->name);
      fprintf(fout, "ERROR: Undeclared Label %s\n", stptr->name);
      res = TRUE;
    }
    stptr = stptr->next;
  }
  return res;
}
