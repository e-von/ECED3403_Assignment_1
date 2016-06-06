/*
  secondpass.c
  This module contains the secondpass() function as well as functions which
  analyze tokens in the record linked-list to combine all the required data
  using the emit functions in emit.c and following that tranlating the output
  form the emits to srecord characters.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: May 31, 2016  - Fixed invalid odd calculation of jump offset
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"
#include "parser.h"
#include "secondpass.h"
#include "errors.h"
#include "instructions.h"
#include "symboltable.h"
#include "emit.h"
#include "srec_gen.h"

unsigned as_value[] = {0, 1, 1, 1, 2, 3, 3};
unsigned ad_value[] = {0, 1, 1, 1};

// Last minute solution to imm consts, no need for -1 and 8 cases since they use
// the same AS as Immediate mode
unsigned as_consts[] = {0, 1, 2, 0, 2};

char    reg_value[] = {-1, -1, PC_REG, SR_REG, -1, -1, PC_REG};

/*
  This function contains the processes required to decode the assembly records
  in the linked list from the first pass output. It sorts the recors into
  instructions & storage directives, decodes the contents of the data and outputs
  them to an srecord file.
*/

void secondpass(void){
  struct record_entry* record = head;
  srec = fopen("srecords.s19", "w");

  printf("\n----------Entered Second Pass Function----------\n");
  fprintf(fout, "\n--------------Second Pass Diagnostic Opcode--------------\n");
  fprintf(fout, "Notes: This file contains the record number and corresponding"
                " opcode. The opcode\nis categorized into instruction opcode,"
                " and source and destination operand values.\nA value of 0000"
                " means non-existing value\n");

  while(record){
    printf("\n----------RECORD: %d----------\n", record->line);
    fprintf(fout, "\n----------RECORD: %d----------\n", record->line);
    if(record->inst){
      switch (record->type) {
        case SINGLE:
        printf("SINGLE\n");
        type1_inst(record);
        break;
        case DOUBLE:
        printf("DOUBLE\n");
        type2_inst(record);
        break;
        case JUMP:
        printf("JUMP\n");
        type3_inst(record);
        break;
        case NONE:
        printf("NONE\n");
        struct inst_el* instptr = get_inst(record->inst);
        srec_gen(instptr->opcode, record->LC, WORDSIZE);
        printf("Output: %04x\n", instptr->opcode);
        break;
        default:
        #ifdef debug2
        printf("Internal Error in Second Pass function\n");
        #endif
      }
    }
    else{
      // wbosb = word byte org string bss
      switch (record->wbosb) {
        case WORD2:
        printf("\n----------Data %d on RECORD: %d----------\n", record->value,
                                                                record->line);
        srec_gen(record->value, record->LC, WORDSIZE);
        break;
        case BYTE2:
        printf("\n----------Data %d on RECORD: %d----------\n", record->value,
                                                                record->line);
        srec_gen(record->value, record->LC, BYTESIZE);
        break;
        case ORG2:
        printf("\n----------ORG %04x----------\n", record->value);
        srec_org(record->value);
        break;
        case STRING2:
        printf("\n----------RECORD: %d----------\n", record->line);
        printf("String: %s\n", record->string);
        srec_char(record->string, record->LC);
        break;
        case BSS2:
        printf("\n----------RECORD: %d----------\n", record->line);
        printf("BSS Value: %d\n", record->value);
        srec_bss(record->value, record->LC);
        break;
        default:
        printf("Something has broken in secondpass().\n");
        break;
      }
    }
    record = record->next;
  }
  /*
    The linked list has been read, so now emit the remaining bytes in the srecs
    that have been added but not emitted and then emit the terminating s9 record
    with the global value determined in the first pass.
  */
  emit_srec();
  emit_s9(start_address);
}

