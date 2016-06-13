/*
  instructions.c
  MSP430 instruction module. This module contains the instruction list of the
  chip and a number of functions which can analyze records containing these
  instructions.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: May 29, 2016  - Fixed the RETI Opcode
                  June 5, 2016  - Fixed the return value of tokenize_operands
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "instructions.h"
#include "assembler.h"
#include "parser.h"
#include "symboltable.h"
#include "records.h"
#include "errors.h"

#define LISTLENGTH 61

struct inst_el inst_list[] = {
  /* Mnemonic - Opcode - Operand - Size */
  {"ADD", 0x5, DOUBLE, WORD},
  {"ADD.B", 0x5, DOUBLE, BYTE},
  {"ADD.W", 0x5, DOUBLE, WORD},

  {"ADDC", 0x6, DOUBLE, WORD},
  {"ADDC.B", 0x6, DOUBLE, BYTE},
  {"ADDC.W", 0x6, DOUBLE, WORD},

  {"AND", 0xF, DOUBLE, WORD},
  {"AND.B", 0xF, DOUBLE, BYTE},
  {"AND.W", 0xF, DOUBLE, WORD},

  {"BIC", 0xC, DOUBLE, WORD},
  {"BIC.B", 0xC, DOUBLE, BYTE},
  {"BIC.W", 0xC, DOUBLE, WORD},

  {"BIS", 0xD, DOUBLE, WORD},
  {"BIS.B", 0xD, DOUBLE, BYTE},
  {"BIS.W", 0xD, DOUBLE, WORD},

  {"BIT", 0xB, DOUBLE, WORD},
  {"BIT.B", 0xB, DOUBLE, BYTE},
  {"BIT.W", 0xB, DOUBLE, WORD},

  {"CALL", 0x25, SINGLE, WORD},

  {"CMP", 0x9, DOUBLE, WORD},
  {"CMP.B", 0x9, DOUBLE, BYTE},
  {"CMP.W", 0x9, DOUBLE, WORD},

  {"DADD", 0xA, DOUBLE, WORD},
  {"DADD.B", 0xA, DOUBLE, BYTE},
  {"DADD.W", 0xA, DOUBLE, WORD},

  {"JC", 0xB, JUMP, OFFSET},
  {"JEQ", 0x9, JUMP, OFFSET},
  {"JGE", 0xD, JUMP, OFFSET},
  {"JHS", 0xB, JUMP, OFFSET},
  {"JL", 0xE, JUMP, OFFSET},
  {"JLO", 0xA, JUMP, OFFSET},
  {"JMP", 0xF, JUMP, OFFSET},
  {"JN", 0xC, JUMP, OFFSET},
  {"JNC", 0xA, JUMP, OFFSET},
  {"JNE", 0x8, JUMP, OFFSET},
  {"JNZ", 0x8, JUMP, OFFSET},
  {"JZ", 0x9, JUMP, OFFSET},

  {"MOV", 0x4, DOUBLE, WORD},
  {"MOV.B", 0x4, DOUBLE, BYTE},
  {"MOV.W", 0x4, DOUBLE, WORD},

  {"PUSH", 0x24, SINGLE, WORD},
  {"PUSH.B", 0x24, SINGLE, BYTE},
  {"PUSH.W", 0x24, SINGLE, WORD},

  {"RETI", 0x1300, NONE, WORD},

  {"RRA", 0x22, SINGLE, WORD},
  {"RRA.B", 0x22, SINGLE, BYTE},
  {"RRA.W", 0x22, SINGLE, WORD},

  {"RRC", 0x20, SINGLE, WORD},
  {"RRC.B", 0x20, SINGLE, BYTE},
  {"RRC.W", 0x20, SINGLE, WORD},

  {"SUB", 0x8, DOUBLE, WORD},
  {"SUB.B", 0x8, DOUBLE, BYTE},
  {"SUB.W", 0x8, DOUBLE, WORD},

  {"SUBC", 0x7, DOUBLE, WORD},
  {"SUBC.B", 0x7, DOUBLE, BYTE},
  {"SUBC.W", 0x7, DOUBLE, WORD},

  {"SWPB", 0x21, SINGLE, WORD},

  {"SXT", 0x23, SINGLE, WORD},

  {"XOR", 0xE, DOUBLE, WORD},
  {"XOR.B", 0xE, DOUBLE, BYTE},
  {"XOR.W", 0xE, DOUBLE, WORD}
};

