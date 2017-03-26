/*
 * CS352 Spring 2015
 * parser.y: Parser for miniscript interpreter
 * Andrew F. Davis
 */

%{
#include "miniscript.hh"
#include "ast.hh"
#include "runtime.hh"
void yyerror(std::list<Statement*>* &program, const char * s);
int yylex();
%}

%parse-param { std::list<Statement*>* &program }

%union {
	char* string_val;
	int int_val;
	Expression* expression;
	Statement* statement;
	std::list<Expression*>* expression_list;
	std::list<Statement*>* statement_list;
	std::list<std::string>* identifier_list;
}

%locations

%token <string_val> STRING ID
%token <int_val> INTEGER
%token START_TAG STOP_TAG DOC_WRITE VAR NEWLINE COLON SEMICOLON BRTAG
%token IF ELSE WHILE DO BREAK CONTINUE TRUE FALSE GT LT GE LE NE EQ OR AND NOT
%token FUNCTION RETURN ASSERT

%type <expression> expression or_expression and_expression 
%type <expression> equality_expression comparison_expression addsub_expression
%type <expression> multidiv_expression negation_expression single_expression 
%type <expression> identifier singleid array_field function_call
%type <statement> statement statementline assignment_statement declaration_statement object_field
%type <statement> if_statement while_statement do_while_statement function_statement
%type <expression_list> parameters array_init array_fields
%type <statement_list> statements statementlines else_if_statement else_statement object_init object_fields
%type <identifier_list> func_params

%%

file:
		optnewlines START_TAG program STOP_TAG optnewlines
		;

program:
		statements                                       { program = $1; }
		;

statements:
		optnewlines statementlines                       { $$ = $2; }
		| optnewlines /* Empty statements */             { $$ = new std::list<Statement*>(); }
		;

statementlines:
		statementline optnewlines                        { $$ = new std::list<Statement*>(1, $1); }
		| statementlines statementline optnewlines       { $1->push_back($2); $$ = $1; }
		;

statementline:
		statement NEWLINE                                { $$ = $1; }
		| statement SEMICOLON                            { $$ = $1; }
		;

optnewlines:
		optnewlines NEWLINE
		| /* or no new line */
		;

statement:
		DOC_WRITE '(' parameters ')'                     { $$ = new DocumentWrite($3, @1.first_line); }
		| assignment_statement                           { $$ = $1; }
		| declaration_statement                          { $$ = $1; }
		| if_statement                                   { $$ = $1; }
		| while_statement                                { $$ = $1; }
		| do_while_statement                             { $$ = $1; }
		| function_statement                             { $$ = new Nop(@1.first_line); /* dirty hack */ }
		| function_call                                  { $$ = new Call($1, @1.first_line); }
		| BREAK                                          { $$ = new Break(@1.first_line); }
		| CONTINUE                                       { $$ = new Continue(@1.first_line); }
		| RETURN expression                              { $$ = new Return($2, @1.first_line); }
//		| ASSERT '(' expression ')'                      { $$ = new Assertion($3, @1.first_line); }
		;

assignment_statement:
		identifier '=' expression                        { $$ = new Assignment($1, $3, @2.first_line); }
		;

declaration_statement:
		VAR singleid                                     { $$ = new Declaration($2, @1.first_line); }
		| VAR singleid '=' expression                    { $$ = new Declaration($2, $4, @1.first_line); }
		| VAR singleid '=' object_init                   { $$ = new Declaration($2, $4, @1.first_line); }
		| VAR singleid '=' array_init                    { $$ = new Declaration($2, $4, @1.first_line); }
		;

if_statement:
		IF '(' expression ')' '{' NEWLINE statements '}'
		                                                 { $$ = new Conditional($3, $7, new std::list<Statement*>(), @1.first_line); }
		| IF '(' expression ')' '{' NEWLINE statements '}' else_if_statement
		                                                 { $$ = new Conditional($3, $7, $9, @1.first_line); }
		;

else_if_statement:
		ELSE if_statement                                { $$ = new std::list<Statement*>(1, $2); }
		| else_statement                                 { $$ = $1; }
		;

else_statement:
		ELSE '{' NEWLINE statements '}'                  { $$ = $4; }
		;

while_statement:
		WHILE '(' expression ')' '{' NEWLINE statements '}'
		                                                 { $$ = new Iterator($3, $7, true, @1.first_line); }
		;

do_while_statement:
		DO '{' NEWLINE statements '}' NEWLINE WHILE '(' expression ')'
		                                                 { $$ = new Iterator($9, $4, false, @1.first_line); }
		;

function_statement:
		FUNCTION ID '(' func_params ')' '{' NEWLINE statements '}'
		                                                 { $$ = new Function(std::string($2), $4, $8, @1.first_line); free($2); }
		;

