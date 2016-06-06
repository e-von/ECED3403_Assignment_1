#ifndef EMIT_H
#define EMIT_H

/*
  emit.h
  Header file for emit.c Based on code from the ECED3403 class of Dalhousie
  University.

  Coder: Elias Vonapartis, with code from ECED3403
  Release Date: May 28, 2016
  Latest Updates: None
*/

struct single_op{
  unsigned reg: 4;
  unsigned as: 2;
  unsigned bw: 1;
  unsigned opcode: 9;
};

struct double_op{
  unsigned dst: 4;
  unsigned as: 2;
  unsigned bw: 1;
  unsigned ad: 1;
  unsigned src: 4;
  unsigned opcode: 4;
};

struct jump_op{
  short offset: 10;
  unsigned opcode: 6;
};

/* Declarations */
unsigned short emit_single(unsigned, unsigned, unsigned, unsigned);
unsigned short emit_double(unsigned, unsigned, unsigned, unsigned, unsigned, unsigned);
unsigned short emit_jump(short, unsigned);
void emit_inst(void);
unsigned short emit_data(unsigned short, unsigned short);
unsigned short emit_string(unsigned short, char* );


#endif /* EMIT_H */
