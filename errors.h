#ifndef ERRORS_H
#define ERRORS_H

/*
  errors.h
  Header file for errors.h. Contains some currently unused code a linked list
  of errors.

  Coder: Elias Vonapartis
  Release Date: May 28, 2016
  Latest Updates: None
*/

/* Globals */
unsigned short errors;
extern FILE* fout;

/* Data Structures */
struct error_el {
    unsigned short line;
    int error_num;
    struct error_el *next;
};

/* Function Declarations */
void errorprinter(struct error_el* );
unsigned char secondpasscheck(void);
void error_count(char*, char* );

#endif /* ERRORS_H */
