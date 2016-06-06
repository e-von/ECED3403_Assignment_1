/*
  directives.c
  MSP430 directive module. This module contains the directive list of the
  chip and a number of functions which can analyze records containing these
  directives.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: May 29, 2016  - Added some error messages
                                - Added some safeguards for null tokens
                  May 31, 2016  - Fixed invalid usage of negatives
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "parser.h"
#include "directives.h"
#include "symboltable.h"
#include "records.h"
#include "errors.h"

// List of MSP430 directives, DIR / TYPE / Enumerated equivalent
// Enumeration was added for easy application of switch cases
struct dir_el dir_list[] = {
  {"ALIGN", NONE, ALIGN},
  {"ASCII", DOUBLE, STRING},
  {"BES", DOUBLE, BES},
  {"BSS", DOUBLE, BSS},
  {"BYTE", SINGLE, DBYTE},
  {"END", SINGLE, END},
  {"EQU", DOUBLE, EQU},
  {"ORG", DOUBLE, ORG},
  {"WORD", DOUBLE, DWORD},
  {LASTDIR, NONE, DLASTDIR} /* End of list */
};

// Linear search through dirlist, returns pointer to entry or NULL
struct dir_el *get_dir(char *dir){
  struct dir_el *ptr;

  ptr = &dir_list[0];

  while(strcasecmp(ptr -> dir, LASTDIR) != 0){
    if(strcasecmp(ptr -> dir, dir) == 0){
      return ptr;
    }
    else{
      ptr++;
    }
  }
  return NULL;
}

// Switch case which calls functions unique for each directive, which perform
// further analysis on the passed record.
void analyzedirective(char* line, struct firsttoken srctoken){
  char *token;
  char *copy;

  copy = storeline(line);
  token = strtok(line, " \t\r\n");

  #ifdef debug
  printf("Directive from source : >>%s<<\n", srctoken.dirptr->dir);
  printf("The directive token is: >>%s<<\n", token);
  #endif /* debug */

  switch (srctoken.dirptr->entry){
    case ALIGN:
    #ifdef debug
    printf("CASE: ALIGN\n");
    #endif /* debug */
    align(token);
    break;
    case BSS:
    #ifdef debug
    printf("CASE: BSS\n");
    #endif /* debug */
    bss(token, STARTING);
    break;
    case DBYTE:
    #ifdef debug
    printf("CASE: BYTE\n");
    #endif /* debug */
    byte(token);
    break;
    case END:
    #ifdef debug
    printf("CASE: END\n");
    #endif /* debug */
    end(token);
    break;
    case EQU:
    #ifdef debug
    printf("CASE: EQU\n");
    #endif /* debug */
    equ(token);
    break;
    case ORG:
    #ifdef debug
    printf("CASE: ORG\n");
    #endif /* debug */
    origin(token);
    break;
    case STRING:
    #ifdef debug
    printf("CASE: STRING\n");
    #endif /* debug */
    string(copy);
    break;
    case DWORD:
    #ifdef debug
    printf("CASE: WORD\n");
    #endif /* debug */
    word(token);
    break;
    default:
    fprintf(fout, "ERROR: Something has broken in the directive sorter.\n");
    break;
  }
  return;
}
//If odd increase LC by 1
void align(char* record){
  if(LC%2){
    adjustLC(HALFWORD, INCREMENT);
  }
}

// Increases LC by stated value in .asm.
void bss(char* record, unsigned char mode){
  int bsval;
  struct symbol_entry* entry;

  if(!record){                    //Safeguard for missing value
    error_count("ERROR: BSS value is missing", NULL);
    return;
  }

  if(bsval = is_number(record)){  // Check if bss equated to valid number
    if(bsval < MAX_LC && bsval > 0){
      add_bss_record(bsval);
      adjustLC(bsval, INCREMENT);
    }
    else{
      error_count("ERROR: BSS value is too large or negative:", record);
      return;
    }
  }
  // Check if block byte number is through an equated label
  else if(entry = get_entry(record)){
    if(entry->type != (REGTYPE && UNKTYPE)){
      if(entry->value < 0 || entry->value > MAX_LC){
        error_count("ERROR: BSS value is too large or negative", NULL);
        return;
      }
      add_bss_record(entry->value);
      adjustLC(entry->value, INCREMENT);
    }
    else{
      error_count("ERROR: BSS value is either a REG or UNKOWN.", NULL);
      return;
    }
  }
  else{
    error_count("ERROR: BSS value is invalid or missing.", NULL);
    return;
  }

  if(flag_first_token_label){         // BSS valid, add label if there is one
    if(get_entry(global)){            // Subtract LC since it has been added
      update_entry(global, (LC - bsval), LBLTYPE);
    }
    else{
      add_entry(global, (LC - bsval), LBLTYPE);
    }
  }
}

void byte(char* record){
  int byteval;

  if(!record){                    //Safeguard for missing value
    error_count("ERROR: Byte Value is missing", NULL);
    return;
  }

  byteval = is_number(record);
  printf("BYTEVAL IS %d\n", byteval);

  if(byteval <= MAXBYTEVAL && byteval > 0){ //Check if byte val is indeed a byte
    if(flag_first_token_label){             //Add label if there is one
      if(get_entry(global)){
        update_entry(global, LC, LBLTYPE);
      }
      else{
        add_entry(global, LC, LBLTYPE);
      }
    }
    add_data_record(byteval, BYTE);
    adjustLC(HALFWORD, INCREMENT);
  }
  else{
    error_count("ERROR: Valid byte values are between 0 & 255 (b10).", NULL);
  }
}

