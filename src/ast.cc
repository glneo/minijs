/*
* CS352 Spring 2015
* Semantic actions to build an AST
* Andrew F. Davis
*/

#include "ast.hh"

#include "miniscript.hh"
#include "runtime.hh"

using namespace std;

DocumentWrite::DocumentWrite(std::list<Expression*>* parameters, int lineNumber) :
	Statement(lineNumber), parameters(parameters)
{
	rdprintf("DocumentWrite: %d\n", lineNumber);
}

Declaration::Declaration(Expression* variable, int lineNumber) :
	Statement(lineNumber), variable(variable)
{
	rdprintf("Declaration: %d\n", lineNumber);
}

Declaration::Declaration(Expression* variable, Expression* expression, int lineNumber) :
	Statement(lineNumber), variable(variable), expression(expression)
{
	rdprintf("Declaration: %d\n", lineNumber);
}

Declaration::Declaration(Expression* variable, std::list<Statement*>* object_init, int lineNumber) :
	Statement(lineNumber), variable(variable), object_init(object_init)
{
	rdprintf("Declaration: %d\n", lineNumber);
}

Declaration::Declaration(Expression* variable, std::list<Expression*>* array_init, int lineNumber) :
	Statement(lineNumber), variable(variable), array_init(array_init)
{
	rdprintf("Declaration: %d\n", lineNumber);
}

Assignment::Assignment(Expression* variable, Expression* expression, int lineNumber) :
	Statement(lineNumber), variable(variable), expression(expression)
{
	rdprintf("Assignment: %d\n", lineNumber);
}

Conditional::Conditional(Expression* condition,
	std::list<Statement*>* ifTrue,
	std::list<Statement*>* ifFalse,
	int lineNumber) :
	Statement(lineNumber), condition(condition), ifTrue(ifTrue), ifFalse(ifFalse)
{
	rdprintf("Conditional: %d\n", lineNumber);
}

Iterator::Iterator(Expression* condition,
	std::list<Statement*>* whileTrue,
	bool testFirst,
	int lineNumber) :
	Statement(lineNumber), condition(condition), whileTrue(whileTrue), testFirst(testFirst)
{
	rdprintf("Iterator: %d\n", lineNumber);
}

Function::Function(std::string name,
	std::list<std::string>* func_params,
	std::list<Statement*>* body,
	int lineNumber) :
	Statement(lineNumber), name(name), func_params(func_params), body(body)
{
	rdprintf("Function: %d\n", lineNumber);
	// add this function to the global context
	shared_ptr<Symbol> newSymbol(new Symbol());
	newSymbol->type = Symbol::FUNCTION;
	newSymbol->declared = true;
	newSymbol->assigned = true;
	newSymbol->function = this;
	(*globalContext)[name] = newSymbol;
}

Call::Call(Expression* callable, int lineNumber) :
	Statement(lineNumber), callable(callable)
{
	rdprintf("Call: %d\n", lineNumber);
}

Break::Break(int lineNumber) :
	Statement(lineNumber)
{
	rdprintf("Break: %d\n", lineNumber);
}

Continue::Continue(int lineNumber) :
	Statement(lineNumber)
{
	rdprintf("Continue: %d\n", lineNumber);
}

Return::Return(Expression* ret, int lineNumber) :
	Statement(lineNumber), ret(ret)
{
	rdprintf("Return: %d\n", lineNumber);
}

Constant::Constant(int lineNumber) : Expression(lineNumber)
{
	rdprintf("Constant: %d\n", lineNumber);
}

IntConst::IntConst(int value, int lineNumber) : Constant(lineNumber)
{
	symbol.type = Symbol::INTEGER;
	symbol.int_value = value;
}

StringConst::StringConst(std::string value, int lineNumber) : Constant(lineNumber)
{
	symbol.type = Symbol::STRING;
	symbol.string_value = value;
}

BRConst::BRConst(int lineNumber) : Constant(lineNumber)
{
	symbol.type = Symbol::BRTAG;
}

BoolConst::BoolConst(bool truth, int lineNumber) : Constant(lineNumber)
{
	symbol.type = Symbol::BOOLEAN;
	symbol.bool_value = truth;
}

Variable::Variable(std::string name, int lineNumber) :
	Expression(lineNumber), name(name)
{
	rdprintf("Variable: %d\n", lineNumber);
}

Variable::Variable(std::string name, std::string object_name, int lineNumber) :
	Expression(lineNumber), name(name), object_name(object_name)
{
	rdprintf("Variable: %d\n", lineNumber);
}

Variable::Variable(std::string name, Expression* index, int lineNumber) :
	Expression(lineNumber), name(name), index(index)
{
	rdprintf("Variable: %d\n", lineNumber);
}

Operation::Operation(OpType opType, Expression* left, Expression* right, int lineNumber) :
	Expression(lineNumber), opType(opType), left(left), right(right)
{
	rdprintf("Operation: %d\n", lineNumber);
}

Negate::Negate(Expression* right, int lineNumber) :
	Expression(lineNumber), right(right)
{
	rdprintf("Negate: %d\n", lineNumber);
}

Callable::Callable(std::string name, std::list<Expression*>* parameters, int lineNumber) :
	Expression(lineNumber), name(name), parameters(parameters)
{
	rdprintf("Callable: %d\n", lineNumber);
}
