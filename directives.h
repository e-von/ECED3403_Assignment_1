#ifndef DIRECTIVES_H
#define DIRECTIVES_H

/*
  directives.h
  Header file for directives.c

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

#define LASTDIR     "zzz"
#define BYTELEN     8       // in bits
#define MAXBYTEVAL  255     // in decimal
#define MAXWORDVAL  65535
#define WORD        2
#define HALFWORD    1
#define EQUATE      0
#define INCREMENT   1
#define STARTING    0
#define ENDING      1
#define START_ADDR  0x0000

extern FILE* fout;

enum DIRENTRY {ALIGN, BES, BSS, DBYTE, END, EQU, ORG, STRING, DWORD, DLASTDIR};

struct dir_el{
  char *dir;
  enum INST_TYPE type;
  enum DIRENTRY entry;
};

/* Function Declarations */
struct dir_el* get_dir(char *dir);
void analyzedirective(char *, struct firsttoken);
void align(char* );
void bss(char* , unsigned char);
void byte(char* );
void end(char* );
void equ(char* );
void origin(char* );
void string(char* );
void word(char* );
void adjustLC(unsigned short , unsigned char );

#endif /* DIRECTIVES_H */
