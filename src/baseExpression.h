#pragma once

#include "opcode.h"
#include <ostream>
#include <vector>

namespace MSL
{
	namespace compiler
	{
		class CodeGenerator;
		class Function;

		/*
		enum for fast cast from base class to derived
		*/
		enum class ExpressionType
		{
			BINARY,
			UNARY,
			WHILE,
			FOR,
			FOREACH,
			IF,
			OBJECT,
			DECLARE,
			RETURN,
			INDEX,
			CALL,
			LAMBDA,
			BASE,
		};

		/*
		BaseExpression is a base class for all types of expressions which parser can generate.
		Each expression class, which derives from BaseExpression,
		should set its type to one of the enum's ExpressionType and override Print() and GenerateBytecode() methods
		*/
		struct BaseExpression
		{
			/*
			type of BaseExpresssion object (or object which derives from BaseExpression)
			*/
			ExpressionType type = ExpressionType::BASE;
			/*
			Print human-read representation of expression to the ostream provided. depth parameter sets the amount of tabs in the beggining of each line
			*/
			virtual void Print(std::ostream& out, int depth = 0) const = 0;
			/*
			Generates bytecode of expression in a recursion way and writes it to the codeGenerator's file. Function is a method, containing this expression
			*/
			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const = 0;
			/*
			virtual destructor of BaseExpression class
			*/
			virtual ~BaseExpression() = default;
		};

		/*
		ExpressionList should be used when expressions are need to be stored in an array
		*/
		typedef std::vector<std::unique_ptr<BaseExpression> > ExpressionList;
	}
}