/*
  emit.c
  Module which contains functions that apply a union process to output contiguous
  opcode. The code contained here was based on code provided by the ECED3403
  class at Dalhousie University.

  Coder: Elias Vonapartis, based on ECED3403 code
  Release Date: May 28, 2016
  Latest Updates: None
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emit.h"
#include "instructions.h"

/*
  Unions are declared first, then each function included here simply assigns the
  code input to the union strucure, then returns the unsigned short opcode. 
*/

union so_olay{
  struct single_op sop;
  unsigned short ussop;
};

union do_olay{
  struct double_op dop;
  unsigned short usdop;
};

union j_olay{
  struct jump_op jop;
  unsigned short usjop;
};

unsigned short emit_single(unsigned reg, unsigned as, unsigned bw,
                           unsigned opcode){
  union so_olay so;

  so.sop.reg = reg;
  so.sop.as = as;
  so.sop.bw = bw;
  so.sop.opcode = opcode;
  return so.ussop;
}

unsigned short emit_double(unsigned dst, unsigned as, unsigned bw, unsigned ad,
                           unsigned src, unsigned opcode){
  union do_olay d_o;

  d_o.dop.dst = dst;
  d_o.dop.as = as;
  d_o.dop.bw = bw;
  d_o.dop.ad = ad;
  d_o.dop.src = src;
  d_o.dop.opcode = opcode;
  return d_o.usdop;
}

unsigned short emit_jump(short offset, unsigned opcode){
  union j_olay jo;

  jo.jop.offset = offset;
  jo.jop.opcode = opcode;
  return jo.usjop;
}
