/*
  errors.c
  Module currently contains only a simple check for existing errors or undeclared
  labels in the symbol table as well as an error counter/printer. Unused code
  included for future implementation.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: May 29, 2016
*/

#include <stdio.h>
#include "assembler.h"
#include "errors.h"
#include "parser.h"
#include "symboltable.h"

struct error_el *error_list_head = NULL;
struct error_el *error_list_tail = NULL;

// Checks whether they're unknowns in the symbol table, or existing errors.
unsigned char secondpasscheck(void){
  unsigned char res = FALSE;

  if(!checkunknown() && errors == FALSE){
    #ifdef debug
    printf("Clear for Second Pass!\n");
    #endif
    return res = TRUE;
  }
  else{
    fprintf(fout, "MESSAGE: %d Errors in the Assembler's First Pass\n", errors);
    return res;
  }
}

/*
  This function is used to increment the error count and print out diagnostics.
  If enabled it will print diagnostics into the terminal, which is useful since
  in case of a crash none will be printed into the file.
*/

void error_count(char* error_message, char* operand){
  errors++;
  if(error_message){
    if(operand){
      fprintf(fout, "%s %s\n", error_message, operand);
      #ifdef debug
      printf("%s %s\n", error_message, operand);
      #endif
    }
    else{
      fprintf(fout, "%s\n", error_message);
      #ifdef debug
      printf("%s\n", error_message);
      #endif
    }
  }
  return;
}