void end(char* value){
  struct symbol_entry* temp;

  if(flag_first_token_label){
    error_count("ERROR: End directive cannot have a label:", global);
  }
  //Don't return. End the reading at this directive. Due to error count second
  //pass cannot happen.

  //global indicator that is read by parse_record to end reading the input file
  flag_end_of_program = TRUE;
  if(value){                      //End can be followed by the starting address
    if(temp = get_entry(value)){    //If there is one, add it to global storage
      start_address = temp->value;  //for s9 record
    }
    else{
      start_address = is_number(value);
    }
  }
  else{
    start_address = START_ADDR;
  }
}

void equ(char* value){
  int intval;
  struct symbol_entry* entry;

  // Safeguard for usage of EQU without a label
  if(global == NULL){
    error_count("ERROR: Missing Label for Equate.", NULL);
    return;
  }
   //Safeguard for missing value
  if(!value){
  error_count("ERROR: Missing Value for Equate", NULL);
    return;
  }

  if(entry = get_entry(global)){
    if(entry->type != REGTYPE){
      if(intval = is_number(value)){
        update_entry(global, intval, LBLTYPE);
      }
      else{
        error_count("ERROR: Not Equating a Valid Number.", NULL);
      }
    }
    else{
      error_count("ERROR: Cannot Equate Registers.", NULL);
    }
  }

  // At this point we know label validity, so we just add the entry to the table
  else if(intval = is_number(value)){
    add_entry(global, intval, LBLTYPE);
  }
  else{
    error_count("ERROR: Invalid or missing equate value.", NULL);
  }
}

void origin(char* record){
  struct symbol_entry *entry;
  int value;

  if(!record){
    error_count("ERROR: Missing Value for Origin", NULL);
    return;
  }

  if(entry = get_entry(record)){
    #ifdef debug
    printf("ORIGIN with existing LABEL\n");
    #endif
    if(entry->value < 0){
      error_count("ERROR: Cannot have a negative origin:", record);
      return;
    }

    // No need to check for maximum value, since it cannot be entered into
    // symbol table.

    add_org_record(entry->value);
    adjustLC(entry->value, EQUATE);
  }
  else if(value = is_number(record)){
    #ifdef debug
    printf("ORIGIN with a NUMBER\n");
    #endif
    if(value >= 0 && value <= MAX_LC){
      add_org_record(value);    // add entry to second pass linked-list
      adjustLC(value, EQUATE);
    }
    else{
      error_count("ERROR: Invalid Origin. Valid values: 0 =< x =< 65535", record);
    }
  }
  else{
    error_count("ERROR: Non Valid ORIGIN.", NULL);
  }
}

void string(char* record){
  char* ptr;
  char* content;
  unsigned short i;

  content = (char* )(malloc(sizeof(short)));
  ptr = (char* )(malloc(sizeof(short)));

  if(ptr = strstr(record, "\"")){                     //Find the opening quotes
    *ptr++;
    for(i = 0; ptr[i] != '"' && i != LINE_LEN; i++){  //Store till closing quotes
      content[i] = ptr[i];
    }

    #ifdef debug
    printf("Contents of string >>%s<<\n", content);
    #endif

    if(ptr[i] == '"'){
      if(flag_first_token_label){
        if(get_entry(global)){
          update_entry(global, LC, LBLTYPE);
        }
        else{
          add_entry(global, LC, LBLTYPE); // Add label if there is one
        }
      }
      add_string_record(content, i); // Add entry to second pass linked-list
      adjustLC(i, INCREMENT);
    }
    else{
      error_count("ERROR: String too long or missing closing quotes.", NULL);
    }
  }
  else{
    error_count("ERROR: String must be enclosed in quotation marks.", NULL);
  }
}

void word(char* record){
  struct symbol_entry* entry;
  int wordval;

  if(wordval = is_number(record)){
    if(wordval <= MAXWORDVAL && wordval >= 0){
      add_data_record(wordval, WORD); // Add entry to second pass linked-list
    }
    else{
      error_count("ERROR: Valid Word values are between 0 & FFFF(h)", record);
      return;
    }
  }

  if(flag_first_token_label){
    if(entry = get_entry(global)){
      if(entry->type != REGTYPE){
        update_entry(global, LC, LBLTYPE);
      }
      else{
        error_count("ERROR: Cannot assign word to register.", NULL);
        return;
      }
    }
    else{
      add_entry(global, LC, LBLTYPE);
    }
  }

  adjustLC(WORD, INCREMENT);
}

/*
  Function to increment the LC when encountering a directive. Written to avoid
  repeated code. The mode bit indicates whether we wish to increment the LC (1)
  or assign it a value (0).
*/

void adjustLC(unsigned short value, unsigned char mode){
  switch (mode) {
    case EQUATE:
    if(value <= MAX_LC){
      LC = value;
    }
    else{
      error_count("ERROR: Assigned LC exceeds limits.", NULL);
    }
    break;

    case INCREMENT:
    if((LC + value) <= MAX_LC){
      LC += value;
    }
    else{
      error_count("ERROR: LC to exceed limits.", NULL);
    }
    break;
  }
}
