#pragma once

#include <istream>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>

#include "stringExtensions.h"
#include "token.h"

namespace MSL
{
	namespace compiler
	{
		/*
		Lexer object for analysing code and spliting it into tokens
		accepts edited string with divided tokens, without strings and comments
		*/
		class Lexer
		{
			/*
			array of tokens which can be iterated using Next(), Peek() and Prev()
			*/
			std::vector<Token> tokens;
			/*
			string with tokens to analyze
			*/
			std::string stream;
			/*
			error token which is returned then Peek() is called out of array range
			*/
			Token EOFtoken;
			/*
			inner pos of token array. Is used by Peek() method to return array element
			*/
			int iteratorPos;
			/*
			current line in source file. Is used by GetLineCount()
			*/
			int lineCount;
		public:
			/*
			creates lexer object using source code from StreamReader object
			*/
			Lexer(const std::string& stream);

			/*
			returns current token or EOFToken if out of range
			*/
			Token& Peek();
			/*
			skips current token to next one
			*/
			void Next();
			/*
			returns to previous token
			*/
			void Prev();
			/*
			starts processing tokens from the beggining
			*/
			void ToBegin();
			/*
			skips to next line and returns current (counting from Peek())
			*/
			std::string ToNextLine();
			/*
			returns to the beggining of current line and returns contents between it and Peek() pos
			*/
			std::string ToPrevLine();
			/*
			sets iterator to specific pos (it can be get by GetIteratorPos() method)
			this method is not recommended to use, because it does not skip ENDLINE tokens,
			use Next() / Prev() / ToNextLine() / ToPrevLine() instead
			*/
			void SetIteratorPos(int pos);
			/*
			replace all string aliases which was set by StreamReader to their actual contents (stringMap can be get by StreamReader)
			*/
			void ReplaceStrings(const std::unordered_map<std::string, std::string>& stringMap);
			/*
			returns true if the iterator reaches end of the token array, false either
			*/
			bool End() const;
			/*
			returns true if the iterator reaches begin of the token array, false either
			*/
			bool Begin() const;
			/*
			returns amount of lines since begin of the source file (counts from 0)
			*/
			int GetLineCount() const;
			/*
			returns current iterator pos in the token array
			this method is not recommended to use due to potential errors,
			consider using Peek() instead
			*/
			int GetIteratorPos() const;
			/*
			prints all tokens which were successfully analysed in input stream
			*/
			std::string PrintFile();
		};
	}
}