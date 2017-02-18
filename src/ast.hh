/*
 * CS352 Spring 2015
 * Semantic actions to build an AST
 * Andrew F. Davis
 */

#ifndef _AST_H
#define _AST_H

#include <string>
#include <map>
#include <list>
#include <vector>
#include <memory>

class Symbol;
class Function;

typedef std::map<std::string, std::shared_ptr<Symbol>> Context;
typedef std::shared_ptr<Context> ContextPtr;
typedef std::vector<std::shared_ptr<Symbol>> Array;

class Symbol
{
public:
	enum Type {
		STRING,
		INTEGER,
		BRTAG,
		BOOLEAN,
		OBJECT,
		ARRAY,
		FUNCTION,
		UNDEFINED
	} type;

	bool declared;
	bool assigned;

	int int_value;
	std::string string_value;
	bool bool_value;
	ContextPtr object;
	Array array;
	Function* function = NULL;

	Symbol() :
		type(UNDEFINED),
		declared(false),
		assigned(false),
		int_value(0),
		bool_value(false),
		object(std::make_shared<Context>()) {}
};

class Statement
{
public:
	int lineNumber;
	bool errorReported;

	Statement(int lineNumber) : lineNumber(lineNumber), errorReported(false) {};
	virtual ~Statement() {};

	/* execute this statment */
	virtual void execute(ContextPtr context) = 0;
};

class Expression
{
public:
	int lineNumber;
	/* Local symbol for runtime info */
	Symbol symbol;

	Expression(int lineNumber) : lineNumber(lineNumber) {};
	virtual ~Expression() { };

	/* evaluate this expression */
	virtual void evaluate(ContextPtr context, bool &errorReported) = 0;
};

class DocumentWrite : public Statement
{
	std::list<Expression*>* parameters = NULL;
public:
	DocumentWrite(std::list<Expression*>* parameters, int lineNumber);

	~DocumentWrite()
	{
		if (parameters)
		{
			for (auto &it : *parameters) delete it;
			delete parameters;
		}
	}

	void execute(ContextPtr context);
};

class Declaration : public Statement
{
	Expression* variable = NULL;
	Expression* expression = NULL;
	std::list<Statement*>* object_init = NULL;
	std::list<Expression*>* array_init = NULL;
public:
	Declaration(Expression* variable, int lineNumber);
	Declaration(Expression* variable, Expression* expression, int lineNumber);
	Declaration(Expression* variable, std::list<Statement*>* object_init, int lineNumber);
	Declaration(Expression* variable, std::list<Expression*>* array_init, int lineNumber);

	~Declaration()
	{
		if (variable)
			delete variable;
		if (expression)
			delete expression;
		if (object_init)
		{
			for (auto &it : *object_init) delete it;
			delete object_init;
		}
		if (array_init)
		{
			for (auto &it : *array_init) delete it;
			delete array_init;
		}
	}

	void execute(ContextPtr context);
};

class Assignment : public Statement
{
	Expression* variable = NULL;
	Expression* expression = NULL;
public:
	Assignment(Expression* variable, Expression* expression, int lineNumber);

	~Assignment()
	{
		if (variable)
			delete variable;
		if (expression)
			delete expression;
	}

	void execute(ContextPtr context);
};

class Conditional : public Statement
{
	Expression* condition = NULL;
	std::list<Statement*>* ifTrue = NULL;
	std::list<Statement*>* ifFalse = NULL;
public:
	Conditional(Expression* condition,
		std::list<Statement*>* ifTrue,
		std::list<Statement*>* ifFalse,
		int lineNumber);

	~Conditional()
	{
		if (condition)
			delete condition;
		if (ifTrue)
		{
			for (auto &it : *ifTrue) delete it;
			delete ifTrue;
		}
		if (ifFalse)
		{
			for (auto &it : *ifFalse) delete it;
			delete ifFalse;
		}
	}

	void execute(ContextPtr context);
};

