%{
#include "hcast.h"
#include "compilestate.h"

void yyerror(char *s);
int yylex(void);

#define YYERROR_VERBOSE

%}

%union
{
	string *identifier_t;
	string *number_t;

	program *program_t;
	
	declarations *declarations_t;
	declaration *declaration_t;

	declarator *declarator_t;
	declarators *declarators_t;

	compound_statement *compound_statement_t;
	statement *statement_t;
	statements *statements_t;

	expression *expression_t;

	expression_list *expression_list_t;
	pin_list *pin_list_t;
}


%type<program_t> program
%type<declarations_t> declarations
%type<declaration_t> declaration
%type<declarator_t> declarator
%type<declarators_t> declarators
%type<compound_statement_t> function
%type<statement_t> statement
%type<statements_t> statements
%type<compound_statement_t> compound_statement
%type<statement_t> seq_statement
%type<statement_t> par_statement
%type<statement_t> while_statement
%type<statement_t> if_statement
%type<statement_t> if_else_statement
%type<statement_t> assignment_statement
%type<statement_t> delay_statement
%type<expression_t> expression
%type<expression_list_t> expression_list
%type<pin_list_t> pin_list


%token VOID
%token MAIN
%token PAR
%token SEQ
%token UNSIGNED
%token SIGNAL
%token RAM
%token EXTRAM
%token PORT
%token IF
%token ELSE
%token WHILE
%token DELAY
%token <identifier_t> IDENTIFIER
%token <number_t> NUMBER

%left BOOL_OR
%left BOOL_AND
%left '^'
%left '|'
%left '&'
%left EQUAL_OP UNEQUAL_OP
%left '+' '-'
%left '@'
%right '!' '~' NEG
%left '['

%%

program:	declarations function			{state.hcast = $$ = new program($1,$2);}
		|	function						{$$ = new program(NULL,$1);}

declarations:	declaration					{$$ = new declarations($1);}
		|		declarations declaration	{$1->push_back($2); $$ = $1;}

declaration:	UNSIGNED NUMBER declarators ';'			{$$ = new plain_declaration($2,$3);}
		|		SIGNAL UNSIGNED NUMBER declarators ';'		{$$ = new signal_declaration($3,$4);}
		|		EXTRAM UNSIGNED NUMBER declarators ';'		{$$ = new extram_declaration($3,$4);}
		|		RAM UNSIGNED NUMBER declarators ';'		{$$ = new ram_declaration($3,$4);}
		|		PORT IDENTIFIER '(' expression_list ')' '(' pin_list ')' ';' {$$ = new port_declaration($2,$4,$7);}

declarator:	IDENTIFIER						{$$ = new plain_declarator($1);}
		|	IDENTIFIER '[' NUMBER ']'		{$$ = new array_declarator($1,$3);}

declarators:	declarator					{$$ = new declarators($1);}
		|	declarators ',' declarator		{$1->push_back($3); $$ = $1;}

function: VOID MAIN '(' VOID ')' compound_statement	{$$ = $6;}


statement:	seq_statement					{$$ = $1;}
		|	par_statement					{$$ = $1;}
		|	assignment_statement			{$$ = $1;}
		|	while_statement					{$$ = $1;}
		|	if_statement					{$$ = $1;}
		|	if_else_statement				{$$ = $1;}
		|	delay_statement					{$$ = $1;}

statements:	statement						{$$ = new statements($1);}
		|	statements statement			{$1->push_back($2); $$ = $1; }

delay_statement:	DELAY ';'				{$$ = new delay_statement();}

compound_statement: '{' '}'					{$$ = new compound_statement(NULL,NULL);}
		|	'{' declarations '}'			{$$ = new compound_statement($2,NULL);}
		|	'{' statements '}'				{$$ = new compound_statement(NULL,$2);}
		|	'{' declarations statements '}'	{$$ = new compound_statement($2,$3);}

seq_statement: compound_statement			{$$ = new seq_statement($1);}
		|	SEQ compound_statement			{$$ = new seq_statement($2);}

par_statement: PAR compound_statement		{$$ = new par_statement($2);}

while_statement:	WHILE '(' expression ')' statement	{$$ = new while_statement($3,$5);}

if_statement:	IF '(' expression ')' statement		{$$ = new if_statement($3,$5,NULL);}

if_else_statement:	IF '(' expression ')' statement ELSE statement	{$$ = new if_statement($3,$5,$7);}

assignment_statement:	IDENTIFIER '=' expression ';'	{$$ = new plain_assignment_statement($1,$3);}
				|	IDENTIFIER '[' expression ']' '=' expression ';' {$$ = new array_assignment_statement($1,$3,$6);}

expression:		IDENTIFIER					{$$ = new identifier_expression($1);}
		|		NUMBER						{$$ = new number_expression($1);}
		|		'(' expression ')'			{$$ = $2;}
		|		expression '+' expression	{$$ = new add_expression($1,$3);}
		|		expression '-' expression	{$$ = new subtract_expression($1,$3);}
		|		expression '@' expression	{$$ = new cat_expression($1,$3);}
		|		expression '[' expression ']'	{$$ = new select_expression($1,$3,$3);}
		|		expression '[' expression ':' expression ']'	{$$ = new select_expression($1,$3,$5);}
		|		expression '&' expression	{$$ = new bitand_expression($1,$3);}
		|		expression '|' expression	{$$ = new bitor_expression($1,$3);}
		|		expression '^' expression	{$$ = new bitxor_expression($1,$3);}
		|		expression EQUAL_OP	expression		{$$ = new equal_expression($1,$3);}
		|		expression UNEQUAL_OP expression	{$$ = new unequal_expression($1,$3);}
		|		expression BOOL_OR expression		{$$ = new boolor_expression($1,$3);}
		|		expression BOOL_AND expression		{$$ = new booland_expression($1,$3);}
		|		'~' expression				{$$ = new com_expression($2);}
		|		'-' expression %prec NEG	{$$ = new neg_expression($2);}

expression_list:	expression				{$$ = new expression_list($1);}
		|	expression_list ',' expression	{$1->push_back($3); $$ = $1;}

pin_list:	NUMBER							{$$ = new pin_list($1);}
		|	pin_list ',' NUMBER				{$1->push_back($3); $$ = $1;}

%%

void yyerror(char *s)
{
	printf(s);
}