/*
  Binary search for instruction list. By adjusting the global LISTLENGTH it
  self-adjusts to find the new middle, whether it's an odd or even length.
  Follows the basic binary search principles.
*/

struct inst_el* get_inst(char* inst){
  int middle;
  int old_middle;
  int length;
  int search;

  if((LISTLENGTH)%2){
    middle = (float)LISTLENGTH/2 + 0.5;
  }
  else{
    middle = LISTLENGTH/2;
  }

  length = middle;
  search = strcasecmp(inst, inst_list[middle].inst);

  while(search){
    if(length == 0){
      return NULL;
    }

    old_middle = middle;

    if(search < 0){
      middle = middle - (float)(length)/2 - 0.5;
      length = old_middle - middle - 1;
    }
    else{
      middle = middle + (float)(length)/2 + 0.5;
      length = middle - old_middle - 1;
    }
    search = strcasecmp(inst, inst_list[middle].inst);
  }
  return &inst_list[middle];
}
/*
 //Commenting this out for backup
struct inst_el* get_inst(char* inst){
  struct inst_el *ptr; // Pointer to the list
  ptr = &inst_list[0]; // Initiate at top of inst_list

  // Compare argument to entries in table
  while(strcasecmp(ptr -> inst, LASTINST) != 0){
    if(strcasecmp(ptr -> inst, inst) == 0){
      return ptr;
    }
    else{
      ptr++;
    }
  }
  return NULL;
}
*/
void analyzeinstruction(char* line, struct firsttoken srctoken){
  #ifdef debug
  printf("INST TOKEN >>%s<<\n", srctoken.instptr->inst);
  printf("INST LINE >>%s<<\n", line);
  #endif /* debug */

  /* If the first token was a label add it to the symbol table with the LC */

  if(flag_first_token_label){
    if(!get_entry(global)){
      add_entry(global, LC, LBLTYPE);
    }
    else{
      update_entry(global , LC, LBLTYPE);
    }
  }

  /*
    What I did here. Since double operands may have a space it would be
    inefficient to simply copy the string in case it is a double. So I will send
    the untokenized string to the following functions for them to deal with the
    specific case of JUMP, SINGLE, or DOUBLE instructions.
  */

  switch (srctoken.instptr->type) {
    case NONE:
    #ifdef debug
    printf("INST CASE: NONE\n");
    #endif /* debug */
    if(checkjunkrecord(line)){
      LC += WORD_INC;                 //Increment the LC by 2
      add_inst_record(srctoken.instptr->inst, srctoken.instptr->type, NULL,
                      NULL, -1, -1);
    }
    break;
    case JUMP:
    #ifdef debug
    printf("INST CASE: JUMP\n");
    #endif /* debug */
    checkjump(line, srctoken.instptr->inst); //checks the validity of the record
    break;
    case SINGLE:
    #ifdef debug
    printf("INST CASE: SINGLE\n");
    #endif /* debug */
    operand_parser(line, SINGLE, srctoken.instptr->inst);
    break;
    case DOUBLE:
    #ifdef debug
    printf("INST CASE: DOUBLE\n");
    #endif /* debug */
    operand_parser(line, DOUBLE, srctoken.instptr->inst);
    break;
  }
}
/*
  The operand parser takes the untokenized operands of single and double operand
  instructions, calls on a tokenizer specifically coded for operands and then
  based on the first char of the operand checks for the validity of the potential
  addressing mode.
*/
void operand_parser(char* operand, enum INST_TYPE type, char* inst){
  enum ADDR_MODE src_addr_mode;
  enum ADDR_MODE dst_addr_mode;

  char* source = malloc(sizeof(short));
  char* destination = malloc(sizeof(short));

  flag_const_immediate = FALSE; //ensure it is false prior to parsing operands

  // Returns the tokenized source and/or destination
  if(!tokenize_operands(operand, type, &source, &destination)){
    fprintf(fout, "Tokenize failed. \n");
    return;
  }

  // Check the source operand
  switch (*source) {
    case '&':
    #ifdef debug
    printf("CHECKING SOURCE >>%s<< ABSOLUTE\n", source);
    #endif /* debug */
    checkabsolute(source, &src_addr_mode);
    break;
    case '@':
    #ifdef debug
    printf("CHECKING SOURCE >>%s<< INDIRECT or INDIRECT AUTO\n", source);
    #endif /* debug */
    checkindirect(source, &src_addr_mode);
    break;
    case '#':
    #ifdef debug
    printf("CHECKING SOURCE >>%s<< IMMEDIATE\n", source);
    #endif /* debug */
    checkimmediate(source, &src_addr_mode);
    break;
    default:
    #ifdef debug
    printf("CHECKING SOURCE >>%s<< for RegDir, Indexed, Symbolic\n", source);
    #endif /* debug */
    checkdefault(source, &src_addr_mode);
    break;
  }

  // If the inst was a doubleop check the accepted destination addr modes
  if(type == DOUBLE){
    switch (*destination) {
      case '&':
      #ifdef debug
      printf("CHECKING DESTINATION >>%s<< ABSOLUTE\n", destination);
      #endif /* debug */
      checkabsolute(destination, &dst_addr_mode);
      break;
      case '@':
      case '#':
      error_count("ERROR: Invalid destination addressing mode.", NULL);
      break;
      default:
      #ifdef debug
      printf("CHECKING DESTINATION >>%s<< DEFAULT\n", destination);
      #endif /* debug */
      checkdefault(destination, &dst_addr_mode);
      break;
    }
  }

  if(src_addr_mode == BAD_ADDR_MODE || dst_addr_mode == BAD_ADDR_MODE){
    fprintf(fout, "Cannot Process this Instruction due to Errors.\n");
    return;
  }
  /* Adds necessary information to a linked list for the second pass codegen */
  add_inst_record(inst, type, source, destination, src_addr_mode, dst_addr_mode);
  /* Based on SRC and DST we increment the LC accordingly */
  incrementLC(&src_addr_mode, &dst_addr_mode);
}

