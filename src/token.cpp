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
                return "\n";
				break;
			case MSL::compiler::Token::APOS:
                return "'";
				break;
			case MSL::compiler::Token::COMMA:
                return ",";
				break;
			case MSL::compiler::Token::SEMICOLON:
                return ";";
				break;
			case MSL::compiler::Token::FOR:
                return "for";
				break;
			case MSL::compiler::Token::IF:
                return "if";
				break;
			case MSL::compiler::Token::ELSE:
                return "else";
				break;
			case MSL::compiler::Token::ELIF:
                return "elif";
				break;
			case MSL::compiler::Token::WHILE:
                return "while";
				break;
			case MSL::compiler::Token::VARIABLE:
                return "var";
				break;
			case MSL::compiler::Token::FUNCTION:
                return "function";
				break;
			case MSL::compiler::Token::NAMESPACE:
                return "namespace";
				break;
			case MSL::compiler::Token::LAMBDA:
                return "lambda";
				break;
			case MSL::compiler::Token::FOREACH:
                return "foreach";
				break;
			case MSL::compiler::Token::IN:
                return "in";
				break;
			case MSL::compiler::Token::USING:
                return "using";
				break;
			case MSL::compiler::Token::TRY:
                return "try";
				break;
			case MSL::compiler::Token::CATCH:
                return "catch";
				break;
			case MSL::compiler::Token::OBJECT:
                return "[object]";
				break;
			case MSL::compiler::Token::THIS:
                return "this";
				break;
			case MSL::compiler::Token::INTEGER_CONSTANT:
                return "[integer]";
				break;
			case MSL::compiler::Token::FLOAT_CONSTANT:
                return "[float]";
				break;
			case MSL::compiler::Token::STRING_CONSTANT:
                return "[string]";
				break;
			case MSL::compiler::Token::TRUE_CONSTANT:
                return "true";
				break;
			case MSL::compiler::Token::FALSE_CONSTANT:
                return "false";
                break;
			case MSL::compiler::Token::NULLPTR:
                return "null";
				break;
			case MSL::compiler::Token::ROUND_BRACKET_O:
                return "(";
				break;
			case MSL::compiler::Token::ROUND_BRACKET_C:
                return ")";
				break;
			case MSL::compiler::Token::SQUARE_BRACKET_O:
                return "[";
				break;
			case MSL::compiler::Token::SQUARE_BRACKET_C:
                return "]";
				break;
			case MSL::compiler::Token::BRACE_BRACKET_O:
                return "{";
				break;
			case MSL::compiler::Token::BRACE_BRACKET_C:
                return "}";
				break;
			case MSL::compiler::Token::NEGATION_OP:
                return "!";
				break;
			case MSL::compiler::Token::NEGATIVE_OP:
                return "-";
				break;
			case MSL::compiler::Token::POSITIVE_OP:
                return "+";
				break;
			case MSL::compiler::Token::RETURN:
                return "return";
				break;
			case MSL::compiler::Token::DOT:
                return ".";
				break;
			case MSL::compiler::Token::ASSIGN_OP:
                return "=";
				break;
			case MSL::compiler::Token::SUM_ASSIGN_OP:
                return "+=";
				break;
			case MSL::compiler::Token::SUB_ASSIGN_OP:
                return "-=";
				break;
			case MSL::compiler::Token::MULT_ASSIGN_OP:
                return "*=";
				break;
			case MSL::compiler::Token::DIV_ASSIGN_OP:
                return "/=";
				break;
			case MSL::compiler::Token::MOD_ASSIGN_OP:
                return "%=";
                break;
            case MSL::compiler::Token::POWER_ASSIGN_OP:
                return "**=";
				break;
			case MSL::compiler::Token::LOGIC_EQUALS:
                return "==";
				break;
			case MSL::compiler::Token::LOGIC_NOT_EQUALS:
                return "!=";
				break;
			case MSL::compiler::Token::LOGIC_LESS:
                return "<";
				break;
			case MSL::compiler::Token::LOGIC_GREATER:
                return ">";
				break;
			case MSL::compiler::Token::LOGIC_LESS_EQUALS:
                return "<=";
				break;
			case MSL::compiler::Token::LOGIC_GREATER_EQUALS:
                return ">=";
				break;
			case MSL::compiler::Token::LOGIC_OR:
                return "||";
				break;
			case MSL::compiler::Token::LOGIC_AND:
                return "&&";
				break;
			case MSL::compiler::Token::SUM_OP:
                return "+";
				break;
			case MSL::compiler::Token::SUB_OP:
                return "-";
				break;
			case MSL::compiler::Token::MULT_OP:
                return "*";
				break;
			case MSL::compiler::Token::DIV_OP:
                return "/";
				break;
			case MSL::compiler::Token::MOD_OP:
                return "%";
				break;
			case MSL::compiler::Token::POWER_OP:
                return "**";
				break;
			case MSL::compiler::Token::CONST:
                return "const";
				break;
			case MSL::compiler::Token::PUBLIC:
                return "public";
				break;
			case MSL::compiler::Token::PRIVATE:
                return "private";
				break;
			case MSL::compiler::Token::STATIC:
                return "static";
				break;
			case MSL::compiler::Token::CLASS:
                return "class";
				break;
			default:
                return "[error]";
			}
		}

		bool find(const std::vector<Token::Type>& vector, Token::Type type)
		{
			return std::find(vector.begin(), vector.end(), type) != vector.end();
		}
	}
}