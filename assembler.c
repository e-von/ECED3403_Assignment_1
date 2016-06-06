/*
  assembler.c
  MSP430 main module for two pass assembler. This module calls opens the .asm
  text file and calls the first pass to build the symbol table. If there are no
  errors the second pass function is called.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: May 29, 2016
*/

#include <stdio.h>
#include <stdlib.h>
#include "assembler.h"
#include "parser.h"
#include "symboltable.h"
#include "errors.h"
#include "emit.h"
#include "records.h"
#include "secondpass.h"

int main(int argc, char const *argv[]) {
  /* The following ensures file accessibility */
  if (argc != 2){
    printf("Format: ./assembler 'filename'\n");
    exit(0);
  }

  if((fp = fopen(argv[1], "r")) == NULL){
    printf("File %s could not be opened\n", argv[1]);
    exit(0);
  }

  initialize();
  firstpass(fp);
  print_symboltable();

  if(secondpasscheck()){
    #ifdef debug
    printf("\n--------------    Starting Second Pass    --------------\n\n");
    #endif
    print_records();
    secondpass();
  }
  terminate();
  print_records();
  exit(0);
}

void initialize(void){
  /* Open the output file for diagnostics */
  fout = fopen("diagnostics.lis", "w");

  /* Initialize the symbol table */
  init_symboltable();
}

void terminate(void){
  clear_table();
  clear_records();
  fclose(fp);
  fclose(fout);
  if(srec){
    fclose(srec);
  }
}
