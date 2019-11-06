#include "expressions.h"

#define TO_BASE(derived_ptr) unique_ptr<BaseExpression>(derived_ptr.release())

namespace MSL
{
	namespace compiler
	{
		using namespace VM;

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

		IfExpression::IfExpression()
		{
			BaseExpression::type = ExpressionType::IF;
		}

		void IfExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			uint16_t labelId = function.labelInnerId;
			function.labelInnerId += (uint16_t)ifStatements.size();

			uint16_t endIf = 0;
			if (hasElseBlock())
			{
				endIf = function.labelInnerId;
				function.labelInnerId++;
			}
			else
			{
				endIf = function.labelInnerId - 1;
			}

			for (size_t i = 0; i < ifStatements.size(); i++, labelId++)
			{
				ifStatements[i]->GenerateBytecode(code, function);
				code.write(OPCODE::JUMP_IF_FALSE);
				code.write(labelId);
				GenerateExpressionListBytecode(bodies[i], code, function);
				code.write(OPCODE::JUMP);
				code.write(endIf);
				code.write(OPCODE::SET_LABEL);
				code.write(labelId);
			}

			if (hasElseBlock())
			{
				GenerateExpressionListBytecode(bodies.back(), code, function);
				code.write(OPCODE::SET_LABEL);
				code.write(endIf);
			}
		}

		ObjectDeclareExpression::ObjectDeclareExpression()
		{
			BaseExpression::type = ExpressionType::DECLARE;
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
			if (isConst)
			{
				code.write(OPCODE::ALLOC_CONST_VAR);
			}
			else
			{
				code.write(OPCODE::ALLOC_VAR);
			}
			code.write(function.GetHash(objectName));
			if (hasAssignment())
			{
				assignment->GenerateBytecode(code, function);
			}
			else
			{
				code.write(OPCODE::PUSH_NULL);
			}
			code.write(OPCODE::ASSIGN_OP);
		}

#undef TO_BASE