unsigned char tokenize_operands(char* operand,enum INST_TYPE type,char** source,
                       char** destination){
  char* token;
  char* src = malloc(sizeof(short));
  char* dst = malloc(sizeof(short));
  char* temp;
  unsigned short i;
  unsigned char res = FALSE;

  *source = NULL;
  *destination = NULL;

  while(*operand == ' '){             // Go to first legible character
    *operand++;
  }
  token = strtok(operand, "\t\r\n");  // Do not search for whitespace since
                                      // double ops may have a space after comma
                                      // this leaves space after the end of the
                                      // double op, which is dealt with later

  if(token == NULL){
    error_count("ERROR: Missing Operand(s) for Instruction.", NULL);
    return res;
  }

  if(type == DOUBLE){
    for(i = 0; token[i] != ','; i++){ // Search for comma, characteristic of dbl
      if (i == MAX_NAME_LEN){         // Label can't be larger than 32
        error_count("ERROR: Operand too long or no ',' for DoubleOp.", NULL);
        return res;                 // To avoid overflow and infinite looping
      }
      src[i] = token[i];              // Copies the chars up to the comma
    }                                 // thus comprising the source operand

    dst = strtok(token + i + 1, " \t\r\n");
    if(dst == NULL){
      error_count("ERROR: Missing Second Operand for Double Instruction.", NULL);
      return res;
    }

    temp = strtok(src, " \t\r\n"); //To remove any potential spaces for extra \t

    #ifdef debug
    printf("double_op_s: >>%s<<\n", temp);
    printf("double_op_d: >>%s<<\n", dst);
    #endif /* debug */
    *source = temp;
    *destination = dst;
  }
  else{
    *source = strtok(token, " ");     // Remove Spaces left by the 1st tokenize
    #ifdef debug
    printf("single_op_s: >>%s<<\n", *source);
    #endif
  }
  res = TRUE;
  return res;
}

