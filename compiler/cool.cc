#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cool-io.h"
#include "cool-tree.h"
#include "utilities.h"

FILE* fin;   // This is the file pointer from which the compiler reads its input.

extern int optind;            // for option processing
extern char* filename;    // output name without suffix
extern Program ast_root;             // root of the abstract syntax tree

extern int cool_yylex();

extern YYSTYPE cool_yylval;           // Not compiled with parser, so must define this.
extern int cool_yydebug;     // not used, but needed to link with handle_flags
extern int curr_lineno;
char* curr_filename;
extern int omerrs;             // a count of lex and parse errors
extern int cool_yyparse();

void handle_flags(int argc, char* argv[]);

extern int do_lexer;
extern int do_parser;
extern int do_semant;

void dump_lexer() {
  int token = 0;
  cout << "#name \"" << curr_filename << "\"" << endl;
  while ((token = cool_yylex()) != 0) {
    dump_cool_token(cout, curr_lineno, token, cool_yylval);
  }
}

void dump_parser() {
  ast_root->dump_with_types(cout, 0);
}

void dump_semant() {
  ast_root->dump_with_types(cout, 0);
}

int main(int argc, char* argv[]) {
  //
  // Init global variables
  //
  curr_lineno = 1;
  //
  // Handle flags
  //
  handle_flags(argc, argv);

  //
  // Process files and names
  //
  curr_filename = strdup(argv[optind]);
  fin = fopen(curr_filename, "r");

  if (fin == NULL) {
    cerr << "Could not open input file " << curr_filename << endl;
    exit(1);
  }

  std::string out_filename;
  if (!filename && optind < argc) {   // no -o option
    char* dot = strrchr(argv[optind], '.');
    if (dot) { *dot = '\0'; } // strip off file extension
    filename = new char[strlen(argv[optind]) + 8];
    strcpy(filename, argv[optind]);
    out_filename = std::string(filename) + ".s";
  } else {
    out_filename = filename;
  }

  //
  // Lexer
  //
  if (do_lexer) {
    dump_lexer();
    goto finish;
  }

  //
  // Parser
  //
  cool_yyparse();
  if (omerrs != 0) {
    cerr << "Compilation halted due to lex and parse errors\n";
    exit(1);
  }

  if (do_parser) {
    dump_parser();
    goto finish;
  }

  //
  // Semant
  //
  ast_root->semant();

  if (do_semant) {
    dump_semant();
    goto finish;
  }

  //
  // Code Generation
  //
  if (filename) {
    ofstream s(out_filename);
    ast_root->cgen(s);
  } else {
    ast_root->cgen(cout);
  }

  //
  // Program finish
  //
  finish:
  fclose(fin);
  return 0;
}
