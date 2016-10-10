/*
*  cool.y
*              Parser definition for the COOL language.
*
*/

// TODO: solve shift-reduce warning

%{
  #include "cool-tree.h"
  #include "stringtab.h"
  #include "utilities.h"
  
  extern char *curr_filename;

  #define cool_yylloc curr_lineno  /* use the curr_lineno from the lexer
  for the location of tokens */
    
    extern int node_lineno;          /* set before constructing a tree node
    to whatever you want the line number
    for the tree node to be */
      
      
      #define YYLLOC_DEFAULT(Current, Rhs, N)         \
      Current = Rhs[1];                             \
      node_lineno = Current;
    
    
    #define SET_NODELOC(Current)  \
    node_lineno = Current;
    
    /* IMPORTANT NOTE ON LINE NUMBERS
    *********************************
    * The above definitions and macros cause every terminal in your grammar to 
    * have the line number supplied by the lexer. The only task you have to
    * implement for line numbers to work correctly, is to use SET_NODELOC()
    * before constructing any constructs from non-terminals in your grammar.
    * Example: Consider you are matching on the following very restrictive 
    * (fictional) construct that matches a plus between two integer constants. 
    * (SUCH A RULE SHOULD NOT BE  PART OF YOUR PARSER):
    
    plus_consts	: INT_CONST '+' INT_CONST 
    
    * where INT_CONST is a terminal for an integer constant. Now, a correct
    * action for this rule that attaches the correct line number to plus_const
    * would look like the following:
    
    plus_consts	: INT_CONST '+' INT_CONST 
    {
      // Set the line number of the current non-terminal:
      // ***********************************************
      // You can access the line numbers of the i'th item with @i, just
      // like you acess the value of the i'th exporession with $i.
      //
      // Here, we choose the line number of the last INT_CONST (@3) as the
      // line number of the resulting expression (@$). You are free to pick
      // any reasonable line as the line number of non-terminals. If you 
      // omit the statement @$=..., bison has default rules for deciding which 
      // line number to use. Check the manual for details if you are interested.
      @$ = @3;
      
      
      // Observe that we call SET_NODELOC(@3); this will set the global variable
      // node_lineno to @3. Since the constructor call "plus" uses the value of 
      // this global, the plus node will now have the correct line number.
      SET_NODELOC(@3);
      
      // construct the result node:
      $$ = plus(int_const($1), int_const($3));
    }
    
    */
    
    
    
    void yyerror(char *s);        /*  defined below; called for each parse error */
    extern int yylex();           /*  the entry point to the lexer  */
    
    /************************************************************************/
    /*                DONT CHANGE ANYTHING IN THIS SECTION                  */
    
    Program ast_root;	      /* the result of the parse  */
    Classes parse_results;        /* for use in semantic analysis */
    int omerrs = 0;               /* number of errors in lexing and parsing */
    %}
    
    /* A union of all the types that can be the result of parsing actions. */
    %union {
      Boolean boolean;
      Symbol symbol;
      Program program;
      Class_ class_;
      Classes classes;
      Feature feature;
      Features features;
      Formal formal;
      Formals formals;
      Case case_;
      Cases cases;
      Expression expression;
      Expressions expressions;
      char *error_msg;
    }
    
    /* 
    Declare the terminals; a few have types for associated lexemes.
    The token ERROR is never used in the parser; thus, it is a parse
    error when the lexer returns it.
    
    The integer following token declaration is the numeric constant used
    to represent that token internally.  Typically, Bison generates these
    on its own, but we give explicit numbers to prevent version parity
    problems (bison 1.25 and earlier start at 258, later versions -- at
    257)
    */
    %token CLASS 258 ELSE 259 FI 260 IF 261 IN 262 
    %token INHERITS 263 LET 264 LOOP 265 POOL 266 THEN 267 WHILE 268
    %token CASE 269 ESAC 270 OF 271 DARROW 272 NEW 273 ISVOID 274
    %token <symbol>  STR_CONST 275 INT_CONST 276 
    %token <boolean> BOOL_CONST 277
    %token <symbol>  TYPEID 278 OBJECTID 279 
    %token ASSIGN 280 NOT 281 LE 282 ERROR 283
    
    /*  DON'T CHANGE ANYTHING ABOVE THIS LINE, OR YOUR PARSER WONT WORK       */
    /**************************************************************************/
    
    /* Complete the nonterminal list below, giving a type for the semantic
    value of each non terminal. (See section 3.6 in the bison 
    documentation for details). */
    
    /* Declare types for the grammar's non-terminals. */
    %type <program> program
    %type <classes> class_list
    %type <class_> class
    
    /* You will want to change the following line. */
	%type <feature> feature
    %type <features> features feature_list
	%type <formal> formal
	%type <formals> formals comma_formal_list
	%type <case_> single_case
	%type <cases> case_list
	%type <expression> expression let_expression
	%type <expressions> semi_expression_list comma_expression_list expressions

    /* Precedence declarations go here. */
    
	%right LET_P
	%right ASSIGN
	%nonassoc NOT
	%nonassoc LE '<' '='
	%left '+' '-'
	%left '*' '/'
	%nonassoc ISVOID
	%nonassoc '~'
	%left '@'
	%left '.'
	
    %%
	
    /* 
    Save the root of the abstract syntax tree in a global variable.
    */
    program
	: class_list
	{ @$ = @1; ast_root = program($1); }
    ;
    
	/* Note that left-recursive is recommended here! */
    class_list
    : class				/* single class */
    { $$ = single_Classes($1);
    parse_results = $$; }
    | class_list class	/* several classes */
    { $$ = append_Classes($1,single_Classes($2)); 
    parse_results = $$; }
    ;
    
    /* If no parent is specified, the class inherits from the Object class. */
    class
	: CLASS TYPEID '{' feature_list '}' ';'
    { $$ = class_($2,idtable.add_string("Object"),$4,
    stringtable.add_string(curr_filename)); }
    | CLASS TYPEID INHERITS TYPEID '{' feature_list '}' ';'
    { $$ = class_($2,$4,$6,stringtable.add_string(curr_filename)); }
	/* Error handling */
	| error ';'
	{ $$ = NULL; }
    ;
    
    /* Feature list may be empty, but no empty features in list. */
    feature_list
	: /* empty */
    { $$ = nil_Features(); }
	| features
	{ $$ = $1; }
	;
	
	/* non empty list of features */
	features
	: feature
	{ $$ = single_Features($1); }
	| features feature
	{ $$ = append_Features($1, single_Features($2)); }
	;
	
	/* Note that only one-line expression is allowed in class declaration */
	feature
	: OBJECTID '(' comma_formal_list ')' ':' TYPEID '{' expression '}' ';'
	{ $$ = method($1, $3, $6, $8); }
	| OBJECTID ':' TYPEID ';'
	{ $$ = attr($1, $3, no_expr()); }
	| OBJECTID ':' TYPEID ASSIGN expression ';'
	{ $$ = attr($1, $3, $5); }
	/* Error handling */
	| error ';'
	{ $$ = NULL; }
	;
	
	/* Note that formal list could also be empty, */
	/* and should be comma separated if more than one */
	comma_formal_list
	: /* empty */
	{ $$ = nil_Formals(); }
	| formals
	{ $$ = $1; }
	;
	
	formals
	: formal
	{ $$ = single_Formals($1); }
	| formals ',' formal
	{ $$ = append_Formals($1, single_Formals($3)); }
	;
	
	formal	: OBJECTID ':' TYPEID
	{ $$ = formal($1, $3); }
	;
	
	/* Note that expression list could be empty */
	/* and should be comma separated if more than one */
	comma_expression_list
	: /* empty */
	{ $$ = nil_Expressions(); }
	| expressions
	{ $$ = $1; }
	;
	
	expressions
	: expression
	{ $$ = single_Expressions($1); }
	| expressions ',' expression
	{ $$ = append_Expressions($1, single_Expressions($3)); }
	;
	
	/* Expressions separated by semi is required not to be empty */
	semi_expression_list
	: expression ';'
	{ $$ = single_Expressions($1); }
	| semi_expression_list expression ';'
	{ $$ = append_Expressions($1, single_Expressions($2)); }
	/* Error handling */
	| error ';'
	{ $$ = NULL; }
	;
	
	/* Represents assign expressions separated by comma */
	/* Here we have to use right recursion, or we have to extend the default tree structure */
	let_expression
	: OBJECTID ':' TYPEID IN expression %prec LET_P
	{ $$ = let($1, $3, no_expr(), $5); }
	| OBJECTID ':' TYPEID ASSIGN expression IN expression %prec LET_P
	{ $$ = let($1, $3, $5, $7); }
	| OBJECTID ':' TYPEID ',' let_expression
	{ $$ = let($1, $3, no_expr(), $5); }
	| OBJECTID ':' TYPEID ASSIGN expression ',' let_expression
	{ $$ = let($1, $3, $5, $7); }
	/* Error handling */
	| error IN expression %prec LET_P
	{ $$ = NULL; }
	| error ',' let_expression
	{ $$ = NULL; }
	;
	
	single_case : OBJECTID ':' TYPEID DARROW expression ';'
	{ $$ = branch($1, $3, $5); }
	;
	
	case_list
	: single_case
	{ $$ = single_Cases($1); }
	| case_list single_case
	{ $$ = append_Cases($1, single_Cases($2)); }
	;
	
	expression
	/* Assign operation */
	: OBJECTID ASSIGN expression
	{ $$ = assign($1, $3); }
	
	/* Three kinds of dispatch */
	/* Note that if the "self" parameter is omitted, then just add that id to id table */
	| expression '@' TYPEID '.' OBJECTID '(' comma_expression_list ')'
	{ $$ = static_dispatch($1, $3, $5, $7); }
	| expression '.' OBJECTID '(' comma_expression_list ')'
	{ $$ = dispatch($1, $3, $5); }
	| OBJECTID '(' comma_expression_list ')'
	{ $$ = dispatch(object(idtable.add_string("self")), $1, $3); }
	
	/* Control flow */
	| IF expression THEN expression ELSE expression FI
	{ $$ = cond($2, $4, $6); }
	| WHILE expression LOOP expression POOL
	{ $$ = loop($2, $4); }
	| CASE expression OF case_list ESAC
	{ $$ = typcase($2, $4); }
	
	/* Statement */
	| LET let_expression
	{ $$ = $2; }
	
	/* Single object */
	| OBJECTID
	{ $$ = object($1); }
	
	/* Operators */
	| expression '+' expression
	{ $$ = plus($1, $3); }
	| expression '-' expression
	{ $$ = sub($1, $3); }
	| expression '*' expression
	{ $$ = mul($1, $3); }
	| expression '/' expression
	{ $$ = divide($1, $3); }
	| expression '<' expression
	{ $$ = lt($1, $3); }
	| expression '=' expression
	{ $$ = eq($1, $3); }
	| expression LE expression
	{ $$ = leq($1, $3); }
	| '~' expression
	{ $$ = neg($2); }
	| NOT expression
	{ $$ = comp($2); }
	| NEW TYPEID
	{ $$ = new_($2); }
	| ISVOID expression
	{ $$ = isvoid($2); }
	
	/* Blocks and brackets */
	| '(' expression ')'
	{ $$ = $2; }
	| '{' semi_expression_list '}'
	{ $$ = block($2); }
	
	/* Constants */
	| INT_CONST
	{ $$ = int_const($1); }
	| STR_CONST
	{ $$ = string_const($1); }
	| BOOL_CONST
	{ $$ = bool_const($1); }
	;
	
    /* end of grammar */
    %%
    
    /* This function is called automatically when Bison detects a parse error. */
    void yyerror(char *s)
    {
      extern int curr_lineno;
      
      cerr << "\"" << curr_filename << "\", line " << curr_lineno << ": " \
      << s << " at or near ";
      print_cool_token(yychar);
      cerr << endl;
      omerrs++;
      
      if(omerrs>50) {fprintf(stdout, "More than 50 errors\n"); exit(1);}
    }
    
    