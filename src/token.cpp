#include "Token.h"

using namespace MSL::utils;

namespace MSL
{
	namespace compiler
	{
		Token::Token(Token::Type type, const std::string& value) : type(type), value(value) { }

        Token::Token(Token::Type type) : type(type) { }

		std::string Token::ToString() const
		{
			std::string out = "[";
			out += ToString(type);
			out += ' ' + value + ']';
			return out;
		}

		std::string Token::ToString(Type type)
		{
			switch (type)
			{
			case MSL::compiler::Token::ENDLINE:
				return STRING(Token::ENDLINE);
				break;
			case MSL::compiler::Token::APOS:
				return STRING(Token::APOS);
				break;
			case MSL::compiler::Token::COMMA:
				return STRING(Token::COMMA);
				break;
			case MSL::compiler::Token::SEMICOLON:
				return STRING(Token::SEMICOLON);
				break;
			case MSL::compiler::Token::FOR:
				return STRING(Token::FOR);
				break;
			case MSL::compiler::Token::IF:
				return STRING(Token::IF);
				break;
			case MSL::compiler::Token::ELSE:
				return STRING(Token::ELSE);
				break;
			case MSL::compiler::Token::ELIF:
				return STRING(Token::ELIF);
				break;
			case MSL::compiler::Token::WHILE:
				return STRING(Token::WHILE);
				break;
			case MSL::compiler::Token::VARIABLE:
				return STRING(Token::VARIABLE);
				break;
			case MSL::compiler::Token::FUNCTION:
				return STRING(Token::FUNCTION);
				break;
			case MSL::compiler::Token::NAMESPACE:
				return STRING(Token::NAMESPACE);
				break;
			case MSL::compiler::Token::LAMBDA:
				return STRING(Token::LAMBDA);
				break;
			case MSL::compiler::Token::FOREACH:
				return STRING(Token::FOREACH);
				break;
			case MSL::compiler::Token::IN:
				return STRING(Token::IN);
				break;
			case MSL::compiler::Token::USING:
				return STRING(Token::USING);
				break;
			case MSL::compiler::Token::TRY:
				return STRING(Token::TRY);
				break;
			case MSL::compiler::Token::CATCH:
				return STRING(Token::CATCH);
				break;
			case MSL::compiler::Token::OBJECT:
				return STRING(Token::OBJECT);
				break;
			case MSL::compiler::Token::THIS:
				return STRING(Token::THIS);
				break;
			case MSL::compiler::Token::INTEGER_CONSTANT:
				return STRING(Token::INTEGER_CONSTANT);
				break;
			case MSL::compiler::Token::FLOAT_CONSTANT:
				return STRING(Token::FLOAT_CONSTANT);
				break;
			case MSL::compiler::Token::STRING_CONSTANT:
				return STRING(Token::STRING_CONSTANT);
				break;
			case MSL::compiler::Token::TRUE_CONSTANT:
				return STRING(Token::TRUE_CONSTANT);
				break;
			case MSL::compiler::Token::FALSE_CONSTANT:
				return STRING(Token::FALSE_CONSTANT);
				break;
			case MSL::compiler::Token::NULLPTR:
				return STRING(Token::NULLPTR);
				break;
			case MSL::compiler::Token::ROUND_BRACKET_O:
				return STRING(Token::ROUND_BRACKET_O);
				break;
			case MSL::compiler::Token::ROUND_BRACKET_C:
				return STRING(Token::ROUND_BRACKET_C);
				break;
			case MSL::compiler::Token::SQUARE_BRACKET_O:
				return STRING(Token::SQUARE_BRACKET_O);
				break;
			case MSL::compiler::Token::SQUARE_BRACKET_C:
				return STRING(Token::SQUARE_BRACKET_C);
				break;
			case MSL::compiler::Token::BRACE_BRACKET_O:
				return STRING(Token::BRACE_BRACKET_O);
				break;
			case MSL::compiler::Token::BRACE_BRACKET_C:
				return STRING(Token::BRACE_BRACKET_C);
				break;
			case MSL::compiler::Token::NEGATION_OP:
				return STRING(Token::NEGATION_OP);
				break;
			case MSL::compiler::Token::NEGATIVE_OP:
				return STRING(Token::NEGATIVE_OP);
				break;
			case MSL::compiler::Token::POSITIVE_OP:
				return STRING(Token::POSITIVE_OP);
				break;
			case MSL::compiler::Token::RETURN:
				return STRING(Token::RETURN);
				break;
			case MSL::compiler::Token::DOT:
				return STRING(Token::DOT);
				break;
			case MSL::compiler::Token::ASSIGN_OP:
				return STRING(Token::ASSIGN_OP);
				break;
			case MSL::compiler::Token::SUM_ASSIGN_OP:
				return STRING(Token::SUM_ASSIGN_OP);
				break;
			case MSL::compiler::Token::SUB_ASSIGN_OP:
				return STRING(Token::SUB_ASSIGN_OP);
				break;
			case MSL::compiler::Token::MULT_ASSIGN_OP:
				return STRING(Token::MULT_ASSIGN_OP);
				break;
			case MSL::compiler::Token::DIV_ASSIGN_OP:
				return STRING(Token::DIV_ASSIGN_OP);
				break;
			case MSL::compiler::Token::MOD_ASSIGN_OP:
				return STRING(Token::MOD_ASSIGN_OP);
				break;
			case MSL::compiler::Token::LOGIC_EQUALS:
				return STRING(Token::LOGIC_EQUALS);
				break;
			case MSL::compiler::Token::LOGIC_NOT_EQUALS:
				return STRING(Token::LOGIC_NOT_EQUALS);
				break;
			case MSL::compiler::Token::LOGIC_LESS:
				return STRING(Token::LOGIC_LESS);
				break;
			case MSL::compiler::Token::LOGIC_GREATER:
				return STRING(Token::LOGIC_GREATER);
				break;
			case MSL::compiler::Token::LOGIC_LESS_EQUALS:
				return STRING(Token::LOGIC_LESS_EQUALS);
				break;
			case MSL::compiler::Token::LOGIC_GREATER_EQUALS:
				return STRING(Token::LOGIC_GREATER_EQUALS);
				break;
			case MSL::compiler::Token::LOGIC_OR:
				return STRING(Token::LOGIC_OR);
				break;
			case MSL::compiler::Token::LOGIC_AND:
				return STRING(Token::LOGIC_AND);
				break;
			case MSL::compiler::Token::SUM_OP:
				return STRING(Token::SUM_OP);
				break;
			case MSL::compiler::Token::SUB_OP:
				return STRING(Token::SUB_OP);
				break;
			case MSL::compiler::Token::MULT_OP:
				return STRING(Token::MULT_OP);
				break;
			case MSL::compiler::Token::DIV_OP:
				return STRING(Token::DIV_OP);
				break;
			case MSL::compiler::Token::MOD_OP:
				return STRING(Token::MOD_OP);
				break;
			case MSL::compiler::Token::POWER_OP:
				return STRING(Token::POWER_OP);
				break;
			case MSL::compiler::Token::CONST:
				return STRING(Token::CONST);
				break;
			case MSL::compiler::Token::PUBLIC:
				return STRING(Token::PUBLIC);
				break;
			case MSL::compiler::Token::PRIVATE:
				return STRING(Token::PRIVATE);
				break;
			case MSL::compiler::Token::INTERNAL:
				return STRING(Token::INTERNAL);
				break;
			case MSL::compiler::Token::ABSTRACT:
				return STRING(Token::ABSTRACT);
				break;
			case MSL::compiler::Token::STATIC:
				return STRING(Token::STATIC);
				break;
			case MSL::compiler::Token::CLASS:
				return STRING(Token::CLASS);
				break;
			default:
				return STRING(Token::ERROR);
			}
		}

		bool find(const std::vector<Token::Type>& vector, Token::Type type)
		{
			return std::find(vector.begin(), vector.end(), type) != vector.end();
		}
	}
}