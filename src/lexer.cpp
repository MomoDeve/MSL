#include "lexer.h"

namespace MSL
{
	namespace compiler
	{
		Lexer::Lexer(const std::string& stream)
			: stream(stream), iteratorPos(0), lineCount(0), EOFtoken(Token::Type::ERROR, "EOF")
		{
			for (int i = 0; i < (int)stream.size(); i++)
			{
				std::string str = readIf(stream, i, [](char c) { return (c != TOKEN_SEPARATOR); });
				i++;
				tokens.emplace_back(Token::GetType(str), str);
			}

			while (!End())
			{
				Token& current = Peek();
				if (current.type == Token::Type::SUB_OP || current.type == Token::Type::SUM_OP)
				{
					Prev();
					Token& prev = Peek();
					Next();
					if (prev.type == Token::Type::ROUND_BRACKET_O || prev.type == Token::Type::SQUARE_BRACKET_O ||
						prev.type == Token::Type::BRACE_BRACKET_O || prev.type == EOFtoken.type ||
						(prev.type & Token::Type::BINARY_OPERAND) || (prev.type & Token::Type::UNARY_OPERAND) ||
						prev.type == Token::Type::SEMICOLON)
					{
						if (current.type == Token::Type::SUB_OP) current.type = Token::Type::NEGATIVE_OP;
						else current.type = Token::Type::POSITIVE_OP;
					}
				}
				Next();
			}
			ToBegin();
		}

		Token& Lexer::Peek()
		{
			if (Begin() || End()) return EOFtoken;
			return tokens[iteratorPos];
		}

		void Lexer::Next()
		{
			lineCount--;
			do
			{
				iteratorPos++;
				lineCount++;
			} while (Peek().type == Token::Type::ENDLINE);
		}

		void Lexer::Prev()
		{
			lineCount++;
			do
			{
				iteratorPos--;
				lineCount--;
			} while (Peek().type == Token::Type::ENDLINE);
		}

		void Lexer::ToBegin()
		{
			iteratorPos = 0;
			lineCount = 0;
		}

		bool Lexer::End() const
		{
			return iteratorPos >= (int)tokens.size();
		}

		bool Lexer::Begin() const
		{
			return iteratorPos < 0;
		}

		int Lexer::GetLineCount() const
		{
			return lineCount;
		}

		int Lexer::GetIteratorPos() const
		{
			return iteratorPos;
		}

		std::string Lexer::ToNextLine()
		{
			iteratorPos = iteratorPos < (int)tokens.size() ? iteratorPos : (int)tokens.size() - 1;
			std::string line;
			while (!End() && tokens[iteratorPos].type != Token::Type::ENDLINE)
			{
				line += tokens[iteratorPos].value + TOKEN_SEPARATOR;
				iteratorPos++;
			}
			Prev();
			return line;
		}

		std::string Lexer::ToPrevLine()
		{
			iteratorPos = iteratorPos < (int)tokens.size() ? iteratorPos : (int)tokens.size() - 1;
			std::string line;
			while (!Begin() && tokens[iteratorPos].type != Token::Type::ENDLINE)
			{
				line = tokens[iteratorPos].value + TOKEN_SEPARATOR + line;
				iteratorPos--;
			}
			Next();
			return line;
		}

		void Lexer::SetIteratorPos(int pos)
		{
			iteratorPos = pos;
		}

		void Lexer::ReplaceStrings(const std::unordered_map<std::string, std::string>& stringMap)
		{
			std::unordered_map<std::string, std::string> reversedMap;
			reversedMap.reserve(stringMap.size());
			for (const auto& pair : stringMap)
			{
				reversedMap.insert({ pair.second, pair.first });
			}
			while (!End())
			{
				if (Peek().type == Token::Type::STRING_CONSTANT)
				{
					std::string stringConstant = Peek().value;
					Peek().value = reversedMap.find(stringConstant)->second;
				}
				Next();
			}
			ToBegin();
		}
	}
}