void checkabsolute(char* operand, enum ADDR_MODE* mode){
  *operand++; //increment pointer past &

  #ifdef debug
  printf("TESTING >>%s<<\n", operand);
  #endif /* debug */

  if(is_label(operand)){        // First check if op can be label
    if(get_entry(operand)){
      #ifdef debug
      printf("OPERAND >>%s<< ABS EXISTING LABEL\n", operand);
      #endif
    }
    else{
      add_entry(operand, 0, UNKTYPE);
      #ifdef debug
      printf("OPERAND >>%s<< ABS UNKNOWN LABEL\n", operand);
      #endif
    }
    *mode = ABSOLUTE;
  }
  else if(is_number(operand) < MAX_BIT_VAL){  //If not label check numeric
    #ifdef debug
    printf("OPERAND >>%s<< ABS NUMERIC\n", operand);
    #endif
    *mode = ABSOLUTE;
  }
  else{
    #ifdef debug
    printf("OPERAND >>%s<< INVALID ABSOLUTE\n", operand);
    #endif
    error_count("ERROR: Invalid Absolute Operand:", operand);
    *mode = BAD_ADDR_MODE;
  }
}

void checkindirect(char* operand, enum ADDR_MODE* mode){
  struct symbol_entry* symbl;
  unsigned short i;
  unsigned char indirect_flag = 0;
  unsigned char autoinc_flag = 0;

  *operand++;
  #ifdef debug
  printf("TESTING >>%s<<\n", operand);
  #endif

  for(i = 0; operand[i] != '+'; i++){
    if (i == REG_SIZE){ // Max length of a register 3, (e.g. R15+)
      #ifdef debug
      printf("OPERAND TOO LONG IF EXPECTING AUTOINC\n");
      #endif /* debug */
      break;       // To avoid overflow and infinite looping
    }
  }
  /*
  If there is a plus sign it checks to see if there is anything else
  following the sign, to ensure proper format
  */
  if(operand[i] == '+' && ((operand[i+1]) == NUL)){
    operand[i] = NUL;   // Now we null the +
    // might need to break this up for accurate error message
    #ifdef debug
    printf("POTENTIALLY AUTOINC\n");
    #endif
    autoinc_flag = TRUE;
  }

  if(symbl = get_entry(operand)){
    if(symbl->type == REGTYPE){
      #ifdef debug
      printf("OPERAND >>%s<< INDIRECT\n", operand);
      #endif
      indirect_flag = TRUE;
      *mode = INDIRECT;
    }
    else{
      error_count("ERROR: Operand must be a Register in Register Indirect"
                  " Addressing:", operand);
      *mode = BAD_ADDR_MODE;
    }
  }
  else{
    error_count("ERROR: Operand Must be a register in Register Indirect"
                " Addressing:", operand);
    *mode = BAD_ADDR_MODE;
  }

  if(autoinc_flag && indirect_flag){
    #ifdef debug
    printf(">>%s<<+ is Indirect Autoincrement\n", operand);
    #endif
    *mode = INDIRECT_INCR;
  }
}