		ObjectExpression::ObjectExpression()
		{
			BaseExpression::type = ExpressionType::OBJECT;
		}

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
				code.write(function.GetHash(object.value));
				break;
			case Token::Type::INTEGER_CONSTANT:
				code.write(OPCODE::PUSH_INTEGER);
				code.write(function.GetHash(object.value));
				break;
			case Token::Type::FLOAT_CONSTANT:
				code.write(OPCODE::PUSH_FLOAT);
				code.write(function.GetHash(object.value));
				break;
			case Token::Type::OBJECT:
				code.write(OPCODE::PUSH_OBJECT);
				code.write(function.GetHash(object.value));
				break;
			case Token::Type::TRUE_CONSTANT:
				code.write(OPCODE::PUSH_TRUE);
				break;
			case Token::Type::FALSE_CONSTANT:
				code.write(OPCODE::PUSH_FALSE);
				break;
			case Token::Type::THIS:
				code.write(OPCODE::PUSH_THIS);
				break;
			case Token::Type::NULLPTR:
				code.write(OPCODE::PUSH_NULL);
				break;
			default:
				code.write(OPCODE::ERROR_SYMBOL);
				break;
			}
		}

		void CallExpression::Print(std::ostream& out, int depth) const
		{
			out << std::string(depth, '\t');
			out << ">> PARENT: " << STRBOOL(hasParent) << '\n';
			out << std::string(depth, '\t');
			out << functionName << '\n';
			out << std::string(depth, '\t') << "(\n";
			for (const auto& param : parameters)
			{
				param->Print(out, depth + 1);
			}
			out << std::string(depth, '\t') << ")\n";
		}

		CallExpression::CallExpression()
		{
			BaseExpression::type = ExpressionType::CALL;
		}

		void CallExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			if (!hasParent)
			{
				code.write(OPCODE::PUSH_THIS);
			}
			for (const auto& param : parameters)
			{
				param->GenerateBytecode(code, function);
			}
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(functionName));

			code.write(OPCODE::CALL_FUNCTION);
			code.write(static_cast<uint8_t>(parameters.size()));
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

		bool ForExpression::hasInitStatement() const
		{
			return init != nullptr;
		}

		ForExpression::ForExpression()
		{
			BaseExpression::type = ExpressionType::FOR;
		}

		void ForExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			if (hasInitStatement())
			{
				init->GenerateBytecode(code, function);
				code.write(OPCODE::POP_STACK_TOP);
			}
			uint16_t labelId = function.labelInnerId;
			function.labelInnerId += 2; // predicate and end-for label

			code.write(OPCODE::SET_LABEL); // predicate
			code.write(labelId);

			predicate->GenerateBytecode(code, function);
			code.write(OPCODE::JUMP_IF_FALSE);
			code.write<uint16_t>(labelId + 1); // to end-for

			GenerateExpressionListBytecode(body, code, function);
			if (hasIterationStatement())
			{
				iteration->GenerateBytecode(code, function);
				code.write(OPCODE::POP_STACK_TOP);
			}

			code.write(OPCODE::JUMP);
			code.write(labelId); // to predicate
			code.write(OPCODE::SET_LABEL);
			code.write<uint16_t>(labelId + 1); // end-for
		}


		void UnaryExpression::Print(std::ostream& out, int depth) const
		{
			out << std::string(depth, '\t') << Token::ToString(expressionType) << '\n';
			out << std::string(depth, '\t') << "{\n";
			expression->Print(out, depth + 1);
			out << std::string(depth, '\t') << "}\n";
		}

		UnaryExpression::UnaryExpression()
		{
			BaseExpression::type = ExpressionType::UNARY;
			expressionType = Token::Type::ERROR;
		}

		void UnaryExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			expression->GenerateBytecode(code, function);
			switch (expressionType)
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
			default:
				code.write(OPCODE::ERROR_SYMBOL);
				break;
			}
		}


		void BinaryExpression::Print(std::ostream& out, int depth) const
		{
			out << std::string(depth, '\t') << Token::ToString(expressionType) << '\n';
			out << std::string(depth, '\t') << "{\n";
			left->Print(out, depth + 1);
			out << std::string(depth, '\t') << "}\n";
			out << std::string(depth, '\t') << "{\n";
			right->Print(out, depth + 1);
			out << std::string(depth, '\t') << "}\n";
		}

		BinaryExpression::BinaryExpression()
		{
			BaseExpression::type = ExpressionType::BINARY;
			expressionType = Token::Type::ERROR;
		}

		void BinaryExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			left->GenerateBytecode(code, function);
			right->GenerateBytecode(code, function);
			switch (expressionType)
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
			case Token::Type::DOT:
				if (right.get()->type != ExpressionType::CALL &&
					right.get()->type != ExpressionType::INDEX)
				{
					code.write(OPCODE::GET_MEMBER);
				}
				break;
			case Token::Type::LOGIC_EQUALS:
				code.write(OPCODE::CMP_EQ);
				break;
			case Token::Type::LOGIC_NOT_EQUALS:
				code.write(OPCODE::CMP_NEQ);
				break;
			case Token::Type::LOGIC_LESS:
				code.write(OPCODE::CMP_L);
				break;
			case Token::Type::LOGIC_GREATER:
				code.write(OPCODE::CMP_G);
				break;
			case Token::Type::LOGIC_LESS_EQUALS:
				code.write(OPCODE::CMP_LE);
				break;
			case Token::Type::LOGIC_GREATER_EQUALS:
				code.write(OPCODE::CMP_GE);
				break;
			case Token::Type::LOGIC_OR:
				code.write(OPCODE::CMP_OR);
				break;
			case Token::Type::LOGIC_AND:
				code.write(OPCODE::CMP_AND);
				break;
			default:
				code.write(OPCODE::ERROR_SYMBOL);
				break;
			}
		}

		void IndexExpression::Print(std::ostream& out, int depth) const
		{
			out << std::string(depth, '\t') << "[\n";
			caller->Print(out, depth + 1);
			out << std::string(depth, '\t') << "]\n";
			out << std::string(depth, '\t') << "[\n";
			parameter->Print(out, depth + 1);
			out << std::string(depth, '\t') << "]\n";
		}

		IndexExpression::IndexExpression()
		{
			BaseExpression::type = ExpressionType::INDEX;
		}

		void IndexExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			parameter->GenerateBytecode(code, function);
			caller->GenerateBytecode(code, function);
			code.write(OPCODE::GET_INDEX);
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

		WhileExpression::WhileExpression()
		{
			BaseExpression::type = ExpressionType::WHILE;
		}

		void WhileExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			uint16_t labelId = function.labelInnerId;
			function.labelInnerId += 2; // for init check and end-while jump
			code.write(OPCODE::SET_LABEL); // predicate
			code.write(labelId);

			predicate->GenerateBytecode(code, function);
			code.write(OPCODE::JUMP_IF_FALSE);
			code.write<uint16_t>(labelId + 1); // to end-while

			GenerateExpressionListBytecode(body, code, function);

			code.write(OPCODE::JUMP);
			code.write(labelId); // to predicate
			code.write(OPCODE::SET_LABEL);
			code.write<uint16_t>(labelId + 1); // end-while
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

		LambdaExpression::LambdaExpression()
		{
			BaseExpression::type = ExpressionType::LAMBDA;
		}

		void LambdaExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
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

		ForeachExpression::ForeachExpression()
		{
			BaseExpression::type = ExpressionType::FOREACH;
		}

		void ForeachExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			code.write(OPCODE::ALLOC_VAR);
			code.write(function.GetHash(iterator));
			code.write(OPCODE::POP_STACK_TOP);

			// var iteratorIndex = container.Begin();
			code.write(OPCODE::ALLOC_VAR);
			code.write(function.GetHash(iteratorIndex));

			auto containerDeclare = reinterpret_cast<ObjectDeclareExpression*>(container.get());
			containerDeclare->GenerateBytecode(code, function);

			// call of .Begin(this);
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash("Begin_0"));
			code.write(OPCODE::CALL_FUNCTION);
			code.write((uint8_t)0);

			code.write(OPCODE::ASSIGN_OP); // init iterator with begin of container
			code.write(OPCODE::POP_STACK_TOP);


			uint16_t labelId = function.labelInnerId;
			function.labelInnerId += 2; // predicate and end-foreach label

			code.write(OPCODE::SET_LABEL); // predicate
			code.write(labelId);

			// if(iteratorIndex == container.End()) jump_end;
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(iteratorIndex));
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(containerDeclare->objectName));

			// call of .End(this);
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash("End_0"));
			code.write(OPCODE::CALL_FUNCTION);
			code.write((uint8_t)0);

			code.write(OPCODE::CMP_EQ);
			code.write(OPCODE::JUMP_IF_TRUE);
			code.write<uint16_t>(labelId + 1); // to end-foreach

			// iterator = container.GetByIter(iteratorIndex);
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(iterator));
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(containerDeclare->objectName));
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(iteratorIndex));

			// call of .GetByIter(this, iter);
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash("GetByIter_1"));
			code.write(OPCODE::CALL_FUNCTION);
			code.write((uint8_t)1);

			code.write(OPCODE::ASSIGN_OP);
			code.write(OPCODE::POP_STACK_TOP);

			// generate body inside { }
			GenerateExpressionListBytecode(body, code, function);

			// iteratorIndex = container.Next(iteratorIndex);
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(iteratorIndex));
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(containerDeclare->objectName));
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash(iteratorIndex));

			// call of .Next(this, iter);
			code.write(OPCODE::PUSH_OBJECT);
			code.write(function.GetHash("Next_1"));
			code.write(OPCODE::CALL_FUNCTION);
			code.write((uint8_t)1);

			code.write(OPCODE::ASSIGN_OP);
			code.write(OPCODE::POP_STACK_TOP);


			code.write(OPCODE::JUMP);
			code.write(labelId); // to predicate
			code.write(OPCODE::SET_LABEL);
			code.write<uint16_t>(labelId + 1); // end-foreach
		}

		void ReturnExpression::Print(std::ostream& out, int depth) const
		{
			out << std::string(depth, '\t') << "return\n";
			out << std::string(depth, '\t') << "{\n";
			if (!Empty()) returnValue->Print(out, depth + 1);
			else out << std::string(depth + 1, '\t') << "VOID\n";
			out << std::string(depth, '\t') << "}\n";
		}

		bool ReturnExpression::Empty() const
		{
			return returnValue == nullptr;
		}

		ReturnExpression::ReturnExpression()
		{
			BaseExpression::type = ExpressionType::RETURN;
		}

		void ReturnExpression::GenerateBytecode(CodeGenerator& code, const Function& function) const
		{
			if (Empty())
			{
				code.write(OPCODE::RETURN);
			}
			else
			{
				returnValue->GenerateBytecode(code, function);
				code.write(OPCODE::POP_TO_RETURN);
			}
		}

		void GenerateExpressionListBytecode(const ExpressionList& list, CodeGenerator& code, const Function& function)
		{
			for (const auto& expr : list)
			{
				expr->GenerateBytecode(code, function);
				switch (expr->type)
				{
				case ExpressionType::BINARY:
				case ExpressionType::UNARY:
				case ExpressionType::CALL:
				case ExpressionType::INDEX:
				case ExpressionType::DECLARE:
				case ExpressionType::OBJECT:
					code.write(OPCODE::POP_STACK_TOP); // if result of these expressions is not needed, it should be discarded by stack_pop
					break;
				default:
					break;
				}
			}
		}
	}
}