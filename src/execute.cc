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

void DocumentWrite::execute(ContextPtr context)
{
	// iterate over the parameters
	for (std::list<Expression*>::const_iterator it = parameters->begin(), end = parameters->end(); it != end; ++it)
	{
		// this makes every parameter report independently
		bool paramError = false;
		try { (*it)->evaluate(context, paramError); }
		catch (...) {} // TODO: something...
		switch ((*it)->symbol.type)
		{
		case Symbol::STRING:
			printf("%s", (*it)->symbol.string_value.c_str());
			break;
		case Symbol::INTEGER:
			printf("%d", (*it)->symbol.int_value);
			break;
		case Symbol::BRTAG:
			printf("\n");
			break;
		case Symbol::BOOLEAN:
			printf(((*it)->symbol.bool_value) ? "true" : "false");
			break;
		case Symbol::UNDEFINED:
			printf("undefined");
			break;
		case Symbol::OBJECT:
			// object as a parameter is a type violation
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			// The spec makes no mention of this but
			// TA endorsed piazza post 158 states that
			// "undefined" must follow this error even
			// though the type is not undefined
			printf("undefined");
			break;
		case Symbol::ARRAY:
			// Array as a parameter is a type violation
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			printf("undefined");
			break;
		default:
			MS_ERROR::report(errorReported, MS_ERROR::PARAMETER, lineNumber);
			break;
		}
	}
}

void Declaration::execute(ContextPtr context)
{
	// see if we also have an assignment to perform
	if (expression != NULL)
	{
		try { expression->evaluate(context, errorReported); }
		catch (...) {} // TODO: something...
	}
	dynamic_cast<Variable*>(variable)->declare(context, errorReported);
	// if we are also doing an assignment
	if (expression != NULL)
		dynamic_cast<Variable*>(variable)->assign(context, expression, errorReported);
	else if (object_init != NULL) // if that assignment is for an object
	{
		// we execute each declaration in the initializer list
		// in the context of the object ( the objects symbol table )
		ContextPtr objectContext = getTableSymbol(context, dynamic_cast<Variable*>(variable)->name)->object;
		for (list<Statement*>::const_iterator it = object_init->begin(), end = object_init->end(); it != end; ++it)
			(*it)->execute(objectContext);
		getTableSymbol(context, dynamic_cast<Variable*>(variable)->name)->type = Symbol::OBJECT;
		getTableSymbol(context, dynamic_cast<Variable*>(variable)->name)->assigned = true;
	}
	else if (array_init != NULL) // if that assignment is for an array
	{
		// we evaluate each expression in the initializer list
		// and add them to our array
		for (list<Expression*>::const_iterator it = array_init->begin(), end = array_init->end(); it != end; ++it)
		{
			(*it)->evaluate(context, errorReported);
			shared_ptr<Symbol> localSymbol(new Symbol());
			// assignment
			localSymbol->type = (*it)->symbol.type;
			localSymbol->int_value = (*it)->symbol.int_value;
			localSymbol->string_value = (*it)->symbol.string_value;
			localSymbol->bool_value = (*it)->symbol.bool_value;
			localSymbol->assigned = true;
			getTableSymbol(context, dynamic_cast<Variable*>(variable)->name)->array.push_back(localSymbol);
		}
		getTableSymbol(context, dynamic_cast<Variable*>(variable)->name)->type = Symbol::ARRAY;
		getTableSymbol(context, dynamic_cast<Variable*>(variable)->name)->assigned = true;
	}
}

void Assignment::execute(ContextPtr context)
{
	// evaluate the right hand side expression
	try { expression->evaluate(context, errorReported); }
	catch (...) {} // TODO: something...
	dynamic_cast<Variable*>(variable)->assign(context, expression, errorReported);
}

// Assumes condition has been evaluated first
bool getTruth(Expression* condition, bool &errorReported)
{
	//we get the result based on type
	bool truth;
	switch (condition->symbol.type)
	{
	case Symbol::BOOLEAN:
		truth = condition->symbol.bool_value;
		break;
	case Symbol::INTEGER:
		// true is a non-zero integer like C
		// so we could just cast to bool but
		// I feel pedantic today :)
		truth = (condition->symbol.int_value != 0);
		break;
	case Symbol::STRING:
		// true is a non-empty string
		truth = (condition->symbol.string_value != "");
		break;
	default:
		// all other types are a violation
		MS_ERROR::report(errorReported, MS_ERROR::CONDITION, condition->lineNumber);
		throw new exception; // don't evaluate further
	}
	return truth;
}

