/*
 * CS352 Spring 2015
 * Interpreter for miniscript
 * Andrew F. Davis
 */

#include <cstdio>
#include <cstdlib>

#include "miniscript.hh"
#include "ast.hh"
#include "runtime.hh"

extern FILE *yyin;
int yyparse(std::list<Statement*>* &program);

void yyerror(std::list<Statement*>* &program, const char * s)
{
	fprintf(stderr, "%s\n", s);
	exit(1); /* just end here */
}

int main(int argc, char *argv[])
{
	/* Open program file */
	yyin = fopen(argv[1], "r");
	if (!yyin)
	{
		fprintf(stderr, "couldn't open file for reading\n");
		return 0;
	}

	/* This will contain the top level program statements */
	std::list<Statement*>* program = NULL;

	/* Parse program */
	yyparse(program);

	/* Run program */
	runProgram(program);

	/* delete the AST */
	for (auto &it : *program) delete it;
	delete program;

	return 0;
}
