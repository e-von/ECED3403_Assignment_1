/*
  srec_gen.c
  Module which contains functions that accept opcode and output srec format
  characters. The code contained here was based on code provided by the ECED3403
  class at Dalhousie University.

  Coder: Elias Vonapartis, based on ECED3403 code
  Release Date: May 28, 2016
  Latest Updates: None
*/

#include <stdio.h>
#include <stdlib.h>
#include "srec_gen.h"
#include "secondpass.h"

#define SREC_DATA_SZ  32
#define HIGHBYTE(x)   ((x >> 8) & 0x00FF)
#define LOWBYTE(x)    (x & 0x00FF)

PRIVATE unsigned char srec_buffer[SREC_DATA_SZ];
PRIVATE unsigned srec_index; /* +3 is length */
PRIVATE unsigned srec_chksum;
PRIVATE unsigned srec_addr;

void start_srec(unsigned short address){
  /*
   Initialize the srecord for output
  */
  srec_index = 0;
  srec_chksum = 0;
  srec_addr = address;
  srec_chksum += address & 0xff;   	   /* Least significant 16-bits */
  srec_chksum += (address >> 8) & 0xff; /* Most significant 16-bits */
}

unsigned char write_srec(unsigned char byte){
  /*
   Write one byte to the srec_buffer[]
   Stop if srec_index exceeds SREC_DATA_SZ
   Otherwise return number of bytes remaining in buffer
  */

  if(srec_index >= SREC_DATA_SZ){
    return -1;
  }

  srec_buffer[srec_index++] = byte;
  srec_chksum += byte;

  return (SREC_DATA_SZ - srec_index);
}

void emit_srec(){
  /*
   Write S1, length, address, bytes, and chksum to s-rec file
  */
  unsigned short len;
  unsigned short i;

  /* Include len (1) and address (2) byte-pair count */
  len = srec_index + 3;

  /* %02x - two hex digits with zero fill */
  printf("S1%02X%04X", len, srec_addr);
  fprintf(srec, "S1%02X%04X", len, srec_addr);

  /* Write contents of buffer to s-rec file */
  for (i=0; i<srec_index; i++){
    printf("%02X", srec_buffer[i]);
  	fprintf(srec, "%02X", srec_buffer[i]);
  }

  /* Include length in checksum */
  srec_chksum += len;

  /* Write ones-complement of checksum - note suppression of sign extension */
  printf("%02X\n", (~srec_chksum) & 0xff);
  fprintf(srec, "%02X\n", (~srec_chksum) & 0xff);
}

void emit_s9(unsigned short address){
  /*
   Write S1, length, address, bytes, and chksum to s-rec file
  */
  int len;
  unsigned char chksum = 0;

  /* Include cksum (1) and address (2) byte-pair count */
  len = 3;

  /* %02x - two hex digits with zero fill */
  fprintf(srec,"S9%02X%04X", len, address);

  /*include length in checcksum*/
  chksum += len;
  chksum += address & 0xff;          /*Least significant 16-bits*/
  chksum += (address >> 8) & 0xff;   /*Most significan 16-bits*/

  /*write ones-complement of checksum - note suppresion of sign extension*/
  fprintf(srec,"%02X\n", (~chksum) & 0xff);
}


/*
  This function makes use of the provided code to generate the srecords in
  little endian format if necessary. It takes the a word sized argument, the
  LC pointing to it and whether the data is a byte or word.
*/

void srec_gen(unsigned short datum, unsigned short location, unsigned char bw){
  unsigned char low_byte;
  unsigned char high_byte;
  unsigned short buff_space;

  printf("\nemit_srec\n");
  printf("emitting datum: %04x\n", datum);

  low_byte = LOWBYTE(datum);
  buff_space = write_srec(low_byte);
  printf("buff_space_byte %d\n", buff_space);
  if(buff_space == 0){
    emit_srec();
    start_srec(location + 1);
  }

  if(bw == WORDSIZE){
    high_byte = HIGHBYTE(datum);
    buff_space = write_srec(high_byte);
    printf("buff_space %d\n", buff_space);
    printf("high_byte %02x\n", high_byte);
    printf("low_byte %02x\n", low_byte);

    /*
      We check to see if the buffer is full. We only need to check for == 0 due
      to the addition of one byte at a time. If the record has reached its max
      value we produce it and start a new one.
    */
    if(buff_space == 0){
      emit_srec();
      start_srec(location + 2);
    }
  }
  
  return;
}

void srec_char(char* datum, unsigned short location){
  unsigned short buff_space;
  unsigned short i = 0;

  // The following is to avoid any crashes. This message should never appear
  if(datum == NULL){
    fprintf(fout, "ERROR: Attempting to write an empty string to srec\n");
    return;
  }
  // Add each char to string seperately
  while(datum[i]){
    buff_space = write_srec(datum[i]);
    if(buff_space == 0){
      emit_srec();
      start_srec(location + i);
    }
    i++;
  }
}

void srec_org(unsigned short address){
  emit_srec();                  //emits the current s-rec
  start_srec(address);          //starts an new one at the address specified
}

void srec_bss(unsigned short length, unsigned short location){
  unsigned short buff_space;
  unsigned short i = 0;

  while(length - i){
    buff_space = write_srec('0');   // Save space by writing char '\0'
    if(buff_space == 0){
      emit_srec();
      start_srec(location + i);
    }
    i++;
  }
}
