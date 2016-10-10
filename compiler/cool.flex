/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here INITIALly
 */
%{
#include <cool.h>
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>
#include <string.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");
#define RET_ERROR(s) {\
	cool_yylval.error_msg = (s);\
	return ERROR;}

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

#define ADD_CHAR_TO_STR(c) \
	if ( string_buf_ptr - string_buf + 1 < MAX_STR_CONST ) \
		*string_buf_ptr++ = (c);\
	else {\
		BEGIN(skip);\
		RET_ERROR("String constant too long")\
	}

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

int num_comment_depth = 0;
 
%}

/*
 * Section of keywords
*/

/* The keywords of cool are: class, else, false, fi, if, in, inherits, 
 * isvoid, let, loop, pool, then, while, case, esac, new, of, not, true.
 */

CLASS		(C|c)(L|l)(A|a)(S|s)(S|s)
ELSE		(E|e)(L|l)(S|s)(E|e)
FI			(F|f)(I|i)
IF			(I|i)(F|f)
IN			(I|i)(N|n)
INHERITS	(I|i)(N|n)(H|h)(E|e)(R|r)(I|i)(T|t)(S|s)
LET			(L|l)(E|e)(T|t)
LOOP		(L|l)(O|o)(O|o)(P|p)
POOL		(P|p)(O|o)(O|o)(L|l)
THEN		(T|t)(H|h)(E|e)(N|n)
WHILE		(W|w)(H|h)(I|i)(L|l)(E|e)
CASE		(C|c)(A|a)(S|s)(E|e)
ESAC		(E|e)(S|s)(A|a)(C|c)
OF			(O|o)(F|f)
NEW			(N|n)(E|e)(W|w)
ISVOID		(I|i)(S|s)(V|v)(O|o)(I|i)(D|d)
NOT			(N|n)(O|o)(T|t)

/* Except for the constants true and false, keywords are case insensitive. */
BOOL_CONST	(t(R|r)(U|u)(E|e))|(f(A|a)(L|l)(S|s)(E|e))

 /*
  *  The multiple-character operators.
  */

ASSIGN		"<-"
DARROW		"=>"
LE			"<="


/*
 * Section of single character
*/


SINGLE		"+"|"-"|"*"|"/"|"="|"<"|"."|"~"|","|";"|":"|"("|")"|"@"|"{"|"}"

WHITE		" "|\t|\f|\r|\v


/*
 * Section of identifiers
*/

/* There are two other identifiers, self and SELF TYPE that are treated specially by Cool but are not treated as keywords */


/* Integers are non-empty strings of digits 0-9. */
INT_CONST	[0-9]+

/* Type identifiers begin with a capital letter; */
TYPEID		[A-Z][A-Za-z0-9_]*

/* object identifiers begin with a lower case letter. */
OBJ_ID		[a-z][A-Za-z0-9_]*


/*
 * Section of strings
*/

COMM_LINE	"--"[^\n\0]*
COMM_OPEN	"(*"
COMM_CLOSE	"*)"
QUOTE		\"

ANY_CHAR	.

%Start		str comm skip

%%

 /*
  *  Nested comments
  */

 /* Do nothing with one line comment
  * But only in the INITIAL state
 */
<INITIAL>{COMM_LINE}		;

 /* Increase comment depth when in comments or in INITIAL state */
<INITIAL,comm>{COMM_OPEN}	{
	BEGIN(comm);
	num_comment_depth++;
}

<INITIAL>{COMM_CLOSE}		RET_ERROR("Unmatched *)")

 /* Comment could be closed only when it is started */
<comm>{COMM_CLOSE}			{
	num_comment_depth--;
	if ( num_comment_depth == 0 ) {
		BEGIN(INITIAL);
	}
}
 /* Show error when meet EOF in comments */
<comm><<EOF>>				{
	BEGIN(INITIAL);
	RET_ERROR("EOF in comment")
}

