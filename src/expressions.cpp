#include "expressions.h"
#include "VM.h"

#define TO_BASE(derived_ptr) unique_ptr<BaseExpression>(derived_ptr.release())

bool IfExpression::hasElseBlock() const
{
	return bodies.size() > ifStatements.size();
}

void IfExpression::Print(std::ostream& out, int depth) const
{	
	out << std::string(depth, '\t');
	out << "if (\n";
	ifStatements[0]->Print(out, depth + 1);
	out << std::string(depth, '\t') << ") {\n";
	for (const auto& expr : bodies[0])
	{
		expr->Print(out, depth + 1);
	}
	for (int i = 1; i < (int)ifStatements.size(); i++)
	{
		out << std::string(depth, '\t');
		out << "} else if (\n";
		ifStatements[i]->Print(out, depth + 1);
		out << std::string(depth, '\t') << ") { \n";
		for (const auto& expr : bodies[i])
		{
			expr->Print(out, depth + 1);
		}
	}
	out << std::string(depth, '\t') << "}\n";
	if (hasElseBlock())
	{
		out << std::string(depth, '\t') << "else {\n";
		for (const auto& expr : bodies.back())
		{
			expr->Print(out, depth + 1);
		}
		out << std::string(depth, '\t') << "}\n";
	}
}

ExpressionList& IfExpression::getElseBlock()
{
	return bodies.back();
}

void IfExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

bool ObjectDeclareExpression::hasAssignment() const
{
	return assignment != nullptr;
}

void ObjectDeclareExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t');
	out << (isConst ? "const " : "");
	out << "var " << objectName;
	if (hasAssignment())
	{
		out << " = \n";
		assignment->Print(out, depth + 1);
	}
}

void ObjectDeclareExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

#undef TO_BASE

void ObjectExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t');
	out << object.ToString() << '\n';
}

void ObjectExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void CallExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t');
	out << functionName << '\n';
	out << std::string(depth, '\t') << "(\n";
	for (const auto& param : parameters)
	{
		param->Print(out, depth + 1);
	}
	out << std::string(depth, '\t') << ")\n";
}

void CallExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void ForExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t');
	out << "for (\n";
	out << std::string(depth, '\t') << ">> INIT:\n";
	init->Print(out, depth + 1);
	out << std::string(depth, '\t') << ">> PRED:\n";
	predicate->Print(out, depth + 1);
	out << std::string(depth, '\t') << ">> ITER:\n";
	if (hasIterationStatement())
	{
		iteration->Print(out, depth + 1);
	}
	out << std::string(depth, '\t') << ") {\n";
	for (const auto& expr : body)
	{
		expr->Print(out, depth + 1);
		out << '\n';
	}
	out << std::string(depth, '\t') << "}\n";
}

bool ForExpression::hasIterationStatement() const
{
	return iteration != nullptr;
}

void ForExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void UnaryExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t') << Token::ToString(type) << '\n';
	out << std::string(depth, '\t') << ">> EXPR:\n";
	expression->Print(out, depth + 1);
}

void UnaryExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void BinaryExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t') << Token::ToString(type) << '\n';
	out << std::string(depth, '\t') << ">> LEFT_EXPR:\n";
	left->Print(out, depth + 1);
	out << std::string(depth, '\t') << ">> RIGHT_EXPR:\n";
	right->Print(out, depth + 1);
}

void BinaryExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	left->GenerateBytecode(bytecode);
	right->GenerateBytecode(bytecode);
	if (type & Token::Type::ONE_OPCODE_OPERAND)
	{
		bytecode.emplace_back(VM::GetOPCodeByToken(type));
	}
}

void IndexExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t');
	out << objectName << '\n';
	out << std::string(depth, '\t') << "[\n";
	for (const auto& param : parameters)
	{
		param->Print(out, depth + 1);
	}
	out << std::string(depth, '\t') << "]\n";
}

void IndexExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void WhileExpression::Print(std::ostream & out, int depth) const
{
	out << std::string(depth, '\t');
	out << "while (\n";
	out << std::string(depth, '\t') << ">> PRED:\n";
	predicate->Print(out, depth + 1);
	out << std::string(depth, '\t') << ") {\n";
	for (const auto& expr : body)
	{
		expr->Print(out, depth + 1);
		out << '\n';
	}
	out << std::string(depth, '\t') << "}\n";
}

void WhileExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void NullExpression::Print(std::ostream & out, int depth) const
{
	out << std::string(depth, '\t') << ">> NULL ERROR EXPRESSION\n";
}

void NullExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void LambdaExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t') << "lambda (";
	for (int i = 0; i < (int)params.size(); i++)
	{
		out << params[i];
		if (i != (int)params.size() - 1)
		{
			out << ", ";
		}
	}
	out << ") {\n";
	for (const auto& expr : body)
	{
		expr->Print(out, depth + 1);
		out << '\n';
	}
	out << std::string(depth, '\t') << "}\n";
}

void LambdaExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void ForeachExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t') << "foreach (\n";
	out << std::string(depth, '\t') << ">> ITER:\n";
	out << std::string(depth + 1, '\t') << "var " << iterator << '\n';
	out << std::string(depth, '\t') << ">> CONTAINER:\n";
	container->Print(out, depth + 1);
	out << std::string(depth, '\t') << ") {\n";
	for (const auto& expr : body)
	{
		expr->Print(out, depth + 1);
		out << '\n';
	}
	out << std::string(depth, '\t') << "}\n";
}

void ForeachExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}

void ReturnExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t') << "return:\n";
	if (!Empty()) returnValue->Print(out, depth + 1);
	else out << std::string(depth + 1, '\t') << "VOID\n";
}

bool ReturnExpression::Empty() const
{
	return returnValue == nullptr;
}

void ReturnExpression::GenerateBytecode(InstructionVector& bytecode) const
{
	return void();
}
