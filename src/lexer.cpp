#include "lexer.h"
#include <sstream>
using namespace MSL::utils;

struct yy_buffer_state
{
    FILE* yy_input_file;
    char* yy_ch_buf;
    char* yy_buf_pos;
    int yy_buf_size;
    int yy_n_chars;
    int yy_is_our_buffer;
    int yy_is_interactive;
    int yy_at_bol;
    int yy_bs_lineno;
    int yy_bs_column;
    int yy_fill_buffer;
    int yy_buffer_status;
};
typedef struct yy_buffer_state* YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_bytes(const char* buffer, int size);
MSL::compiler::Token yy_lex();
void yy_delete_buffer(YY_BUFFER_STATE);
MSL::compiler::Token::Type yy_get_last();
void yy_set_last(MSL::compiler::Token::Type type);

namespace MSL
{
	namespace compiler
	{
		Lexer::Lexer(std::string stream)
			: iteratorPos(0), lineCount(0), EOFtoken(Token::Type::ERROR, "EOF")
		{
            stream += "__EOF__"; // yy_lex will return Token::Type::ENDOFFILE when meet __EOF__
            auto buffer = yy_scan_bytes(stream.c_str(), stream.size());
            Token token = yy_lex();
            while (token.type != Token::Type::ENDOFFILE)
            {
                yy_set_last(token.type);
                tokens.push_back(std::move(token));
                token = yy_lex();
            }
            yy_delete_buffer(buffer);
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
			while (!tokens.empty() && tokens[iteratorPos].type == Token::ENDLINE)
			{
				iteratorPos++;
				lineCount++;
			}
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

		std::string Lexer::PrintFile()
		{
			std::stringstream out;
			int pos = this->iteratorPos;
			ToBegin();
			while (!End())
			{
				out << beautify(ToNextLine()) << '\n';
				Next();
			}
			iteratorPos = pos;
			return out.str();
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
					if (reversedMap.find(stringConstant) == reversedMap.end())
					{
						Peek().type = Token::Type::ERROR;
						Peek().value = "<error> closing `\"` not found";
						break;
					}
					else
						Peek().value = reversedMap.find(stringConstant)->second;
				}
				Next();
			}
			ToBegin();
		}
	}
}