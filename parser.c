/*
  parser.c
  This module contains the firstpass() function as well as supporting functions
  to analyze tokens in the records.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "instructions.h"
#include "directives.h"
#include "symboltable.h"
#include "errors.h"

/*
  firstpass() reads the input assembly file and calls the parser to tokenize
  and analyze each record separetely.
*/
void firstpass(FILE* fp){
  char instring[LINE_LEN];

  //Initializing Globals
  flag_end_of_program = FALSE; // Used to end assembly when END is encountered
  flag_max_lc = FALSE;
  line_number = 1;             // Numbers in this case = Readable
  errors = 0;

  fprintf(fout,"\n--------------    Input Records    --------------\n");

  while((fgets(instring, LINE_LEN, fp) != NULL) && flag_end_of_program == FALSE){
    /* Truncate lines too long */
    instring[LINE_LEN-1] = NUL;
    #ifdef debug
    printf("\n------Record %d------: %s", line_number, instring);
    #endif
    fprintf(fout, "\n------Record %d------: %s", line_number, instring);
    /* Completely skip record if it starts with a comment or it's a blank */
    if((instring[0] != '\r') && (instring[0] != ';') && (instring[0] != '\n')){
        parse_record(instring);
    }
    line_number++; // Line number is incremented regardless of blank or not
  }
}

void parse_record(char* line){
  char *token;
  char *string;
  char *strptr;
  unsigned short safeguard = 0;
  struct firsttoken tokinfo;

  string = storeline(line);        //Store the record for error printing purposes

  token = strtok(line, " \t\r\n"); // Isolate the token for analysis

  while(!isalnum(*string)){       // Go to first legible alphanumeric
    *string++;
    if(safeguard++ == LINE_LEN){
      break;
    }
  }

  strptr = storeline(string + strlen(token)); // Store the record minus the
                                              // 1st token for analysis in
                                              // subsequent parsers
  if(token != NULL){
    /* Specify 1st token type in record (INST, DIR, LABEL, ERROR) */
    tokinfo = sort(token);
    switch (tokinfo.type) {
      case INST:
      analyzeinstruction(strptr, tokinfo);
      break;
      case DIR:
      analyzedirective(strptr, tokinfo);
      break;
      case LABEL:
      analyzelabel(strptr, token);
      break;
      case COMMENT:
      return;
      break;
      default:
      error_count("ERROR: Unclassifiable first token >>%s<< in line", token);
      break;
    }
    token = strtok(NULL, " \t\n");
  }
  #ifdef debug
  printf("LC after record %d\n", LC);
  #endif /* debug */
  return;
}

/*
  This function sorts the first token of the record. The first token can be an
  instruction, a directive or a label. Anything else results in an error,
  indicated here as type UNKNOWN. It gets called again when the first token is
  a LABEL to determine the type of the following token.
*/
struct firsttoken sort(char *token){
  struct firsttoken result;
  struct symbol_entry* entry;
  result.instptr = NULL;
  result.dirptr = NULL;
  result.type = UNKNOWN;

  if(result.instptr = get_inst(token)){       //Check INST list
    result.type = INST;
    return result;
  }
  else if(result.dirptr = get_dir(token)){    //Check DIR list
    result.type = DIR;
    return result;
  }
  else if(is_label(token)){                   //Check Label rules
    result.type = LABEL;
    if(entry = get_entry(token)){
      if(entry->type == REGTYPE){       //Ensures that the "valid" label is
        result.type = UNKNOWN;          //not a register. If it's a reg
      }                                 //return UNKNOWN to show error.
    }
    return result;
  }
  else if(*token = ';'){                //For the cases in which the comment
    result.type = COMMENT;              //starts further into the line
  }
  return result;                        //If none of the above return UNKNOWN
}

/*
  If a label is encountered in parse_record, this function gets called to deal
  with the remaining record. The following tokens in the record can be either
  an instruction, a directive or a comment/null. Anything else is an error
*/
void analyzelabel(char* line, char* token){
  char* nexttoken;
  char* string;
  char* strptr;
  struct firsttoken result;

  #ifdef debug
  printf("FOUND LABEL >>%s<<\n", token);
  #endif /* debug */

  global = token;                       // Global string saving the label token
  string = storeline(line);
  nexttoken = strtok(line, " \t\r\n");

  if(nexttoken != NULL && *nexttoken != ';' && nexttoken[0] != '\r'){

    while(!isalnum(*string)){       // Go to first legible char
      *string++;
    }

    //Points to the start of the next token for the analysis functions

    strptr = storeline(string + strlen(nexttoken));

    #ifdef debug
    printf("TOKEN AFTER LABEL IS >>%s<<\n", nexttoken);
    printf("SENDING TO ANALYZERS: >>%s<<\n", strptr);
    #endif /* debug */

    result = sort(nexttoken);
    flag_first_token_label = TRUE; // indicator used for storing in symtbl
    switch (result.type) {
      case INST:
      analyzeinstruction(strptr, result);
      break;
      case DIR:
      analyzedirective(strptr, result);
      break;
      default:
      error_count("ERROR: Invalid token after label:", nexttoken);
      break;
    }
    flag_first_token_label = FALSE;
  }
  else{                                      // Nothing follows the label
    #ifdef debug
    printf("SOLO LABEL >>%s<<\n", token);
    #endif
    if(!get_entry(token)){            // If label unique, add it with LC
      add_entry(token, LC, UNKTYPE);
    }
  }

  global = NULL; //Reset the global label for further usage
}