void type1_inst(struct record_entry* singleinst){
  struct inst_el *instptr;
  struct single_op inst;
  unsigned char as;
  unsigned char reg;
  int val = 0;
  unsigned short inst_out;

  instptr = get_inst(singleinst->inst);
  //as = as_value[singleinst->src_mode]; // This now is assigned to numval
  reg = reg_value[singleinst->src_mode];
  numval_extractor(singleinst->src_mode, singleinst->src_op, &val, &reg, &as);
  inst_out = emit_single(reg, as, instptr->bw, instptr->opcode);
  srec_gen(inst_out, singleinst->LC, WORDSIZE);

  // If there is a value write it in the next location, note in constant generator
  // circumstances value has been set to 0.
  if(val){
    printf("In the single inst we have a value of %04x\n", val);
    srec_gen(val, (singleinst->LC + WORDINC), WORDSIZE);
  }

  opcode_printer(inst_out, val, 0, SINGLE);

  #ifdef debug2
  printf("\nOpcode: %04x\n", instptr->opcode);
  printf("BW: %d\n", instptr->bw);
  printf("As: %d\n", as);
  printf("Source: %s\n", singleinst->src_op);
  if(singleinst->src_mode == RELATIVE ||
     singleinst->src_mode == ABSOLUTE ||
     singleinst->src_mode == IMMEDIATE){
       printf("We have a value %d\n", val);
  }
  printf("Source reg: %d\n", reg);
  printf("Output: %04x\n", inst_out);
  #endif /* debug2 */
  return;
}

void type2_inst(struct record_entry* doubleinst){
  struct inst_el *instptr;
  struct double_op inst;
  unsigned char as;
  unsigned char ad;
  unsigned char junk;
  unsigned char sreg;
  unsigned char dreg;
  int val0 = 0;
  int val1 = 0;
  unsigned short inst_out;

  instptr = get_inst(doubleinst->inst);
  //as = as_value[doubleinst->src_mode]; // This now is assigned to numval
  ad = ad_value[doubleinst->dst_mode];
  sreg = reg_value[doubleinst->src_mode];
  dreg = reg_value[doubleinst->dst_mode];
  numval_extractor(doubleinst->src_mode, doubleinst->src_op, &val0, &sreg, &as);
  numval_extractor(doubleinst->dst_mode, doubleinst->dst_op, &val1, &dreg, &junk);
  inst_out = emit_double(dreg, as, instptr->bw, ad, sreg, instptr->opcode);
  srec_gen(inst_out, doubleinst->LC, WORDSIZE);

  if(val0){
    printf("We have a val0 %d\n", val0);
    srec_gen(val0, (doubleinst->LC + WORDINC), WORDSIZE);
  }
  if(val1){
    printf("We have a val1 %d\n", val1);
    srec_gen(val1, (doubleinst->LC + DOUBLEWORDINC), WORDSIZE);
  }

  opcode_printer(inst_out, val0, val1, DOUBLE);

  #ifdef debug2
  printf("\nOpcode: %04x\n", instptr->opcode);
  printf("Source: %s\n", doubleinst->src_op);
  if(doubleinst->src_mode == RELATIVE ||
     doubleinst->src_mode == ABSOLUTE ||
     doubleinst->src_mode == IMMEDIATE){
       printf("Source Value: %d\n", val0);
  }
  printf("Source reg: %d\n", sreg);
  printf("Ad: %d\n", ad);
  printf("BW: %d\n", instptr->bw);
  printf("As: %d\n", as);
  printf("Destination: %s\n", doubleinst->dst_op);
  if(doubleinst->dst_mode == RELATIVE ||
     doubleinst->dst_mode == ABSOLUTE ||
     doubleinst->dst_mode == IMMEDIATE){
       printf("Destination Value: %d\n", val1);
  }
  printf("Destination reg: %d\n", dreg);
  printf("Output: %04x\n", inst_out);
  #endif /* debug2 */
  return;
}

void type3_inst(struct record_entry* jumpinst){
  struct inst_el *instptr;
  struct double_op inst;
  unsigned short offset;
  short distance;
  short halfdist;
  unsigned short inst_out;

  instptr = get_inst(jumpinst->inst);
  offset = jumpinst->offset;
  printf("Offset is: %d\n", offset);
  distance = offset - jumpinst->LC;
  printf("jumpinst->LC is %d\n", jumpinst->LC);
  halfdist = half_value(distance);

  #ifdef debug2
  printf("Full distance is %d\n", distance);
  printf("Half distance is %d\n", halfdist);
  #endif /* debug2 */

  if(distance >= MAX_POS_OFFSET || distance <= MAX_NEG_OFFSET){
    fprintf(fout, "ERROR: The offset used in record %d is beyond the maximum "
           "attainable\n", jumpinst->line);
  }
  else if((distance)%2){
    fprintf(fout, "ERROR: Invalid odd address %d for offset in record %d\n",
          distance, jumpinst->line);
  }
  else{
    inst_out = emit_jump(halfdist, instptr->opcode);
    srec_gen(inst_out, jumpinst->LC, WORDSIZE);
  }

  opcode_printer(inst_out, 0, 0, JUMP);

  #ifdef debug2
  printf("\nOpcode: %04x\n", instptr->opcode);
  printf("Offset: %d\n", halfdist);
  printf("Output: %04x\n", inst_out);
  #endif /* debug2 */
  return;
}

