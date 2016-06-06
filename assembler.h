#ifndef ASSEMBLER_H
#define ASSEMBLER_H

/*
  assembler.h
  Header file for assembler.c. Structures provided by the ECED3403 class of
  Dalhousie University.

  Coder: Elias Vonapartis, with code from ECED3403
  Release Date: May 28, 2016
  Latest Updates: None
*/

#define debug

/* Global Files for I/O */
FILE *fp;   // input file
FILE *fout; // contains symbol table and error messages
FILE *srec; // contains srecords

int LC; // Won't declare it as unsigned short since it would be best to see
        // the value of a potential overflow.
unsigned char flag_max_lc;

enum ADDR_MODE{REGISTER, INDEXED, RELATIVE, ABSOLUTE, INDIRECT, INDIRECT_INCR,
              IMMEDIATE, BAD_ADDR_MODE};
enum INST_TYPE {NONE, SINGLE, DOUBLE, JUMP};
enum BYTE_COMB {WORD, BYTE, OFFSET};

/* Function declarations */
void initialize(void);
void terminate(void);

#endif /* ASSEMBLER_H */
