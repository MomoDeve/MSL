#pragma once

#include "codeGenerator.h"
#include "token.h"
#include "function.h"

#include <vector>
#include <memory>
#include <stack>
#include <ostream>

namespace MSL
{
	namespace compiler
	{
		template<typename T>
		using unique_ptr = std::unique_ptr<T>;

		/*
		generates bytecode for each of expressionList expressions
		*/
		void GenerateExpressionListBytecode(const ExpressionList& list, CodeGenerator& code, const Function& function);

		/*
		binary expression (such as +, -, *, etc)
		*/
		struct BinaryExpression : public BaseExpression
		{
			Token::Type expressionType;
			unique_ptr<BaseExpression> left;
			unique_ptr<BaseExpression> right;
			void Print(std::ostream& out, int depth = 0) const override;

			BinaryExpression();

			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
		};

		/*f
		unary expression (such as -, +)
		*/
		struct UnaryExpression : public BaseExpression
		{
			Token::Type expressionType;
			unique_ptr<BaseExpression> expression;
			void Print(std::ostream& out, int depth = 0) const override;

			UnaryExpression();

			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
		};

		/*
		for expression (with iterator and predicate)
		*/
		struct ForExpression : public BaseExpression
		{
			ExpressionList body;
			unique_ptr<BaseExpression> init;
			unique_ptr<BaseExpression> predicate;
			unique_ptr<BaseExpression> iteration;

			void Print(std::ostream& out, int depth = 0) const override;
			bool hasIterationStatement() const;
			bool hasInitStatement() const;

			ForExpression();

			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
		};

		/*
		while expression (with predicate)
		*/
		struct WhileExpression : public BaseExpression
		{
			ExpressionList body;
			unique_ptr<BaseExpression> predicate;

			void Print(std::ostream& out, int depth = 0) const override;

			WhileExpression();

			virtual void GenerateBytecode(CodeGenerator & code, const Function& function) const override;
		};

		/*
		if expression (with elif and else statements)
		*/
		struct IfExpression : public BaseExpression
		{
			ExpressionList ifStatements;
			std::vector<ExpressionList> bodies;

			bool hasElseBlock() const;
			void Print(std::ostream& out, int depth = 0) const override;
			ExpressionList& getElseBlock();

			IfExpression();


			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
		};

		/*
		call expression (used for function, 'hasParent' indicates existance of caller)
		*/
		struct CallExpression : public BaseExpression
		{
			bool hasParent = false;
			std::string functionName;
			ExpressionList parameters;
			void Print(std::ostream& out, int depth = 0) const override;

			CallExpression();

			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
		};

		/*
		index expression (operator [] with only one parameter)
		*/
		struct IndexExpression : public BaseExpression
		{
			unique_ptr<BaseExpression> caller;
			unique_ptr<BaseExpression> parameter;
			void Print(std::ostream& out, int depth = 0) const override;

			IndexExpression();

			virtual void GenerateBytecode(CodeGenerator & code, const Function & function) const override;
		};

		/*
		object expression (just its name and type in case of constant)
		*/
		struct ObjectExpression : public BaseExpression
		{
			Token object = Token(Token::Type::ERROR, "unnamed");

			ObjectExpression();

			void Print(std::ostream& out, int depth = 0) const override;
			virtual void GenerateBytecode(CodeGenerator & code, const Function & function) const override;
		};

		/*
		object declare (object name and assignment pointer)
		*/
		struct ObjectDeclareExpression : public BaseExpression
		{
			std::string objectName;
			unique_ptr<BaseExpression> assignment;
			bool isConst = false;

			ObjectDeclareExpression();

			bool hasAssignment() const;
			void Print(std::ostream& out, int depth = 0) const override;

			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
		};

		/*
		Currently unused in compiler code
		*/
		struct LambdaExpression : public BaseExpression
		{
			std::vector<std::string> params;
			ExpressionList body;

			void Print(std::ostream& out, int depth = 0) const override;

			LambdaExpression();


			virtual void GenerateBytecode(CodeGenerator & code, const Function & function) const override;
		};

		/*
		foreach exptression (with iterator over container)
		object should have begin(), next() and end() methods
		*/
		struct ForeachExpression : public BaseExpression
		{
			std::string iterator;
			std::string iteratorIndex;
			unique_ptr<BaseExpression> container;
			ExpressionList body;

			void Print(std::ostream& out, int depth = 0) const override;

			ForeachExpression();


			virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
		};

		/*
		return expression (with pointer to return value)
		*/
		struct ReturnExpression : public BaseExpression
		{
			unique_ptr<BaseExpression> returnValue;
			void Print(std::ostream& out, int depth = 0) const override;
			bool Empty() const;

			ReturnExpression();

			virtual void GenerateBytecode(CodeGenerator & code, const Function & function) const override;
		};
	}
}