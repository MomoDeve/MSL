#pragma once

#include <ostream>
#include <vector>

#include "instruction.h"

class CodeGenerator;
class Function;

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

struct BaseExpression
{
	ExpressionType type = ExpressionType::BASE;
	virtual void Print(std::ostream& out, int depth = 0) const = 0;
	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const = 0;
	virtual ~BaseExpression();
};

typedef std::vector<std::unique_ptr<BaseExpression> > ExpressionList;

struct ControlAttribute
{
	static uint16_t id;
	static void Reset();
};