class Iterator : public Statement
{
	Expression* condition = NULL;
	std::list<Statement*>* whileTrue = NULL;
	bool testFirst;
public:
	Iterator(Expression* condition,
		std::list<Statement*>* whileTrue,
		bool testFirst,
		int lineNumber);

	~Iterator()
	{
		if (condition)
			delete condition;
		if (whileTrue)
		{
			for (auto &it : *whileTrue) delete it;
			delete whileTrue;
		}
	}

	void execute(ContextPtr context);
};

class Nop : public Statement
{
public:
	Nop(int lineNumber) : Statement(lineNumber) {};

	void execute(ContextPtr context) {}
};

class Function : public Statement
{
	std::string name;
	std::list<std::string>* func_params = NULL;
	std::list<Statement*>* body = NULL;
public:
	Function(std::string name,
		std::list<std::string>* func_params,
		std::list<Statement*>* body,
		int lineNumber);

	~Function()
	{
		if (func_params)
			delete func_params;
		if (body)
		{
			for (auto &it : *body) delete it;
			delete body;
		}
	}

	unsigned int getNumberOfArgs() { return func_params->size(); }

	void execute(ContextPtr context);
};

class Call : public Statement
{
	Expression* callable;
public:
	Call(Expression* callable, int lineNumber);

	~Call()
	{
		if (callable)
			delete callable;
	}

	void execute(ContextPtr context);
};

class Break : public Statement
{
public:
	Break(int lineNumber);

	void execute(ContextPtr context);
};

class Continue : public Statement
{
public:
	Continue(int lineNumber);

	void execute(ContextPtr context);
};

class Return : public Statement
{
	Expression* ret;
public:
	Return(Expression* ret, int lineNumber);

	~Return()
	{
		if (ret)
			delete ret;
	}

	void execute(ContextPtr context);
};

class Constant : public Expression
{
public:
	Constant(int lineNumber);

	/* constants are already final */
	void evaluate(ContextPtr context, bool &errorReported) { return; }
};

class IntConst : public Constant
{
public:
	IntConst(int value, int lineNumber);
};

class StringConst : public Constant
{
public:
	StringConst(std::string value, int lineNumber);
};

class BRConst : public Constant
{
public:
	BRConst(int lineNumber);
};

class BoolConst : public Constant
{
public:
	BoolConst(bool truth, int lineNumber);
};

class Variable : public Expression
{
public:
	std::string name;
	std::string object_name;
	Expression* index = NULL;

	Variable(std::string name, int lineNumber);
	Variable(std::string name, std::string object_name, int lineNumber);
	Variable(std::string name, Expression* index, int lineNumber);

	~Variable()
	{
		if (index)
			delete index;
	}

	void evaluate(ContextPtr context, bool &errorReported);
	void declare(ContextPtr context, bool &errorReported);
	/* note: expression here is assumed to have been previously evaluated */
	void assign(ContextPtr context, Expression* expression, bool &errorReported);
};

class Operation : public Expression
{
public:
	enum OpType {
		GT, LT, GE, LE,
		NE, EQ, OR, AND,
		ADDITION,
		SUBTRACTION,
		MULTIPLICATION,
		DIVISION
	} opType;

	Expression* left;
	Expression* right;

	Operation(OpType opType, Expression* left, Expression* right, int lineNumber);

	~Operation()
	{
		if (left)
			delete left;
		if (right)
			delete right;
	}

	void evaluate(ContextPtr context, bool &errorReported);
};

class Negate : public Expression
{
public:
	Expression* right;

	Negate(Expression* right, int lineNumber);

	~Negate()
	{
		if (right)
			delete right;
	}

	void evaluate(ContextPtr context, bool &errorReported);
};

class Callable : public Expression
{
public:
	std::string name;
	std::list<Expression*>* parameters = NULL;

	Callable(std::string name, std::list<Expression*>* parameters, int lineNumber);

	~Callable()
	{
		if (parameters)
		{
			for (auto &it : *parameters) delete it;
			delete parameters;
		}
	}

	void evaluate(ContextPtr context, bool &errorReported);
};

#endif // _AST_H