/*
  is_label checks to see if a token follows all the rules for a label to be
  considered valid. First character must be alphabetic and the following can
  be alphanumeric. The maximum length of a label cannot exceed 32 characters.
*/

unsigned char is_label(char* token){
  int short i = 1;
  unsigned char res = FALSE;

  if(isalpha(*token)){
    while(i < (strlen(token))){
      res = (!isalnum(token[i]) || i == MAX_NAME_LEN) ? FALSE : TRUE;
      if(res == FALSE){     // Breaks loop once an error is detected
        if(i == MAX_NAME_LEN){
          error_count("ERROR: Label is too long:", token);
        }
        return res;
      }
      i++;
    }

    if(get_inst(token)){  // Labels cannot have instruction names
      res = FALSE;
      #ifdef debug
      printf("LABEL >>%s<< HAS INSTRUCTION NAME\n", token);
      #endif /* debug */
    }

    if(get_dir(token)){ // Lables cannot have directive names
      res = FALSE;
      #ifdef debug
      printf("LABEL >>%s<< HAS DIRECTIVE NAME\n", token);
      #endif /* debug */
    }
  }
  return res;
}

/*
  As per the function's name, it returns TRUE if the characters passed to it
  comprise a number. Returns false otherwise.
*/
int is_number(char* line){
  char* source;
  char* token;
  int res = FALSE;
  unsigned char flag_negative = FALSE;

  if(!line){
    return res;
  }

  token = strtok(line, " \t\r\n");
  #ifdef debug
  printf("Testing if >>%s<< is a number\n", token);
  #endif /* debug */

  if(token[0] == '-'){
    #ifdef debug
    printf("Negative Number\n");
    #endif /* debug */
    flag_negative = TRUE;
    source = &token[1];
  }
  else{
    source = token;
  }

  switch (source[0]) {
    case '$':
    *source++;
    if(cyclenumber(source, HEXADECIMAL)){ //cycle through chars expecting hex
      res = strtol(source, NULL, 16);     //return as int
    }
    break;
    case '0':
    if (source[1] == 'x' || source[1] == 'X') {
      if(cyclenumber(source, HEXADECIMAL)){
        res = strtol(source, NULL, 16);
      }
    }
    break;
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    if(cyclenumber(source, DECIMAL)){ //cycle through chars expecting decimal
      res = strtol(source, NULL, 10); //return as int
    }
    break;
    default:
    #ifdef debug
    printf("%c not a valid number\n", source[0]);
    #endif
    break;
  }

  if(flag_negative){
    res = -res;         // return the number as a negative
  }

  return res;
}

/*
  Small function to create a copy of a string. This is generally used in cases
  where the tokenize function is used which will affect the integrity of the
  string.
*/
char* storeline(char* line){
  unsigned short i = 0;
  unsigned short length;

  length = strlen(line);
  char *string = malloc(sizeof(char)*length + 1);  // Plus 1 for NULL character

  while(line[i] != NUL){
    string[i] = line[i];
    i++;
  }
  return string;
}

/*
  As per the function name it simply cycles through the chars of an array to
  check whether the array is indeed a number, be it hex or decimal.
*/
int cyclenumber(char* number, int type){
  unsigned short i = 0;
  unsigned short res = TRUE;
  unsigned short length;

  switch (type) {
    case HEXADECIMAL:
    #ifdef debug
    printf("Checking for HEX\n");
    #endif /* debug */
    number += 2;
    length = strlen(number);
    while(i < length){
      if(!(((number[i] >= '0') && (number[i] <= '9')) ||
         ((number[i] >= 'a') && (number[i] <= 'f')) ||
         ((number[i] >= 'A') && (number[i] <= 'F')))) {
        #ifdef debug
        printf("%s is not a HEX\n", number);
        #endif /* debug */
        return res = FALSE;
      }
      i++;
    }
    break;

    case DECIMAL:
    #ifdef debug
    printf("Checking for DECIMAL\n");
    #endif /* debug */
    length = strlen(number);
    while (i < length){
      if(!isdigit(number[i])){
        #ifdef debug
        printf("%s is not a DECIMAL\n", number);
        #endif /* debug */
        return res = FALSE;
      }
      i++;
    }
    break;
  }
  return res;
}
