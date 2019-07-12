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

void IfExpression::GenerateBytecode(CodeGenerator & code, const Function & function) const
{
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

void ObjectDeclareExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
{
	if (hasAssignment())
	{
		assignment->GenerateBytecode(code, function);
	}
	else
	{
		code.write(OPCODE::PUSH_NULL);
	}
	if (isConst)
	{
		code.write(OPCODE::SET_CONST_VAR);
	}
	else
	{
		code.write(OPCODE::SET_VAR);
	}
	code.write(function.GetHash(objectName));
}

#undef TO_BASE

void ObjectExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t');
	out << object.ToString() << '\n';
}

void ObjectExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
{
	switch (object.type)
	{
	case Token::Type::STRING_CONSTANT:
		code.write(OPCODE::PUSH_STRING);
		break;
	case Token::Type::INTEGER_CONSTANT:
		code.write(OPCODE::PUSH_INTEGER);
		break;
	case Token::Type::FLOAT_CONSTANT:
		code.write(OPCODE::PUSH_FLOAT);
		break;
	case Token::Type::OBJECT:
		code.write(OPCODE::PUSH_OBJECT);
		break;
	case Token::Type::TRUE_CONSTANT:
		code.write(OPCODE::PUSH_TRUE);
		break;
	case Token::Type::FALSE_CONSTANT:
		code.write(OPCODE::PUSH_FALSE);
		break;
	default:
		code.write(OPCODE::ERROR_SYMBOL);
		break;
	}
	code.write(function.GetHash(object.value));
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

void CallExpression::GenerateBytecode(CodeGenerator & code, const Function & function) const
{
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

void ForExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
{
}


void UnaryExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t') << Token::ToString(type) << '\n';
	out << std::string(depth, '\t') << ">> EXPR:\n";
	expression->Print(out, depth + 1);
}

void UnaryExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
{
	expression->GenerateBytecode(code, function);
	switch (type)
	{
	case Token::Type::NEGATION_OP:
		code.write(OPCODE::NEGATION_OP);
		break;
	case Token::Type::NEGATIVE_OP:
		code.write(OPCODE::NEGATIVE_OP);
		break;
	case Token::Type::POSITIVE_OP:
		code.write(OPCODE::POSITIVE_OP);
		break;
	case Token::Type::NEW:
		code.write(OPCODE::ALLOC_PUSH);
		break;
	default:
		code.write(OPCODE::ERROR_SYMBOL);
		break;
	}
}


void BinaryExpression::Print(std::ostream& out, int depth) const
{
	out << std::string(depth, '\t') << Token::ToString(type) << '\n';
	out << std::string(depth, '\t') << ">> LEFT_EXPR:\n";
	left->Print(out, depth + 1);
	out << std::string(depth, '\t') << ">> RIGHT_EXPR:\n";
	right->Print(out, depth + 1);
}

void BinaryExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
{
	left->GenerateBytecode(code, function);
	right->GenerateBytecode(code, function);
	switch (type)
	{
	case Token::Type::SUM_OP:
		code.write(OPCODE::SUM_OP);
		break;
	case Token::Type::SUB_OP:
		code.write(OPCODE::SUB_OP);
		break;
	case Token::Type::MULT_OP:
		code.write(OPCODE::MULT_OP);
		break;
	case Token::Type::DIV_OP:
		code.write(OPCODE::DIV_OP);
		break;
	case Token::Type::MOD_OP:
		code.write(OPCODE::MOD_OP);
		break;
	case Token::Type::POWER_OP:
		code.write(OPCODE::POWER_OP);
		break;
	case Token::Type::ASSIGN_OP:
		code.write(OPCODE::ASSIGN_OP);
		break;
	case Token::Type::SUM_ASSIGN_OP:
		code.write(OPCODE::SET_ALU_INCR);
		code.write(OPCODE::SUM_OP);
		break;
	case Token::Type::SUB_ASSIGN_OP:
		code.write(OPCODE::SET_ALU_INCR);
		code.write(OPCODE::SUB_OP);
		break;
	case Token::Type::MULT_ASSIGN_OP:
		code.write(OPCODE::SET_ALU_INCR);
		code.write(OPCODE::MULT_OP);
		break;
	case Token::Type::DIV_ASSIGN_OP:
		code.write(OPCODE::SET_ALU_INCR);
		code.write(OPCODE::DIV_OP);
		break;
	case Token::Type::MOD_ASSIGN_OP:
		code.write(OPCODE::SET_ALU_INCR);
		code.write(OPCODE::MOD_OP);
		break;
	default:
		code.write(OPCODE::ERROR_SYMBOL);
		break;
	}

			/*DOT = (BINARY_OPERAND + 1) | 0x00F00000 | ONE_OPCODE_OPERAND,
			LOGIC_EQUALS = (BINARY_OPERAND + 8) | 0x00100000 | ONE_OPCODE_OPERAND,
			LOGIC_NOT_EQUALS = (BINARY_OPERAND + 9) | 0x00100000 | ONE_OPCODE_OPERAND,
			LOGIC_LESS = (BINARY_OPERAND + 10) | 0x00100000 | ONE_OPCODE_OPERAND,
			LOGIC_GREATER = (BINARY_OPERAND + 11) | 0x00100000 | ONE_OPCODE_OPERAND,
			LOGIC_LESS_EQUALS = (BINARY_OPERAND + 12) | 0x00100000 | ONE_OPCODE_OPERAND,
			LOGIC_GREATER_EQUALS = (BINARY_OPERAND + 13) | 0x00100000 | ONE_OPCODE_OPERAND,
			LOGIC_OR = (BINARY_OPERAND + 14) | 0x00200000 | ONE_OPCODE_OPERAND,
			LOGIC_AND = (BINARY_OPERAND + 15) | 0x00300000 | ONE_OPCODE_OPERAND,*/
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

void IndexExpression::GenerateBytecode(CodeGenerator & code, const Function & function) const
{
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

void WhileExpression::GenerateBytecode(CodeGenerator & code, const Function & function) const
{
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

void LambdaExpression::GenerateBytecode(CodeGenerator & code, const Function & function) const
{
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

void ForeachExpression::GenerateBytecode(CodeGenerator & code, const Function & function) const
{
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

void ReturnExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
{
	if (Empty())
	{
		code.write(OPCODE::PUSH_NULL); // if function has no return => return null
	}
	else
	{
		returnValue->GenerateBytecode(code, function);
	}
	code.write(OPCODE::POP_TO_RETURN);
}
