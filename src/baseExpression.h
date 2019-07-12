#pragma once

#include <ostream>
#include <vector>

#include "instruction.h"

struct BaseExpression
{
	virtual void Print(std::ostream& out, int depth = 0) const = 0;
	virtual void GenerateBytecode(InstructionVector& bytecode) const = 0;
	virtual ~BaseExpression();
};

typedef std::vector<std::unique_ptr<BaseExpression> > ExpressionList;