bool getTruth(shared_ptr<Expression> condition, bool &errorReported)
{
	//we get the result based on type
	bool truth;
	switch (condition->symbol.type)
	{
	case Symbol::BOOLEAN:
		truth = condition->symbol.bool_value;
		break;
	case Symbol::INTEGER:
		// true is a non-zero integer like C
		// so we could just cast to bool but
		// I feel pedantic today :)
		truth = (condition->symbol.int_value != 0);
		break;
	case Symbol::STRING:
		// true is a non-empty string
		truth = (condition->symbol.string_value != "");
		break;
	default:
		// all other types are a violation
		MS_ERROR::report(errorReported, MS_ERROR::CONDITION, condition->lineNumber);
		throw new exception; // don't evaluate further
	}
	return truth;
}

void Conditional::execute(ContextPtr context)
{
	// get the result
	bool truth = false;
	try
	{
		// start by evaluating the conditional
		condition->evaluate(context, errorReported);
		truth = getTruth(condition, errorReported);
	}
	catch (...)
	{
		return; // this will cause us to just skip the conditional
	}
	if (truth)
	{
		/* for each Statement in the true block */
		for (list<Statement*>::const_iterator it = ifTrue->begin(), end = ifTrue->end(); it != end; ++it)
			(*it)->execute(context);
	}
	else
	{
		/* for each Statement in the false block */
		for (list<Statement*>::const_iterator it = ifFalse->begin(), end = ifFalse->end(); it != end; ++it)
			(*it)->execute(context);
	}
}

void Iterator::execute(ContextPtr context)
{
	try
	{
		// if we evaluate first
		if (testFirst)
		{
			condition->evaluate(context, errorReported);
			if (getTruth(condition, errorReported) == false)
				return; // don't run
		}
		do
		{
			/* for each Statement in the while block */
			for (list<Statement*>::const_iterator it = whileTrue->begin(), end = whileTrue->end(); it != end; ++it)
			{
				try { (*it)->execute(context); }
				catch (Break*) { return; }
				catch (Continue*) { break; } // mind == explode
			}
			// re-evaluate conditional
			condition->evaluate(context, errorReported);
		} while (getTruth(condition, errorReported));
	}
	catch (...)
	{
		return; // this will cause us to just skip the conditional
	}
}

void Function::execute(ContextPtr context)
{
	// make a function local context
	ContextPtr localContext = make_shared<map<string, shared_ptr<Symbol>>>();
	// add arguments on the call stack to this local context
	for (list<std::string>::reverse_iterator it = func_params->rbegin(), end = func_params->rend(); it != end; ++it)
	{
		//FIXME: do we pass by reference or value?
		// assume by value, so make a new symbol and fill it
		shared_ptr<Symbol> newSymbol(new Symbol());
		newSymbol->type = (callStack.top()->symbol).type;
		newSymbol->int_value = (callStack.top()->symbol).int_value;
		newSymbol->string_value = (callStack.top()->symbol).string_value;
		newSymbol->bool_value = (callStack.top()->symbol).bool_value;
		newSymbol->object = (callStack.top()->symbol).object;
		newSymbol->array = (callStack.top()->symbol).array;
		newSymbol->function = (callStack.top()->symbol).function;
		newSymbol->declared = true;
		newSymbol->assigned = true;
		(*localContext)[(*it)] = newSymbol;
		callStack.pop();
	}

	// for each Statement in the function body
	for (list<Statement*>::const_iterator it = body->begin(), end = body->end(); it != end; ++it)
	{
		(*it)->execute(localContext);
		// notice we let exceptions pass up the stack
		// this allows us to catch returns higher up
		// and use our interpreters call stack as our
		// program's call stack
	}
}

void Call::execute(ContextPtr context)
{
	callable->evaluate(context, errorReported);
}

void Break::execute(ContextPtr context)
{
	throw this;
}

void Continue::execute(ContextPtr context)
{
	throw this;
}

void Return::execute(ContextPtr context)
{
	ret->evaluate(context, errorReported);
	throw ret;
}
