#pragma once

#include "baseExpression.h"
#include "token.h"
#include "function.h"

#include <vector>
#include <memory>
#include <stack>
#include <ostream>

template<typename T>
using unique_ptr = std::unique_ptr<T>;

struct BinaryExpression : BaseExpression
{
	Token::Type type;
	unique_ptr<BaseExpression> left;
	unique_ptr<BaseExpression> right;
	void Print(std::ostream& out, int depth = 0) const override;

	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct NullExpression : public BaseExpression
{
	void Print(std::ostream& out, int depth = 0) const override;

	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct UnaryExpression : public BaseExpression
{
	Token::Type type;
	unique_ptr<BaseExpression> expression;
	void Print(std::ostream& out, int depth = 0) const override;
	
	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct ForExpression : public BaseExpression
{
	ExpressionList body;
	unique_ptr<BaseExpression> init;
	unique_ptr<BaseExpression> predicate;
	unique_ptr<BaseExpression> iteration;

	void Print(std::ostream& out, int depth = 0) const override;

	bool hasIterationStatement() const;

	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct WhileExpression : public BaseExpression
{
	ExpressionList body;
	unique_ptr<BaseExpression> predicate;

	void Print(std::ostream& out, int depth = 0) const override;
	
	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct IfExpression : public BaseExpression
{
	ExpressionList ifStatements;
	std::vector<ExpressionList> bodies;

	bool hasElseBlock() const;
	void Print(std::ostream& out, int depth = 0) const override;
	ExpressionList& getElseBlock();

	
	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct CallExpression : public BaseExpression
{
	std::string functionName;
	ExpressionList parameters;
	void Print(std::ostream& out, int depth = 0) const override;

	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct IndexExpression : public BaseExpression
{
	std::string objectName;
	ExpressionList parameters;
	void Print(std::ostream& out, int depth = 0) const override;
	
	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct ObjectExpression : public BaseExpression
{
	Token object = Token(Token::Type::ERROR, "unnamed");
	void Print(std::ostream& out, int depth = 0) const override;

	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct ObjectDeclareExpression : public BaseExpression
{
	std::string objectName;
	unique_ptr<BaseExpression> assignment;
	bool isConst;

	bool hasAssignment() const;
	void Print(std::ostream& out, int depth = 0) const override;
	
	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct LambdaExpression : public BaseExpression
{
	std::vector<std::string> params;
	ExpressionList body;

	void Print(std::ostream& out, int depth = 0) const override;

	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct ForeachExpression : public BaseExpression
{
	std::string iterator;
	unique_ptr<BaseExpression> container;
	ExpressionList body;

	void Print(std::ostream& out, int depth = 0) const override;

	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};

struct ReturnExpression : public BaseExpression
{
	unique_ptr<BaseExpression> returnValue;
	void Print(std::ostream& out, int depth = 0) const override;
	bool Empty() const;
	
	virtual void GenerateBytecode(InstructionVector& bytecode) const override;
};
