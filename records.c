/*
  records.c
  This module contains the firstpass() function as well as supporting functions
  to analyze tokens in the records.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: June 8, 2016  - Added jump forward reference support
*/

#include <stdio.h>
#include <stdlib.h>
#include "records.h"
#include "symboltable.h"

struct record_entry* new_entry(char* inst, enum INST_TYPE type, char* src_op,
            char* dst_op, enum ADDR_MODE src_mode, enum ADDR_MODE dst_mode,
            unsigned short offset, char* string, int value, unsigned char wbosb){

	struct record_entry* newentry;
  newentry = malloc(sizeof(struct record_entry));

  newentry->line = line_number;
  newentry->LC = LC;
  newentry->inst = inst;
  newentry->type = type;
  newentry->src_op = src_op;
  newentry->dst_op = dst_op;
  newentry->src_mode = src_mode;
  newentry->dst_mode = dst_mode;
  newentry->offset = offset;
  newentry->string = string;
  newentry->value = value;
  newentry->wbosb = wbosb;

	newentry->prev = NULL;
	newentry->next = NULL;
	return newentry;
}

/*
  This function creates a new node to add to the linked-list. All data needed
  for the entry is passed through newentry.
*/
void double_linking(struct record_entry* newentry, struct record_entry* temp){
  if(head == NULL){
    head = newentry;
    return;
  }
  while(temp->next !=NULL){
    temp = temp->next;
  }
  temp->next = newentry;
  newentry->prev = temp;
}

/*
  The following add_x_record functions are different renditions of the same code.
  They take the arguments needed for their respective x variables. Repeated code
  for clarity reasons in the first pass code of the assembler.
*/
void add_inst_record(char* inst, enum INST_TYPE type, char* src_op, char* dst_op,
                     enum ADDR_MODE src_mode, enum ADDR_MODE dst_mode){
   struct record_entry* temp = head;
   struct record_entry* newentry;
   newentry = new_entry(inst, type, src_op, dst_op, src_mode, dst_mode, -1,
                        NULL, -1, -1);
   double_linking(newentry, temp);
}

void add_jump_record(char* inst, enum INST_TYPE type, char* offset){
  struct record_entry* temp = head;
  struct record_entry* newentry;
  struct symbol_entry* tmp;
  short off;

  (tmp = get_entry(offset)) ? (off = tmp->value) : (off = is_number(offset));

  if(tmp){
    if(tmp->type == UNKTYPE){
      newentry = new_entry(inst, type, offset, NULL, -1, -1, off, NULL, -1, -1);
    }
    else{
      newentry = new_entry(inst, type, NULL, NULL, -1, -1, off, NULL, -1, -1);
    }
  }
  else{
    newentry = new_entry(inst, type, NULL, NULL, -1, -1, off, NULL, -1, -1);
  }
  double_linking(newentry, temp);
}

void add_string_record(char* string, unsigned short length){
  struct record_entry* temp = head;
  struct record_entry* newentry;
  newentry=new_entry(NULL, -1, NULL, NULL, -1, -1, -1, string, -1, STRING2);
  double_linking(newentry, temp);
}

void add_data_record(int number, unsigned char BW){
  struct record_entry* temp = head;
  struct record_entry* newentry;
  newentry=new_entry(NULL, -1, NULL, NULL, -1, -1, -1, NULL, number, BW);
  double_linking(newentry, temp);
}

void add_org_record(unsigned short address){
  struct record_entry* temp = head;
  struct record_entry* newentry;
  newentry=new_entry(NULL, -1, NULL, NULL, -1, -1, -1, NULL, address, ORG2);
  double_linking(newentry, temp);
}

void add_bss_record(unsigned short addresses){
  struct record_entry* temp = head;
  struct record_entry* newentry;
  newentry=new_entry(NULL, -1, NULL, NULL, -1, -1, -1, NULL, addresses, BSS2);
  double_linking(newentry, temp);
}
/*
  Prints part of the structures' members for debugging purposes.
*/
void print_records(void){
  struct record_entry* temp = head;

  if(temp == NULL){
    printf("The record table is empty\n");
    return;
  }

	while(temp != NULL) {
    printf("Record: %d \tLC: %d \tINST: %s \tType: %d\n\t\tSRC: %s \tDST: %s "
    "\tSRCMODE: %d\t DSTMODE: %d \tOffset: %d\n",
    temp->line, temp->LC, temp->inst, temp->type, temp->src_op, temp->dst_op,
    temp->src_mode, temp->dst_mode, temp->offset);
		temp = temp->next;
	}
}

/* This function clears the whole record table */
void clear_records(void){
  struct record_entry *oldentry = head;
  while(oldentry){
    free(oldentry);
    head = head->next;
    oldentry = head;     // Update to the next existing node
  }
}
