#include "Token.h"

Token::Token(Token::Type type, const std::string& value) : type(type), value(value) { }

Token::Type Token::GetType(const std::string& value)
{	
	if (value == "\n") return Type::ENDLINE;
	if (value == ".") return Type::DOT;
	if (value == "'") return Type::APOS;
	if (value == ",") return Type::COMMA;
	if (value == ";") return Type::SEMICOLON;
	if (value == "(") return Type::ROUND_BRACKET_O;
	if (value == ")") return Type::ROUND_BRACKET_C;
	if (value == "[") return Type::SQUARE_BRACKET_O;
	if (value == "]") return Type::SQUARE_BRACKET_C;
	if (value == "{") return Type::BRACE_BRACKET_O;
	if (value == "}") return Type::BRACE_BRACKET_C;
	if (value == "=") return Type::ASSIGN_OP;
	if (value == "!") return Type::NEGATION_OP;
	if (value == "+") return Type::SUM_OP;
	if (value == "-") return Type::SUB_OP;
	if (value == "*") return Type::MULT_OP;
	if (value == "/") return Type::DIV_OP;
	if (value == "%") return Type::MOD_OP;
	if (value == "&&") return Type::LOGIC_AND;
	if (value == "||") return Type::LOGIC_OR;
	if (value == "==") return Type::LOGIC_EQUALS;
	if (value == "!=") return Type::LOGIC_NOT_EQUALS;
	if (value == "<") return Type::LOGIC_LESS;
	if (value == ">") return Type::LOGIC_GREATER;
	if (value == "<=") return Type::LOGIC_LESS_EQUALS;
	if (value == ">=") return Type::LOGIC_GREATER_EQUALS;
	if (value == "**") return Type::POWER_OP;
	if (value == "+=") return Type::SUM_ASSIGN_OP;
	if (value == "-=") return Type::SUB_ASSIGN_OP;
	if (value == "*=") return Type::MULT_ASSIGN_OP;
	if (value == "/=") return Type::DIV_ASSIGN_OP;
	if (value == "%=") return Type::MOD_ASSIGN_OP;
	if (value == "class") return Type::CLASS;
	if (value == "for") return Type::FOR;
	if (value == "if") return Type::IF;
	if (value == "else") return Type::ELSE;
	if (value == "elif") return Type::ELIF;
	if (value == "while") return Type::WHILE;
	if (value == "static") return Type::STATIC;
	if (value == "var") return Type::VARIABLE;
	if (value == "function") return Type::FUNCTION;
	if (value == "const") return Type::CONST;
	if (value == "public") return Type::PUBLIC;
	if (value == "private") return Type::PRIVATE;
	if (value == "internal") return Type::INTERNAL;
	if (value == "abstract") return Type::ABSTRACT;
	if (value == "namespace") return Type::NAMESPACE;
	if (value == "interface") return Type::INTERFACE;
	if (value == "return") return Type::RETURN;
	if (value == "new") return Type::NEW;
	if (value == "lambda") return Type::LAMBDA;
	if (value == "this") return Type::THIS;
	if (value == "in") return Type::IN;
	if (value == "foreach") return Type::FOREACH;
	if (value == "true") return Type::TRUE_CONSTANT;
	if (value == "false") return Type::FALSE_CONSTANT;
	if (value == "null") return Type::NULLPTR;
	if (isInteger(value)) return Type::INTEGER_CONSTANT;
	if (isFloat(value)) return Type::FLOAT_CONSTANT;
	if (isString(value)) return Type::STRING_CONSTANT;
	if (isObject(value)) return Type::OBJECT;
	return Type::ERROR;
}

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
	case Token::Type::ENDLINE:
		return STRING(Token::Type::ENDLINE);
	case Token::Type::APOS:
		return STRING(Token::Type::APOS);
	case Token::Type::COMMA:
		return STRING(Token::Type::COMMA);
	case Token::Type::SEMICOLON:
		return STRING(Token::Type::SEMICOLON);
	case Token::Type::FOR:
		return STRING(Token::Type::FOR);
	case Token::Type::IF:
		return STRING(Token::Type::IF);
	case Token::Type::ELSE:
		return STRING(Token::Type::ELSE);
	case Token::Type::ELIF:
		return STRING(Token::Type::ELIF);
	case Token::Type::WHILE:
		return STRING(Token::Type::WHILE);
	case Token::Type::VARIABLE:
		return STRING(Token::Type::VARIABLE);
	case Token::Type::FUNCTION:
		return STRING(Token::Type::FUNCTION);
	case Token::Type::NAMESPACE:
		return STRING(Token::Type::NAMESPACE);
	case Token::Type::LAMBDA:
		return STRING(Token::Type::LAMBDA);
	case Token::Type::THIS:
		return STRING(Token::Type::THIS);
	case Token::Type::IN:
		return STRING(Token::Type::IN);
	case Token::Type::FOREACH:
		return STRING(Token::Type::FOREACH);
	case Token::Type::OBJECT:
		return STRING(Token::Type::OBJECT);
	case Token::Type::TRUE_CONSTANT:
		return STRING(Token::Type::TRUE_CONSTANT);
	case Token::Type::FALSE_CONSTANT:
		return STRING(Token::Type::FALSE_CONSTANT);
	case Token::Type::NULLPTR:
		return STRING(Token::Type::NULLPTR);
	case Token::Type::INTEGER_CONSTANT:
		return STRING(Token::Type::INTEGER_CONSTANT);
	case Token::Type::FLOAT_CONSTANT:
		return STRING(Token::Type::FLOAT_CONSTANT);
	case Token::Type::STRING_CONSTANT:
		return STRING(Token::Type::STRING_CONSTANT);
	case Token::Type::ROUND_BRACKET_O:
		return STRING(Token::Type::ROUND_BRACKET_O);
	case Token::Type::ROUND_BRACKET_C:
		return STRING(Token::Type::ROUND_BRACKET_C);
	case Token::Type::SQUARE_BRACKET_O:
		return STRING(Token::Type::SQUARE_BRACKET_O);
	case Token::Type::SQUARE_BRACKET_C:
		return STRING(Token::Type::SQUARE_BRACKET_C);
	case Token::Type::BRACE_BRACKET_O:
		return STRING(Token::Type::BRACE_BRACKET_O);
	case Token::Type::BRACE_BRACKET_C:
		return STRING(Token::Type::BRACE_BRACKET_C);
	case Token::Type::NEGATION_OP:
		return STRING(Token::Type::NEGATION_OP);
	case Token::Type::NEGATIVE_OP:
		return STRING(Token::Type::NEGATIVE_OP);
	case Token::Type::POSITIVE_OP:
		return STRING(Token::Type::POSITIVE_OP);
	case Token::Type::NEW:
		return STRING(Token::Type::NEW);
	case Token::Type::RETURN:
		return STRING(Token::Type::RETURN);
	case Token::Type::DOT:
		return STRING(Token::Type::DOT);
	case Token::Type::ASSIGN_OP:
		return STRING(Token::Type::ASSIGN_OP);
	case Token::Type::SUM_ASSIGN_OP:
		return STRING(Token::Type::SUM_ASSIGN_OP);
	case Token::Type::SUB_ASSIGN_OP:
		return STRING(Token::Type::SUB_ASSIGN_OP);
	case Token::Type::MULT_ASSIGN_OP:
		return STRING(Token::Type::MULT_ASSIGN_OP);
	case Token::Type::DIV_ASSIGN_OP:
		return STRING(Token::Type::DIV_ASSIGN_OP);
	case Token::Type::MOD_ASSIGN_OP:
		return STRING(Token::Type::MOD_ASSIGN_OP);
	case Token::Type::LOGIC_EQUALS:
		return STRING(Token::Type::LOGIC_EQUALS);
	case Token::Type::LOGIC_NOT_EQUALS:
		return STRING(Token::Type::LOGIC_NOT_EQUALS);
	case Token::Type::LOGIC_LESS:
		return STRING(Token::Type::LOGIC_LESS);
	case Token::Type::LOGIC_GREATER:
		return STRING(Token::Type::LOGIC_GREATER);
	case Token::Type::LOGIC_LESS_EQUALS:
		return STRING(Token::Type::LOGIC_LESS_EQUALS);
	case Token::Type::LOGIC_GREATER_EQUALS:
		return STRING(Token::Type::LOGIC_GREATER_EQUALS);
	case Token::Type::LOGIC_OR:
		return STRING(Token::Type::LOGIC_OR);
	case Token::Type::LOGIC_AND:
		return STRING(Token::Type::LOGIC_AND);
	case Token::Type::SUM_OP:
		return STRING(Token::Type::SUM_OP);
	case Token::Type::SUB_OP:
		return STRING(Token::Type::SUB_OP);
	case Token::Type::MULT_OP:
		return STRING(Token::Type::MULT_OP);
	case Token::Type::DIV_OP:
		return STRING(Token::Type::DIV_OP);
	case Token::Type::MOD_OP:
		return STRING(Token::Type::MOD_OP);
	case Token::Type::POWER_OP:
		return STRING(Token::Type::POWER_OP);
	case Token::Type::CONST:
		return STRING(Token::Type::CONST);
	case Token::Type::PUBLIC:
		return STRING(Token::Type::PUBLIC);
	case Token::Type::PRIVATE:
		return STRING(Token::Type::PRIVATE);
	case Token::Type::INTERNAL:
		return STRING(Token::Type::INTERNAL);
	case Token::Type::ABSTRACT:
		return STRING(Token::Type::ABSTRACT);
	case Token::Type::STATIC:
		return STRING(Token::Type::STATIC);
	case Token::Type::CLASS:
		return STRING(Token::Type::CLASS);
	case Token::Type::INTERFACE:
		return STRING(Token::Type::INTERFACE);
	default:
		return STRING(Token::Type::ERROR);
	}
}

bool find(const std::vector<Token::Type>& vector, Token::Type type)
{
	return std::find(vector.begin(), vector.end(), type) != vector.end();
}
