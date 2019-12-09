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

			uint16_t endIf = function.labelInnerId;
			function.labelInnerId++;

			for (size_t i = 0; i < ifStatements.size(); i++, labelId++)
			{
				ifStatements[i]->GenerateBytecode(code, function);
				code.Write(OPCODE::JUMP_IF_FALSE);
				code.Write(labelId);
				GenerateExpressionListBytecode(bodies[i], code, function);
				code.Write(OPCODE::JUMP);
				code.Write(endIf);
				code.Write(OPCODE::SET_LABEL);
				code.Write(labelId);
			}

			if (hasElseBlock())
			{
				GenerateExpressionListBytecode(bodies.back(), code, function);
				code.Write(OPCODE::JUMP);
				code.Write(endIf);
			}
			code.Write(OPCODE::SET_LABEL);
			code.Write(endIf);
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
				code.Write(OPCODE::ALLOC_CONST_VAR);
			}
			else
			{
				code.Write(OPCODE::ALLOC_VAR);
			}
			code.Write(function.GetHash(objectName));
			if (hasAssignment())
			{
				assignment->GenerateBytecode(code, function);
			}
			else
			{
				code.Write(OPCODE::PUSH_NULL);
			}
			code.Write(OPCODE::ASSIGN_OP);
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
				code.Write(OPCODE::PUSH_STRING);
				code.Write(function.GetHash(object.value));
				break;
			case Token::Type::INTEGER_CONSTANT:
				code.Write(OPCODE::PUSH_INTEGER);
				code.Write(function.GetHash(object.value));
				break;
			case Token::Type::FLOAT_CONSTANT:
				code.Write(OPCODE::PUSH_FLOAT);
				code.Write(function.GetHash(object.value));
				break;
			case Token::Type::OBJECT:
				code.Write(OPCODE::PUSH_OBJECT);
				code.Write(function.GetHash(object.value));
				break;
			case Token::Type::TRUE_CONSTANT:
				code.Write(OPCODE::PUSH_TRUE);
				break;
			case Token::Type::FALSE_CONSTANT:
				code.Write(OPCODE::PUSH_FALSE);
				break;
			case Token::Type::THIS:
				code.Write(OPCODE::PUSH_THIS);
				break;
			case Token::Type::NULLPTR:
				code.Write(OPCODE::PUSH_NULL);
				break;
			default:
				code.Write(OPCODE::ERROR_SYMBOL);
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
				code.Write(OPCODE::PUSH_THIS);
			}
			for (const auto& param : parameters)
			{
				param->GenerateBytecode(code, function);
			}
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(functionName));

			code.Write(OPCODE::CALL_FUNCTION);
			code.Write(static_cast<uint8_t>(parameters.size()));
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
				code.Write(OPCODE::POP_STACK_TOP);
			}
			uint16_t labelId = function.labelInnerId;
			function.labelInnerId += 2; // predicate and end-for label

			code.Write(OPCODE::SET_LABEL); // predicate
			code.Write(labelId);

			predicate->GenerateBytecode(code, function);
			code.Write(OPCODE::JUMP_IF_FALSE);
			code.Write<uint16_t>(labelId + 1); // to end-for

			GenerateExpressionListBytecode(body, code, function);
			if (hasIterationStatement())
			{
				iteration->GenerateBytecode(code, function);
				code.Write(OPCODE::POP_STACK_TOP);
			}

			code.Write(OPCODE::JUMP);
			code.Write(labelId); // to predicate
			code.Write(OPCODE::SET_LABEL);
			code.Write<uint16_t>(labelId + 1); // end-for
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
				code.Write(OPCODE::NEGATION_OP);
				break;
			case Token::Type::NEGATIVE_OP:
				code.Write(OPCODE::NEGATIVE_OP);
				break;
			case Token::Type::POSITIVE_OP:
				code.Write(OPCODE::POSITIVE_OP);
				break;
			default:
				code.Write(OPCODE::ERROR_SYMBOL);
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
				code.Write(OPCODE::SUM_OP);
				break;
			case Token::Type::SUB_OP:
				code.Write(OPCODE::SUB_OP);
				break;
			case Token::Type::MULT_OP:
				code.Write(OPCODE::MULT_OP);
				break;
			case Token::Type::DIV_OP:
				code.Write(OPCODE::DIV_OP);
				break;
			case Token::Type::MOD_OP:
				code.Write(OPCODE::MOD_OP);
				break;
			case Token::Type::POWER_OP:
				code.Write(OPCODE::POWER_OP);
				break;
			case Token::Type::ASSIGN_OP:
				code.Write(OPCODE::ASSIGN_OP);
				break;
			case Token::Type::SUM_ASSIGN_OP:
				code.Write(OPCODE::SET_ALU_INCR);
				code.Write(OPCODE::SUM_OP);
				break;
			case Token::Type::SUB_ASSIGN_OP:
				code.Write(OPCODE::SET_ALU_INCR);
				code.Write(OPCODE::SUB_OP);
				break;
			case Token::Type::MULT_ASSIGN_OP:
				code.Write(OPCODE::SET_ALU_INCR);
				code.Write(OPCODE::MULT_OP);
				break;
			case Token::Type::DIV_ASSIGN_OP:
				code.Write(OPCODE::SET_ALU_INCR);
				code.Write(OPCODE::DIV_OP);
				break;
			case Token::Type::MOD_ASSIGN_OP:
				code.Write(OPCODE::SET_ALU_INCR);
				code.Write(OPCODE::MOD_OP);
				break;
			case Token::Type::DOT:
				if (right.get()->type != ExpressionType::CALL &&
					right.get()->type != ExpressionType::INDEX)
				{
					code.Write(OPCODE::GET_MEMBER);
				}
				break;
			case Token::Type::LOGIC_EQUALS:
				code.Write(OPCODE::CMP_EQ);
				break;
			case Token::Type::LOGIC_NOT_EQUALS:
				code.Write(OPCODE::CMP_NEQ);
				break;
			case Token::Type::LOGIC_LESS:
				code.Write(OPCODE::CMP_L);
				break;
			case Token::Type::LOGIC_GREATER:
				code.Write(OPCODE::CMP_G);
				break;
			case Token::Type::LOGIC_LESS_EQUALS:
				code.Write(OPCODE::CMP_LE);
				break;
			case Token::Type::LOGIC_GREATER_EQUALS:
				code.Write(OPCODE::CMP_GE);
				break;
			case Token::Type::LOGIC_OR:
				code.Write(OPCODE::CMP_OR);
				break;
			case Token::Type::LOGIC_AND:
				code.Write(OPCODE::CMP_AND);
				break;
			default:
				code.Write(OPCODE::ERROR_SYMBOL);
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
			code.Write(OPCODE::GET_INDEX);
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
			code.Write(OPCODE::SET_LABEL); // predicate
			code.Write(labelId);

			predicate->GenerateBytecode(code, function);
			code.Write(OPCODE::JUMP_IF_FALSE);
			code.Write<uint16_t>(labelId + 1); // to end-while

			GenerateExpressionListBytecode(body, code, function);

			code.Write(OPCODE::JUMP);
			code.Write(labelId); // to predicate
			code.Write(OPCODE::SET_LABEL);
			code.Write<uint16_t>(labelId + 1); // end-while
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
			code.Write(OPCODE::ALLOC_VAR);
			code.Write(function.GetHash(iterator));
			code.Write(OPCODE::POP_STACK_TOP);

			// var iteratorIndex = container.Begin();
			code.Write(OPCODE::ALLOC_VAR);
			code.Write(function.GetHash(iteratorIndex));

			auto containerDeclare = reinterpret_cast<ObjectDeclareExpression*>(container.get());
			containerDeclare->GenerateBytecode(code, function);

			// call of .Begin(this);
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash("Begin_0"));
			code.Write(OPCODE::CALL_FUNCTION);
			code.Write((uint8_t)0);

			code.Write(OPCODE::ASSIGN_OP); // init iterator with begin of container
			code.Write(OPCODE::POP_STACK_TOP);


			uint16_t labelId = function.labelInnerId;
			function.labelInnerId += 2; // predicate and end-foreach label

			code.Write(OPCODE::SET_LABEL); // predicate
			code.Write(labelId);

			// if(iteratorIndex == container.End()) jump_end;
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(iteratorIndex));
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(containerDeclare->objectName));

			// call of .End(this);
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash("End_0"));
			code.Write(OPCODE::CALL_FUNCTION);
			code.Write((uint8_t)0);

			code.Write(OPCODE::CMP_EQ);
			code.Write(OPCODE::JUMP_IF_TRUE);
			code.Write<uint16_t>(labelId + 1); // to end-foreach

			// iterator = container.GetByIter(iteratorIndex);
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(iterator));
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(containerDeclare->objectName));
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(iteratorIndex));

			// call of .GetByIter(this, iter);
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash("GetByIter_1"));
			code.Write(OPCODE::CALL_FUNCTION);
			code.Write((uint8_t)1);

			code.Write(OPCODE::ASSIGN_OP);
			code.Write(OPCODE::POP_STACK_TOP);

			// generate body inside { }
			GenerateExpressionListBytecode(body, code, function);

			// iteratorIndex = container.Next(iteratorIndex);
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(iteratorIndex));
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(containerDeclare->objectName));
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash(iteratorIndex));

			// call of .Next(this, iter);
			code.Write(OPCODE::PUSH_OBJECT);
			code.Write(function.GetHash("Next_1"));
			code.Write(OPCODE::CALL_FUNCTION);
			code.Write((uint8_t)1);

			code.Write(OPCODE::ASSIGN_OP);
			code.Write(OPCODE::POP_STACK_TOP);


			code.Write(OPCODE::JUMP);
			code.Write(labelId); // to predicate
			code.Write(OPCODE::SET_LABEL);
			code.Write<uint16_t>(labelId + 1); // end-foreach
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
				code.Write(OPCODE::RETURN);
			}
			else
			{
				returnValue->GenerateBytecode(code, function);
				code.Write(OPCODE::POP_TO_RETURN);
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
					code.Write(OPCODE::POP_STACK_TOP); // if result of these expressions is not needed, it should be discarded by stack_pop
					break;
				default:
					break;
				}
			}
		}

		void TryExpression::Print(std::ostream& out, int depth) const
		{
			out << std::string(depth, '\t') << "try\n";
			out << std::string(depth, '\t') << "{\n";
			for(const auto& expr : tryBody)
				expr->Print(out, depth + 1);
			out << std::string(depth, '\t') << "}\n";
			out << std::string(depth, '\t') << "catch\n";
			if (!variable.empty())
				out << std::string(depth, '\t') << '(' << variable << ")\n";
			out << std::string(depth, '\t') << "{\n";
			for (const auto& expr : catchBody)
				expr->Print(out, depth + 1);
			out << std::string(depth, '\t') << "}\n";
		}

		TryExpression::TryExpression()
		{
			BaseExpression::type = ExpressionType::TRY;
		}

		void TryExpression::GenerateBytecode(MSL::compiler::CodeGenerator& code, const MSL::compiler::Function& function) const
		{
			uint16_t labelId = function.labelInnerId;
			function.labelInnerId += 2; // for catch block and end of catch block

			// try { ... }
			code.Write(OPCODE::PUSH_CATCH);
			code.Write(labelId);
			GenerateExpressionListBytecode(tryBody, code, function);
			code.Write(OPCODE::POP_CATCH);
			code.Write(OPCODE::JUMP);
			code.Write<uint16_t>(labelId + 1);
			
			code.Write(OPCODE::SET_LABEL);
			code.Write(labelId);

			if (!variable.empty())
			{
				// [variable] = System.Exception;
				code.Write(OPCODE::ALLOC_CONST_VAR);
				code.Write(function.GetHash(variable));
				code.Write(OPCODE::PUSH_OBJECT);
				code.Write(function.GetHash("System"));
				code.Write(OPCODE::PUSH_OBJECT);
				code.Write(function.GetHash("Exception"));
				code.Write(OPCODE::GET_MEMBER);
				code.Write(OPCODE::PUSH_OBJECT);
				code.Write(function.GetHash("Instance_0"));
				code.Write(OPCODE::CALL_FUNCTION);
				code.Write((uint8_t)0);
				code.Write(OPCODE::ASSIGN_OP);
				code.Write(OPCODE::POP_STACK_TOP);
			}

			GenerateExpressionListBytecode(catchBody, code, function);
			code.Write(OPCODE::SET_LABEL);
			code.Write<uint16_t>(labelId + 1);
		}
	}
}