/*
* CS352 Spring 2015
* Evaluating actions for miniscript
* Andrew F. Davis
*/

#include "runtime.hh"

#include <cstdio>
#include <map>
#include <string>
#include <sstream>

#include "miniscript.hh"
#include "ast.hh"

using namespace std;

void Operation::evaluate(ContextPtr context, bool &errorReported)
{
	// assume we will be undefined
	symbol.type = Symbol::UNDEFINED;

	// evaluate both sides of our operation
	left->evaluate(context, errorReported);

	// check if we can short-circuit evaluate
	if (opType == Operation::AND && getTruth(left, errorReported) == false)
	{
		symbol.bool_value = false;
		symbol.type = Symbol::BOOLEAN;
		return;
	}
	else if (opType == Operation::OR && getTruth(left, errorReported) == true)
	{
		symbol.bool_value = true;
		symbol.type = Symbol::BOOLEAN;
		return;
	}

	// now with function calls (right) may re-evaluate (left)
	// changing its value before we extract it in its current
	// incarnation, so we save (left)'s value here before we call (right)
	shared_ptr<Constant> newLeft(new Constant(0));
	newLeft->symbol.type = left->symbol.type;
	newLeft->symbol.int_value = left->symbol.int_value;
	newLeft->symbol.string_value = left->symbol.string_value;
	newLeft->symbol.bool_value = left->symbol.bool_value;
	newLeft->symbol.object = left->symbol.object;
	newLeft->symbol.array = left->symbol.array;
	newLeft->symbol.function = left->symbol.function;
	newLeft->symbol.declared = left->symbol.declared;
	newLeft->symbol.assigned = left->symbol.assigned;

	right->evaluate(context, errorReported);

	// if one of the sides is undefined
	if (newLeft->symbol.type == Symbol::UNDEFINED ||
		right->symbol.type == Symbol::UNDEFINED)
		return;

	// if the types are not the same
	if (newLeft->symbol.type != right->symbol.type)
	{
		// if both sides are of these types then we get 'truthness'
		if ((newLeft->symbol.type == Symbol::STRING ||
			newLeft->symbol.type == Symbol::INTEGER ||
			newLeft->symbol.type == Symbol::BOOLEAN) &&
			(right->symbol.type == Symbol::STRING ||
			right->symbol.type == Symbol::INTEGER ||
			right->symbol.type == Symbol::BOOLEAN))
		{
			bool leftTruth = getTruth(newLeft, errorReported);
			bool rightTruth = getTruth(right, errorReported);
			switch (opType)
			{
			case Operation::OR:
				symbol.bool_value = (leftTruth || rightTruth);
				symbol.type = Symbol::BOOLEAN;
				break;
			case Operation::AND:
				symbol.bool_value = (leftTruth && rightTruth);
				symbol.type = Symbol::BOOLEAN;
				break;
			default:
				// report type violation
				MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
				break;
			}
			return;
		}
		else
		{
			// report type violation
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			return;
		}
	}

	if (newLeft->symbol.type == Symbol::STRING)
	{
		switch (opType)
		{
		case Operation::ADDITION:
			symbol.string_value = newLeft->symbol.string_value + right->symbol.string_value;
			symbol.type = Symbol::STRING;
			break;
		case Operation::OR:
			symbol.bool_value = (!newLeft->symbol.string_value.empty() || !right->symbol.string_value.empty());
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::AND:
			symbol.bool_value = (!newLeft->symbol.string_value.empty() && !right->symbol.string_value.empty());
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::EQ:
			symbol.bool_value = (newLeft->symbol.string_value == right->symbol.string_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::NE:
			symbol.bool_value = (newLeft->symbol.string_value != right->symbol.string_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		default:
			// otherwise report type violation
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			return;
		}
	}
	else if (newLeft->symbol.type == Symbol::INTEGER)
	{
		switch (opType)
		{
		case Operation::ADDITION:
			symbol.int_value = newLeft->symbol.int_value + right->symbol.int_value;
			symbol.type = Symbol::INTEGER;
			break;
		case Operation::SUBTRACTION:
			symbol.int_value = newLeft->symbol.int_value - right->symbol.int_value;
			symbol.type = Symbol::INTEGER;
			break;
		case Operation::MULTIPLICATION:
			symbol.int_value = newLeft->symbol.int_value * right->symbol.int_value;
			symbol.type = Symbol::INTEGER;
			break;
		case Operation::DIVISION:
			symbol.int_value = newLeft->symbol.int_value / right->symbol.int_value;
			symbol.type = Symbol::INTEGER;
			break;
		case Operation::GT:
			symbol.bool_value = (newLeft->symbol.int_value > right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::LT:
			symbol.bool_value = (newLeft->symbol.int_value < right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::GE:
			symbol.bool_value = (newLeft->symbol.int_value >= right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::LE:
			symbol.bool_value = (newLeft->symbol.int_value <= right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::OR:
			symbol.bool_value = (newLeft->symbol.int_value || right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::AND:
			symbol.bool_value = (newLeft->symbol.int_value && right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::EQ:
			symbol.bool_value = (newLeft->symbol.int_value == right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::NE:
			symbol.bool_value = (newLeft->symbol.int_value != right->symbol.int_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		default:
			// report type violation
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			return;
		}
	}
	else if (newLeft->symbol.type == Symbol::BOOLEAN)
	{
		switch (opType)
		{
		case Operation::OR:
			symbol.bool_value = (newLeft->symbol.bool_value || right->symbol.bool_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::AND:
			symbol.bool_value = (newLeft->symbol.bool_value && right->symbol.bool_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::EQ:
			symbol.bool_value = (newLeft->symbol.bool_value == right->symbol.bool_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		case Operation::NE:
			symbol.bool_value = (newLeft->symbol.bool_value != right->symbol.bool_value);
			symbol.type = Symbol::BOOLEAN;
			break;
		default:
			// report type violation
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			return;
		}
	}
	else
	{
		// all other type-operation combinations are invalid
		MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
		return;
	}
}

void Negate::evaluate(ContextPtr context, bool &errorReported)
{
	// evaluate of our operand
	right->evaluate(context, errorReported);
	// negation only works on truthy types
	if (right->symbol.type != Symbol::BOOLEAN &&
		right->symbol.type != Symbol::INTEGER &&
		right->symbol.type != Symbol::STRING)
	{
		// print an error message if not
		MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
		symbol.type = Symbol::UNDEFINED;
		return;
	}
	// do the negation
	symbol.bool_value = !getTruth(right, errorReported);
	symbol.type = Symbol::BOOLEAN;
}

void Variable::evaluate(ContextPtr context, bool &errorReported)
{
	shared_ptr<Symbol> tableSymbol = getTableSymbol(context, name);

	// first check if it has been declared
	if (!tableSymbol->declared)
	{
		// now we check the global context
		tableSymbol = getTableSymbol(globalContext, name);
		// if it's still not declared there then we error
		if (!tableSymbol->declared)
		{
			// use before being declared is a value error
			MS_ERROR::report(errorReported, MS_ERROR::VALUE, lineNumber, name);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}
	// now we check if it has been previously assigned
	if (!tableSymbol->assigned)
	{
		// print an error message if not
		MS_ERROR::report(errorReported, MS_ERROR::VALUE, lineNumber, name);
		symbol.type = Symbol::UNDEFINED;
		return;
	}

	// check if the variable is an object but not
	// used like one
	if (tableSymbol->type == Symbol::OBJECT)
	{
		// if not referencing a member
		if (object_name.empty())
		{
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}

	// now check to see if it is used like an object
	// note: this could be made into a loop to allow
	// nested objects
	if (!object_name.empty())
	{
		if (tableSymbol->type != Symbol::OBJECT)
		{
			// it's a type violation to use a non
			// object type like an object
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
		// if it is we get our symbol information
		// from the object pointer
		tableSymbol = getTableSymbol(tableSymbol->object, object_name);

		// first check if it has been declared
		if (!tableSymbol->declared)
		{
			// use before being declared is a value error
			MS_ERROR::report(errorReported, MS_ERROR::VALUE, lineNumber, name + "." + object_name);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
		// now we check if it has been previously assigned
		if (!tableSymbol->assigned)
		{
			// print an error message if not
			MS_ERROR::report(errorReported, MS_ERROR::VALUE, lineNumber, name + "." + object_name);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}
	else
	{
		// make sure if it isn't used like an object that it isn't one
		if (tableSymbol->type == Symbol::OBJECT)
		{
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}

	// now check to see if it is used like an object
	// note: this could be made into a loop to allow
	// nested objects
	if (index != NULL)
	{
		// evaluate the index
		index->evaluate(context, errorReported);
		// only integer indexes accepted
		if (index->symbol.type != Symbol::INTEGER)
		{
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}

		// make sure we actualy are an array
		if (tableSymbol->type != Symbol::ARRAY)
		{
			// it's a type violation to use a non
			// array like type like an array
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
		// if we are out of bounds we resize
		if ((unsigned)index->symbol.int_value >= tableSymbol->array.size())
		{
			int resizeAmount = (index->symbol.int_value - tableSymbol->array.size()) + 1;
			for (int i = 0; i < resizeAmount; i++)
				tableSymbol->array.push_back(make_shared<Symbol>());
		}
		// get our symbol from the array for this variable
		tableSymbol = tableSymbol->array[index->symbol.int_value];

		// now we check if it has been previously assigned
		if (!tableSymbol->assigned)
		{
			// print an error message if not
			MS_ERROR::report(errorReported, MS_ERROR::VALUE, lineNumber, name + "[" +
				static_cast<ostringstream*>(&(ostringstream() << (index->symbol.int_value)))->str() +"]");
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}
	else
	{
		// make sure if it isn't used like an array that it isn't one
		if (tableSymbol->type == Symbol::ARRAY)
		{
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}

	// copy from the symbol table
	symbol.type = tableSymbol->type;
	symbol.int_value = tableSymbol->int_value;
	symbol.string_value = tableSymbol->string_value;
	symbol.bool_value = tableSymbol->bool_value;
	symbol.object = tableSymbol->object;
	symbol.array = tableSymbol->array;
}

void Variable::declare(ContextPtr context, bool &errorReported)
{
	// check if the variable has already been declared
//	if ((*context).count(name))
//		delete (*context)[name]; // free its memory
	// add our new variable to the table
	(*context)[name] = make_shared<Symbol>();
	// it has been declared
	(*context)[name]->declared = true;
}

// NOTE: This assumes the expression already has its local
// symbol info filled out ( evaluate called )
void Variable::assign(ContextPtr context, Expression* expression, bool &errorReported)
{
	shared_ptr<Symbol> tableSymbol = getTableSymbol(context, name);
	// now we check if it has been previously declared
	if (!tableSymbol->declared)
	{
		// print an error message if not
		MS_ERROR::report(errorReported, MS_ERROR::UNDECLARED, lineNumber, name);
		// Lab3 part 3.2 states we now consider this variable declared
		tableSymbol->declared = true;
	}

	// now check to see if it is used like an object
	// note: this could be made into a loop to allow
	// nested objects
	if (!object_name.empty())
	{
		if (tableSymbol->type != Symbol::OBJECT)
		{
			// it's a type violation to use a non
			// object like type like an object
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
		// if it is we get our symbol information
		// from the object pointer
		tableSymbol = getTableSymbol(tableSymbol->object, object_name);

		// we do not need to check if it has been declared
		// as object members do not need to be according to
		// the lab specifications; we must also consider the
		// assignment itself as the declaration for later use
		tableSymbol->declared = true;
	}
	else
	{
		// make sure if it isn't used like an object that it isn't one
		if (tableSymbol->type == Symbol::OBJECT)
		{
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}

	// We have been accessed though an index
	if (index != NULL)
	{
		// evaluate the index
		index->evaluate(context, errorReported);
		// only integer indexes accepted
		if (index->symbol.type != Symbol::INTEGER)
		{
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
		// if we are out of bounds we resize
		if ((unsigned)index->symbol.int_value >= tableSymbol->array.size())
		{
			int resizeAmount = (index->symbol.int_value - tableSymbol->array.size()) + 1;
			for (int i = 0; i < resizeAmount; i++)
				tableSymbol->array.push_back(make_shared<Symbol>());
		}
		// get our symbol from the array for this variable
		tableSymbol = tableSymbol->array[index->symbol.int_value];
	}
	else
	{
		// make sure if it isn't used like an array that it isn't one
		if (tableSymbol->type == Symbol::ARRAY)
		{
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}

	// do the actual assignment
	tableSymbol->type = expression->symbol.type;
	tableSymbol->int_value = expression->symbol.int_value;
	tableSymbol->string_value = expression->symbol.string_value;
	tableSymbol->bool_value = expression->symbol.bool_value;
	tableSymbol->object = expression->symbol.object;
	tableSymbol->array = expression->symbol.array;
	// mark the variable as having been assigned
	tableSymbol->assigned = true;
}

void Callable::evaluate(ContextPtr context, bool &errorReported)
{
	// get function pointer out of our symbol table
	shared_ptr<Symbol> tableSymbol = getTableSymbol(context, name);
	// check if the function has been declared
	if (!tableSymbol->declared)
	{
		// now we check the global context
		tableSymbol = getTableSymbol(globalContext, name);
		// if it's still not declared there then we error
		if (!tableSymbol->declared)
		{
			// use before being declared is a type violation
			MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
			symbol.type = Symbol::UNDEFINED;
			return;
		}
	}

	// ensure it is actually a function
	if (tableSymbol->type != Symbol::FUNCTION)
	{
		// it's a type violation to call a variable
		MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
		symbol.type = Symbol::UNDEFINED;
		return;
	}

	// check for matching number of parameters
	if (parameters->size() != (tableSymbol->function)->getNumberOfArgs())
	{
		MS_ERROR::report(errorReported, MS_ERROR::TYPE, lineNumber);
		symbol.type = Symbol::UNDEFINED;
		return;
	}

	// iterate over the parameters
	for (std::list<Expression*>::const_iterator it = parameters->begin(), end = parameters->end(); it != end; ++it)
	{
		// this makes every parameter report independently
		bool paramError = false;
		(*it)->evaluate(context, paramError);
		// now we push the evaluated expression onto the runtime stack
		callStack.push((*it));
	}

	// call the function
	try { (tableSymbol->function)->execute(context); }
	catch (Expression* ret)
	{
		// copy from the return value
		symbol.type = (ret->symbol).type;
		symbol.int_value = (ret->symbol).int_value;
		symbol.string_value = (ret->symbol).string_value;
		symbol.bool_value = (ret->symbol).bool_value;
		symbol.object = (ret->symbol).object;
		symbol.array = (ret->symbol).array;
		symbol.function = (ret->symbol).function; // http://i.imgur.com/eAjDV5C.jpg
		return;
	}

	// if we get this far then the function returned without a return statement
	symbol.type = Symbol::UNDEFINED;
}
