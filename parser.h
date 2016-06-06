#ifndef PARSER_H
#define PARSER_H

/*
  parser.h
  Header file for parser.c

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/


#define LINE_LEN  256
#define NUL       '\0'
#define TRUE        1
#define FALSE       0
#define HEXADECIMAL 1
#define DECIMAL     0

extern FILE* fout;

unsigned char flag_end_of_program;
unsigned char flag_first_token_label;
unsigned short start_address;

char* global;
unsigned int line_number;

enum TOKENTYPE {LABEL, INST, DIR, OP, COMMENT, UNKNOWN};

struct firsttoken{
  enum TOKENTYPE type;
  struct dir_el *dirptr;
  struct inst_el *instptr;
};

/* Function Declarations */
void firstpass(FILE* );
void parse_record(char* );
struct firsttoken sort(char *);
void analyzelabel(char* , char* );
unsigned char is_label(char* );
int is_number(char* );
char* storeline(char* );
int cyclenumber(char* , int );

#endif /* PARSER_H */