/*
  This function determines 'as', the source and destination registers as well as
  any numeric values that are needed for the opcode generation. 'as' was a later
  addition to handle cases in which immdiate constants were being used.
*/
void numval_extractor(enum ADDR_MODE mode, char* src, int* value,
                      unsigned char* reg, unsigned char* as){
  char* temp = src;
  char* index;
  char* baseaddress;
  char* ptr;
  struct symbol_entry* symbol;
  unsigned short i = 0;

  switch (mode) {
    case INDIRECT:
    case INDIRECT_INCR:
    /* Not searching for the +, it does not get passed by the first pass */
    *temp++;
    case REGISTER:
    if(mode == REGISTER){
      if(symbol = get_entry(temp)){  //In the case the coder is using an alias
        *reg = symbol->value;
        *as = as_value[mode];
        break;
      }
    }
    *temp++;
    *reg = is_number(temp);
    *as = as_value[mode];
    #ifdef debug2
    printf("*as is %d\n", *as);
    printf("*reg is %d\n", *reg);
    #endif
    break;
    case IMMEDIATE:
    *reg = PC_REG;
    *temp++;
    *value = is_number(temp);
    *as = as_value[mode];
    if(CONGEN(*value)){
      switch (*value) {
        case 0:
        case 1:
        case 2:
        *as = as_consts[*value];
        case -1:
        *reg = CG_REG;  //In constant generator cases use R3
        if(*value == -1){
          *as = as_value[mode];
        }
        break;
        case 4:
        *as = as_consts[*value];
        case 8:
        *reg = SR_REG;  // Use R2
        if(*value == 8){
          *as = as_value[mode];
        }
        break;
      }
      *value = 0;    // Indicate that there is no data to be written
    }
    #ifdef debug2
    printf("IMMEDIATE\n");
    printf("*value is %d\n", *value);
    printf("*as is %d\n", *as);
    printf("*reg is %d\n", *reg);
    #endif
    break;
    case ABSOLUTE:
    *reg = SR_REG;
    *temp++;
    case RELATIVE:
    if(mode != ABSOLUTE){
      *reg = PC_REG;
    }
    if(symbol = get_entry(temp)){
      *value = symbol->value;
    }
    else{
      *value = is_number(temp);
    }
    *as = as_value[mode];
    break;
    case INDEXED:
    baseaddress = (char* )malloc(sizeof(char)*MAX_NAME_LEN);
    index = (char* )malloc((sizeof(char))*REG_SIZE);

    ptr = strstr(temp, "(");
    *ptr++;

    for(i = 0; ptr[i] != ')'; i++){
      index[i] = ptr[i];
    }
    symbol = get_entry(index);
    *reg = symbol->value;
    printf("Index Register is: %d\n", *reg);

    /* Now to find the base address */
    i = 0; // Reset the counter

    while(temp[i] != '('){        // Search for the opening parenthesis
      baseaddress[i] = temp[i];
      i++;                        // No need to break at MAX_NAME_LEN since
    }                             // is_label checks
    symbol = get_entry(baseaddress);
    *value = symbol->value;
    *as = as_value[mode];
    printf("Base Address Value %d\n", *value);
    break;
  }
}

/* This function has been written simply for diagnostic purposes */
void opcode_printer(unsigned short inst, int val0, int val1, unsigned char type){
  switch (type) {
    case SINGLE:
    fprintf(fout, "Instruction Opcode: %04x\n"
                  "Source Value: %04x",
                   inst, val0);
    break;
    case DOUBLE:
    fprintf(fout, "Instruction Opcode: %04x\n"
                  "Source Value: %04x\nDestination Value: %04x\n",
                   inst, val0, val1);
    break;
    default:
    fprintf(fout, "Instruction Opcode: %04x\n", inst);
    break;
  }
}