void checkimmediate(char* operand, enum ADDR_MODE* mode){
  struct symbol_entry* symbl;
  int temp;

  flag_const_immediate = FALSE;
  *operand++;
  #ifdef debug
  printf("TESTING >>%s<<\n", operand);
  #endif

  if(is_label(operand)){
    if(symbl = get_entry(operand)){
      if(symbl->type != REGTYPE){
        #ifdef debug
        printf("OPERAND >>%s<< IMMEDIATE EXISTING LABEL\n", operand);
        #endif
        *mode = IMMEDIATE;
        if(CONGEN(symbl->value)){
          flag_const_immediate = TRUE;
        }
      }
      else{
        error_count("ERROR: Operand cannot be a Register in Immediate"
                    " Addressing:", operand);
        *mode = BAD_ADDR_MODE;
      }
    }
    else{
      add_entry(operand, 0, UNKTYPE);
      #ifdef debug
      printf("OPERAND >>%s<< IMMEDIATE UNKNOWN LABEL\n", operand);
      #endif
      *mode = IMMEDIATE;
    }
  }
  else if((temp = is_number(operand)) != EXIT_FAIL){
    #ifdef debug
    printf("OPERAND >>%s<< IMMEDIATE NUMERICAL\n", operand);
    #endif
    *mode = IMMEDIATE;
    if(CONGEN(temp)){ // I need to show that this is a special case
      #ifdef debug
      printf("IMMEDIATE CONSTANT GENERATOR NUMBER\n");
      #endif
      *mode = IMMEDIATE;
      flag_const_immediate = TRUE;
    }
  }
  else{
    error_count("ERROR: Operand Invalid Immediate:", operand);
    *mode = BAD_ADDR_MODE;
  }
}

// Checks for indexed, relative or register direct
void checkdefault(char* operand, enum ADDR_MODE* mode){
  struct symbol_entry* symbl;
  char* baseaddress;
  char* index;
  char* ptr;
  unsigned char flag_valid_base = FALSE;
  unsigned char flag_valid_index = FALSE;
  unsigned short i = 0;

  baseaddress = (char* )malloc(sizeof(char)*strlen(operand));
  index = (char* )malloc((sizeof(char))*REG_SIZE);

  if(is_label(operand)){
    if(symbl = get_entry(operand)){
      if(symbl->type != REGTYPE){
        printf("OPERAND >>%s<< EXISTING LABEL RELATIVE\n", operand);
        *mode = RELATIVE;
      }
      else{
        printf("OPERAND >>%s<< REGISTER DIRECT\n", operand);
        *mode = REGISTER;
      }
    }
    else{
      printf("OPERAND >>%s<< UNKNOWN LABEL RELATIVE\n", operand);
      add_entry(operand, 0, UNKTYPE);
      *mode = RELATIVE;
    }
  }

  else if(ptr = strstr(operand, "(")){
    *ptr++;

    for(i = 0; ptr[i] != ')' && i <= REG_SIZE; i++){
      index[i] = ptr[i];
    }
    if(i == (REG_SIZE + 1)){ //The for loop increases i once unecessarily
      error_count("ERROR: There may be a missing closing parenthesis.", NULL);
      *mode = BAD_ADDR_MODE;
    }
    else{
      symbl = get_entry(index);
      if(symbl->type != REGTYPE){
        error_count("ERROR: Index Operand is not a Register:", index);
        *mode = BAD_ADDR_MODE;
      }
      else if(symbl->name == "RO" || symbl->name == "PC"){
        fprintf(fout, "WARNING: By using an index with the PC you are making use"
                " of relative addressing\n");
                flag_valid_index = TRUE;
      }
      else{
        flag_valid_index = TRUE;
        #ifdef debug
        printf("Operand >>%s<< valid index.\n", index);
        #endif /* debug */
      }
    }

    /* Now to find the base address */
    i = 0; // Reset the counter

    while(operand[i] != '('){     // Search for the opening parenthesis
      baseaddress[i] = operand[i];// No need to break at MAX_NAME_LEN since
      i++;                        // is_label checks
    }
    printf("Testing BASE ADDRESS >>%s<<\n", baseaddress);
    if(is_label(baseaddress)){            // Check to see if it follows label
      if(symbl = get_entry(baseaddress)){ // Check to see if existing label
        if(symbl->type == REGTYPE){
          error_count("ERROR: The base address cannot be a register:",
		       baseaddress);
          *mode = BAD_ADDR_MODE;
        }
        else{
          flag_valid_base = TRUE; // At this point we know that the base is valid
          #ifdef debug
          printf("BASE ADDRESS >>%s<< KNOWN VALID\n", baseaddress);
          #endif
        }
      }
      else{
        flag_valid_base = TRUE;
        #ifdef debug
        printf("BASE ADDRESS >>%s<< UNKNOWN VALID\n", baseaddress);
        #endif /* debug */
        add_entry(baseaddress, 0, UNKTYPE); // Add the forward reference
      }
    }
    else{
      error_count("ERROR: Base Address Invalid:", baseaddress);
      *mode = BAD_ADDR_MODE;
    }

    if(flag_valid_index && flag_valid_base){
      #ifdef debug
      printf("OPERAND >>%s<< INDEXED\n", operand);
      #endif
      *mode = INDEXED;
    }
  }   /* Ended searching for Indexed*/

  else if((is_number(operand)) != EXIT_FAIL){
    #ifdef debug
    printf("OPERAND >>%s<< NUMERIC RELATIVE\n", operand);
    #endif
    *mode = RELATIVE;
  }

  else{
    error_count("ERROR: Operand Unindentifiable:", operand);
    *mode = BAD_ADDR_MODE;
  }
}

