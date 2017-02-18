/*
 * CS352 Spring 2015
 * Runtime actions for miniscript
 * Andrew F. Davis
 */

#ifndef _RUNTIME_H
#define _RUNTIME_H

#include <string>
#include <map>
#include <list>
#include <stack>
#include <memory>

#include "ast.hh"

extern ContextPtr globalContext;

extern std::stack<Expression*> callStack;

class MS_ERROR
{
public:
	enum ERROR_TYPE {
		TYPE,
		VALUE,
		PARAMETER,
		CONDITION,
		UNDECLARED
	};

	static void report(bool &errorReported, ERROR_TYPE type, int lineNumber);
	static void report(bool &errorReported, ERROR_TYPE type, int lineNumber, std::string varName);
};

std::shared_ptr<Symbol> getTableSymbol(ContextPtr context, std::string name);

// Assumes condition has been evaluated first
bool getTruth(Expression* condition, bool &errorReported);
bool getTruth(std::shared_ptr<Expression> condition, bool &errorReported);

void runProgram(std::list<Statement*>* program);

#endif // _RUNTIME_H
