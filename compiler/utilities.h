#ifndef _UTILITIES_H_
#define _UTILITIES_H_


#include "cool-io.h"

/* Locations */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE int /* the type of locations */
#endif
#include "cool-parse.h"

extern char* cool_token_to_string(int tok);

extern void print_cool_token(int tok);

extern void fatal_error(char*);

extern void print_escaped_string(ostream& str, const char* s);

extern char* pad(int);
/*  On some machines strdup is not in the standard library. */
//char *strdup(const char *s);
extern void dump_cool_token(ostream& out, int lineno,
                            int token, YYSTYPE yylval);

#endif
