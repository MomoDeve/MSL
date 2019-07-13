#pragma once

#include "codeGenerator.h"
#include "token.h"
#include "function.h"

#include <vector>
#include <memory>
#include <stack>
#include <ostream>

template<typename T>
using unique_ptr = std::unique_ptr<T>;

void GenerateExpressionListBytecode(const ExpressionList& list, CodeGenerator& code, const Function& function);

struct BinaryExpression : public BaseExpression
{
	Token::Type type;
	unique_ptr<BaseExpression> left;
	unique_ptr<BaseExpression> right;
	void Print(std::ostream& out, int depth = 0) const override;

	BinaryExpression();

	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
};

struct UnaryExpression : public BaseExpression
{
	Token::Type type;
	unique_ptr<BaseExpression> expression;
	void Print(std::ostream& out, int depth = 0) const override;

	UnaryExpression();

	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
};

struct ForExpression : public BaseExpression, public ControlAttribute
{
	ExpressionList body;
	unique_ptr<BaseExpression> init;
	unique_ptr<BaseExpression> predicate;
	unique_ptr<BaseExpression> iteration;

	void Print(std::ostream& out, int depth = 0) const override;
	bool hasIterationStatement() const;

	ForExpression();

	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
};

struct WhileExpression : public BaseExpression, public ControlAttribute
{
	ExpressionList body;
	unique_ptr<BaseExpression> predicate;

	void Print(std::ostream& out, int depth = 0) const override;

	WhileExpression();

	virtual void GenerateBytecode(CodeGenerator & code, const Function& function) const override;
};

struct IfExpression : public BaseExpression, public ControlAttribute
{
	ExpressionList ifStatements;
	std::vector<ExpressionList> bodies;

	bool hasElseBlock() const;
	void Print(std::ostream& out, int depth = 0) const override;
	ExpressionList& getElseBlock();

	IfExpression();

	// Унаследовано через BaseExpression
	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
};

struct CallExpression : public BaseExpression
{
	bool hasParent = false;
	std::string functionName;
	ExpressionList parameters;
	void Print(std::ostream& out, int depth = 0) const override;

	CallExpression();

	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
};

struct IndexExpression : public BaseExpression
{
	std::string objectName;
	unique_ptr<BaseExpression> parameter;
	void Print(std::ostream& out, int depth = 0) const override;

	IndexExpression();

	virtual void GenerateBytecode( CodeGenerator & code, const Function & function) const override;
};

struct ObjectExpression : public BaseExpression
{
	Token object = Token(Token::Type::ERROR, "unnamed");
	void Print(std::ostream& out, int depth = 0) const override;
	virtual void GenerateBytecode( CodeGenerator & code, const Function & function) const override;
};

struct ObjectDeclareExpression : public BaseExpression
{
	std::string objectName;
	unique_ptr<BaseExpression> assignment;
	bool isConst;

	ObjectDeclareExpression();

	bool hasAssignment() const;
	void Print(std::ostream& out, int depth = 0) const override;

	virtual void GenerateBytecode( CodeGenerator& code, const Function& function) const override;
};

struct LambdaExpression : public BaseExpression
{
	std::vector<std::string> params;
	ExpressionList body;

	void Print(std::ostream& out, int depth = 0) const override;

	LambdaExpression();

	// Унаследовано через BaseExpression
	virtual void GenerateBytecode( CodeGenerator & code, const Function & function) const override;
};

struct ForeachExpression : public BaseExpression, public ControlAttribute
{
	std::string iterator;
	unique_ptr<BaseExpression> container;
	ExpressionList body;

	void Print(std::ostream& out, int depth = 0) const override;

	ForeachExpression();

	// Унаследовано через BaseExpression
	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const override;
};

struct ReturnExpression : public BaseExpression
{
	unique_ptr<BaseExpression> returnValue;
	void Print(std::ostream& out, int depth = 0) const override;
	bool Empty() const;

	ReturnExpression();

	virtual void GenerateBytecode(CodeGenerator & code, const Function & function) const override;
};