#ifndef SREC_GEN_H
#define SREC_GEN_H

/*
  srec_gen.h
  Header file for srec_gen.c. Based on code provided by the ECED3403 course at
  Dalhousie University.

  Coder: Code from ECED3403 with additions by Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

/* Definitions */
#define PRIVATE static

/* Data Declarations */
extern FILE *srec;

/* Function Declarations */
void start_srec(unsigned short);
unsigned char write_srec(unsigned char);
void emit_srec(void);
void srec_gen(unsigned short, unsigned short, unsigned char );
void emit_s9(unsigned short );
void srec_char(char* , unsigned short );
void srec_org(unsigned short);
void srec_bss(unsigned short, unsigned short);

#endif /* SREC_GEN_H */