parameters:
		expression                                       { $$ = new std::list<Expression*>(1, $1); }
		| parameters ',' expression                      { $1->push_back($3); $$ = $1; }
		| /* Empty parameter */                          { $$ = new std::list<Expression*>(); }
		;

func_params:
		ID                                               { $$ = new std::list<std::string>(1, std::string($1)); free($1); }
		| func_params ',' ID                             { $1->push_back(std::string($3)); $$ = $1; free($3); }
		| /* Empty func_param */                         { $$ = new std::list<std::string>(); }

identifier:
		ID                                               { $$ = new Variable(std::string($1), @1.first_line); free($1); }
		| ID '.' ID                                      { $$ = new Variable(std::string($1), std::string($3), @1.first_line); free($1); }
		| ID '[' expression ']'                          { $$ = new Variable(std::string($1), $3, @1.first_line); free($1); }
		;

singleid:
		ID                                               { $$ = new Variable(std::string($1), @1.first_line); free($1); }
		;

expression:
		or_expression                                    { $$ = $1; }
		;

or_expression:
		and_expression                                   { $$ = $1; }
		| or_expression OR and_expression                { $$ = new Operation(Operation::OR, $1, $3, @2.first_line); }
		;

and_expression:
		equality_expression                              { $$ = $1; }
		| and_expression AND equality_expression         { $$ = new Operation(Operation::AND, $1, $3, @2.first_line); }
		;

equality_expression:
		comparison_expression                            { $$ = $1; }
		| equality_expression NE comparison_expression   { $$ = new Operation(Operation::NE, $1, $3, @2.first_line); }
		| equality_expression EQ comparison_expression   { $$ = new Operation(Operation::EQ, $1, $3, @2.first_line); }
		;

comparison_expression:
		addsub_expression                                { $$ = $1; }
		| comparison_expression GT addsub_expression     { $$ = new Operation(Operation::GT, $1, $3, @2.first_line); }
		| comparison_expression LT addsub_expression     { $$ = new Operation(Operation::LT, $1, $3, @2.first_line); }
		| comparison_expression GE addsub_expression     { $$ = new Operation(Operation::GE, $1, $3, @2.first_line); }
		| comparison_expression LE addsub_expression     { $$ = new Operation(Operation::LE, $1, $3, @2.first_line); }
		;

addsub_expression:
		multidiv_expression                              { $$ = $1; }
		| addsub_expression '+' multidiv_expression      { $$ = new Operation(Operation::ADDITION, $1, $3, @2.first_line); }
		| addsub_expression '-' multidiv_expression      { $$ = new Operation(Operation::SUBTRACTION, $1, $3, @2.first_line); }
		;

multidiv_expression:
		negation_expression                              { $$ = $1; }
		| multidiv_expression '*' negation_expression    { $$ = new Operation(Operation::MULTIPLICATION, $1, $3, @2.first_line); }
		| multidiv_expression '/' negation_expression    { $$ = new Operation(Operation::DIVISION, $1, $3, @2.first_line); }
		;

negation_expression:
		single_expression                                { $$ = $1; }
		| NOT single_expression                          { $$ = new Negate($2, @1.first_line); }
		;

single_expression:
		INTEGER                                          { $$ = new IntConst($1, @1.first_line); }
		| STRING                                         { $$ = new StringConst(std::string($1), @1.first_line); free($1); }
		| BRTAG                                          { $$ = new BRConst(@1.first_line); }
		| TRUE                                           { $$ = new BoolConst(true, @1.first_line); }
		| FALSE                                          { $$ = new BoolConst(false, @1.first_line); }
		| identifier                                     { $$ = $1; }
		| function_call                                  { $$ = $1; }
		| '(' expression ')'                             { $$ = $2; }
		;

function_call:
		ID '(' parameters ')'                            { $$ = new Callable(std::string($1), $3, @1.first_line); free($1); }
		;

object_init:
		'{' '}'                                          { $$ = new std::list<Statement*>(); }
		| '{' optnewlines object_fields optnewlines '}'  { $$ = $3; }
		;

object_fields:
		object_field                                     { $$ = new std::list<Statement*>(1, $1); }
		| object_fields ',' optnewlines object_field     { $1->push_back($4); $$ = $1; }
		;

object_field:
		identifier COLON expression                      { $$ = new Declaration($1, $3, @2.first_line); }
		;

array_init:
		'[' ']'                                          { $$ = new std::list<Expression*>(); }
		| '[' optnewlines array_fields optnewlines ']'   { $$ = $3; }
		;

array_fields:
		array_field                                      { $$ = new std::list<Expression*>(1, $1); }
		| array_fields ',' optnewlines array_field       { $1->push_back($4); $$ = $1; }
		;

array_field:
		expression                                       { $$ = $1; }
		;