void checkjump(char* line, char* jumpinst){
  char* token;
  int value;
  token = strtok(line, " \t\r\n");

  if(token == NULL){
    error_count("ERROR: Missing the Jump Operand.", NULL);
    return;
  }

  if(is_label(token)){
    if(!get_entry(token)){
      add_entry(token, 0, UNKTYPE);
      printf("%s is an unknown label\n", token);
    }
    // Adds the operand and instruction to the record list for the second pass
    add_jump_record(jumpinst, JUMP, token);
    (LC + WORD_INC) <= MAX_LC ? LC+=WORD_INC : (flag_max_lc = TRUE);
  }
  else if((value = is_number(line)) != EXIT_FAIL){
    printf("RETURNED value %d\n", value);
    printf("%s is a numerical jump\n", token);
    // Adds the operand and instruction to the record list for the second pass
    add_jump_record(jumpinst, JUMP, token);
    (LC + WORD_INC) <= MAX_LC ? LC+=WORD_INC : (flag_max_lc = TRUE);
  }
  else{
    error_count("ERROR: The Jump Operand is Invalid.", NULL);
  }
}

// Checks the record for unecessary chars.
unsigned char checkjunkrecord(char* line){
  char res = FALSE;
  char* token = strtok(line, " \t\r\n");

  if(token == NULL || *token == ';' || token[0] == '\r'){
    return res = TRUE;
  }
  else{
    error_count("ERROR: Line contains unecessary text.", NULL);
    return res;
  }
}

/*
  Increase the LC by 2 for every instruction is incorporated in the SRC increase.
  This function currently only deals with SINGLE and DOUBLE OP instructions.
*/

void incrementLC(enum ADDR_MODE* src_addr, enum ADDR_MODE* dst_addr){
  //{REGISTER, INDEXED, RELATIVE, ABSOLUTE, INDIRECT, INDIRECT_INCR, IMMEDIATE}
  unsigned char LC_INCREMENTS[] = {0, 2, 2, 2, 0, 0, 2};
  printf("LC_INCREMENTS[%d]: %d\n", *src_addr, LC_INCREMENTS[*src_addr]);
  printf("LC_INCREMENTS[%d]: %d\n", *dst_addr, LC_INCREMENTS[*dst_addr]);

  //No need to have a special case for no DST since DST NULL will up the LC
  //by the REG amount of 0
  if(!flag_max_lc){
    LC += (WORD_INC + LC_INCREMENTS[*src_addr] + LC_INCREMENTS[*dst_addr]
      -2*flag_const_immediate); //not a very elegant solution to const_gen value
  }                             //but it works more efficiently than conditionals

  if(LC >= MAX_LC){
    flag_max_lc = TRUE;
    error_count("ERROR: Exceeded Maximum LC.", NULL);
  }
}
