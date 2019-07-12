#pragma once

#include <ostream>
#include <vector>

#include "instruction.h"

class CodeGenerator;
class Function;

struct BaseExpression
{
	virtual void Print(std::ostream& out, int depth = 0) const = 0;
	virtual void GenerateBytecode(CodeGenerator& code, const Function& function) const = 0;
	virtual ~BaseExpression();
};

typedef std::vector<std::unique_ptr<BaseExpression> > ExpressionList;