<comm>\n					curr_lineno++;

 /* Do nothing for other characters */
<comm>.						;

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */

<INITIAL>{QUOTE}	{
	string_buf_ptr = string_buf;
	*string_buf_ptr = '\0';
	BEGIN(str);
}		

<str><<EOF>> 	{
	BEGIN(INITIAL);
	RET_ERROR("EOF in string constant")
}

 /* For escaped new line, regard it as new line character */
<str>\\\n		{
	curr_lineno++;
	ADD_CHAR_TO_STR('\n')
}
 /* For single new line, show error and then auto come to next line */
 /* But notice that the state should be turned into INITIAL */
<str>\n			{
	curr_lineno++;
	BEGIN(INITIAL);
	RET_ERROR("Unterminated string constant")
}
 /* For null character, manually skip to end of the string */
<str>\0			{
	BEGIN(skip);
	RET_ERROR("String contains null character")
}
<str>\\\0			{
	BEGIN(skip);
	RET_ERROR("String contains escaped null character")
}
<str>\\b		ADD_CHAR_TO_STR('\b')
<str>\\t		ADD_CHAR_TO_STR('\t')
<str>\\n		ADD_CHAR_TO_STR('\n')
<str>\\f		ADD_CHAR_TO_STR('\f')
<str>\\.		ADD_CHAR_TO_STR(yytext[1])
 /* This should be here in order to make '\"' work properly */
<str>{QUOTE}	{
	*string_buf_ptr = '\0';
	cool_yylval.symbol = stringtable.add_string(strdup(string_buf));
	BEGIN(INITIAL);
	return STR_CONST;
}
 /* This should be at last */
<str>.			ADD_CHAR_TO_STR(yytext[0])

 /*
  * Skip mode used to skip useless characters in the string constant
  */

 /* Skip mode: ignore any character until quote or newline */
<skip>\n		{curr_lineno++; BEGIN(INITIAL);}
<skip>\"		BEGIN(INITIAL);
<skip>.			;

 /* Other things could be processed only when at INITIAL state */

<INITIAL>{

 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */

{CLASS}			{ return CLASS; }
{ELSE}			{ return ELSE; }
{FI}			{ return FI; }
{IF}			{ return IF; }
{IN}			{ return IN; }
{INHERITS}		{ return INHERITS; }
{LET}			{ return LET; }
{LOOP}			{ return LOOP; }
{POOL}			{ return POOL; }
{THEN}			{ return THEN; }
{WHILE}			{ return WHILE; }
{CASE}			{ return CASE; }
{ESAC}			{ return ESAC; }
{OF}			{ return OF; }
{NEW}			{ return NEW; }
{ISVOID}		{ return ISVOID; }
{NOT}			{ return NOT; }

 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return DARROW; }
{ASSIGN}		{ return ASSIGN; }
{LE}			{ return LE; }

 /*
  *  ID and constants
  */

{BOOL_CONST}	{
	/* if you match on a token BOOL_CONST,
	 * your lexer has to record whether its value is true or false
	*/
	cool_yylval.boolean = (yytext[0] == 't') ? 1 : 0;
	return BOOL_CONST;
}
  
{TYPEID}|(SELF_TYPE)	{
	cool_yylval.symbol = idtable.add_string(yytext);
	return TYPEID;
}

{OBJ_ID}|(self)	{
	cool_yylval.symbol = idtable.add_string(yytext);
	return OBJECTID;
}

{INT_CONST}		{
	cool_yylval.symbol = inttable.add_string(yytext);
	return INT_CONST;
}

 /*
  *  The ASCII single characters
  */

{SINGLE}		{ return yytext[0]; }
{WHITE}			;
\n				{ curr_lineno++; }

 /* When an invalid character (one that can’t begin any token) is encountered, 
  * a string containing just that character should be returned as the error string.
  */
{ANY_CHAR} 		RET_ERROR(yytext);

}

%%