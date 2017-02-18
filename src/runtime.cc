/*
 * CS352 Spring 2015
 * Runtime actions for miniscript
 * Andrew F. Davis
 */

#include "runtime.hh"

#include <cstdio>
#include <map>

#include "miniscript.hh"
#include "ast.hh"

using namespace std;

/* generate a new global execution context/symbol table */
ContextPtr globalContext = make_shared<map<string, shared_ptr<Symbol>>>();

/* Used for passing arguments to functions */
stack<Expression*> callStack;

void MS_ERROR::report(bool &errorReported, ERROR_TYPE type, int lineNumber)
{
	report(errorReported, type, lineNumber, "");
}

void MS_ERROR::report(bool &errorReported, ERROR_TYPE type, int lineNumber, std::string varName)
{
	// if we haven't reported an error before
	if (!errorReported)
		switch (type)
		{
			case MS_ERROR::TYPE:
				fprintf(stderr, "Line %d, type violation\n", lineNumber);
				break;
			case MS_ERROR::VALUE:
				fprintf(stderr, "Line %d, %s has no value\n", lineNumber, varName.c_str());
				break;
			case MS_ERROR::PARAMETER:
				fprintf(stderr, "Line %d, unknown parameter type\n", lineNumber);
				break;
			case MS_ERROR::CONDITION:
				fprintf(stderr, "Line %d, condition unknown\n", lineNumber);
				break;
			case MS_ERROR::UNDECLARED:
				fprintf(stderr, "Line %d, %s undeclared\n", lineNumber, varName.c_str());
				break;
			default:
				fprintf(stderr, "\"Unknown error\" error :p\n");
				break;
		}
	// we have now
	errorReported = true;
}

shared_ptr<Symbol> getTableSymbol(ContextPtr context, string name)
{
	// check for the variable in the symbol table
	if (!context->count(name))
	{
		// if not we create it but leave it undeclared
		(*context)[name] = make_shared<Symbol>();
	}
	// return the pointer to the symbol
	return (*context)[name];
}

void runProgram(list<Statement*>* program)
{
	if (program == NULL)
		return;

	/* for each Statement in the program */
	for (list<Statement*>::const_iterator it = program->begin(), end = program->end(); it != end; ++it)
	{
		try { (*it)->execute(globalContext); }
		catch (Statement* s)
		{
			// this happens when a break/continue are
			// used outside of a container
			fprintf(stderr, "Line %d, type violation\n", s->lineNumber);
		}
	}
}
