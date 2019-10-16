#pragma once

#include "stringExtensions.h"

namespace MSL
{
	namespace compiler
	{
		/*
		Token is a pair of type and value, which is generated by lexer from source code of MSL program
		is used by parser and code generator to generate bytecode
		*/
		struct Token
		{
			// uint32:				 0000 0000 0000 0000 0000 0000 0000 0000 -> 0x00000000
			// operand priority:	 0000 000X XXXX 0000 0000 0000 0000 0000 -> 0x0XX00000
			// bracket				 0000 0100 0000 0000 0000 0000 0000 0000 -> 0x04000000
			// value type:			 0000 1000 0000 0000 0000 0000 0000 0000 -> 0x08000000
			// unit declaration:	 0001 0000 0000 0000 0000 0000 0000 0000 -> 0x10000000
			// modifier:			 0010 0000 0000 0000 0000 0000 0000 0000 -> 0x20000000
			// unary operand:		 0100 0000 0000 0000 0000 0000 0000 0000 -> 0x40000000
			// binary operand:		 1000 0000 0000 0000 0000 0000 0000 0000 -> 0x80000000

			/*
			enums of all token types and masks to simplify their parsing
			*/

			enum Type : uint32_t
			{
				BRACKET = 0x04000000,
				VALUE_TYPE = 0x08000000,
				UNIT_DECLARATION = 0x10000000,
				MODIFIER = 0x20000000,
				UNARY_OPERAND = 0x40000000,
				BINARY_OPERAND = 0x80000000,
				PRIORITY = 0x01F00000,
				LOWEST_PRIORITY = 0x00000000,
				HIGHEST_PRIORITY = 0x01F00000,

				ERROR = 0x00000000,
				ENDLINE,
				APOS,
				COMMA,
				SEMICOLON,
				FOR,
				IF,
				ELSE,
				ELIF,
				WHILE,
				VARIABLE,
				FUNCTION,
				NAMESPACE,
				LAMBDA,
				FOREACH,
				IN,
				USING,

				OBJECT = VALUE_TYPE + 1,
				THIS,
				INTEGER_CONSTANT,
				FLOAT_CONSTANT,
				STRING_CONSTANT,
				TRUE_CONSTANT,
				FALSE_CONSTANT,
				NULLPTR,

				ROUND_BRACKET_O = BRACKET + 1,
				ROUND_BRACKET_C,
				SQUARE_BRACKET_O,
				SQUARE_BRACKET_C,
				BRACE_BRACKET_O,
				BRACE_BRACKET_C,

				NEGATION_OP = (UNARY_OPERAND + 1) | 0x00E00000,
				NEGATIVE_OP = (UNARY_OPERAND + 2) | 0x00E00000,
				POSITIVE_OP = (UNARY_OPERAND + 3) | 0x00E00000,
				RETURN =      (UNARY_OPERAND + 4) | 0x00000000,

				DOT =                  (BINARY_OPERAND + 1)  | 0x00F00000,
				ASSIGN_OP =            (BINARY_OPERAND + 2)  | 0x00100000,
				SUM_ASSIGN_OP =        (BINARY_OPERAND + 3)  | 0x00100000,
				SUB_ASSIGN_OP =        (BINARY_OPERAND + 4)  | 0x00100000,
				MULT_ASSIGN_OP =       (BINARY_OPERAND + 5)  | 0x00100000,
				DIV_ASSIGN_OP =        (BINARY_OPERAND + 6)  | 0x00100000,
				MOD_ASSIGN_OP =        (BINARY_OPERAND + 7)  | 0x00100000,
				LOGIC_EQUALS =         (BINARY_OPERAND + 8)  | 0x00200000,
				LOGIC_NOT_EQUALS =     (BINARY_OPERAND + 9)  | 0x00200000,
				LOGIC_LESS =           (BINARY_OPERAND + 10) | 0x00200000,
				LOGIC_GREATER =        (BINARY_OPERAND + 11) | 0x00200000,
				LOGIC_LESS_EQUALS =    (BINARY_OPERAND + 12) | 0x00200000,
				LOGIC_GREATER_EQUALS = (BINARY_OPERAND + 13) | 0x00200000,
				LOGIC_OR =             (BINARY_OPERAND + 14) | 0x00300000,
				LOGIC_AND =            (BINARY_OPERAND + 15) | 0x00400000,
				SUM_OP =               (BINARY_OPERAND + 16) | 0x00500000,
				SUB_OP =               (BINARY_OPERAND + 17) | 0x00500000,
				MULT_OP =              (BINARY_OPERAND + 18) | 0x00600000,
				DIV_OP =               (BINARY_OPERAND + 19) | 0x00600000,
				MOD_OP =               (BINARY_OPERAND + 20) | 0x00600000,
				POWER_OP =             (BINARY_OPERAND + 21) | 0x00700000,

				CONST = MODIFIER + 1,
				PUBLIC,
				PRIVATE,
				INTERNAL,
				ABSTRACT,
				STATIC,

				CLASS = UNIT_DECLARATION + 1,
				INTERFACE,
			};
			/*
			type of token as 32-bit integer
			*/
			Type type;
			/*
			underlying value of token (object name / constant)
			*/
			std::string value;

			/*
			creates token using its type and string value
			*/
			Token(Type type, const std::string& value);

			/*
			returns type of token or ERROR if no type matched
			*/
			static Type GetType(const std::string& value);
			/*
			returns string representation of token type in format "TYPE::TOKEN_TYPE"
			*/
			static std::string ToString(Type type);

			/*
			returns string representation of token (its type and value as pair)
			*/
			std::string ToString() const;
		};

		/*
		performs linear search in array of tokens for a specific one. Return true if match was found, false either
		*/
		bool find(const std::vector<Token::Type>& vector, Token::Type type);
	}
}