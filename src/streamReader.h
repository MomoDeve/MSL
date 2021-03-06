#pragma once
#include "stringExtensions.h"
#include <unordered_map>
#include <sstream>

namespace MSL
{
	namespace compiler
	{
		/*
		StreamReader class is used to edit source code of the MSL program: 
		it deleted comments, replace strings with aliases and divide file content for lexer
		*/
		class StreamReader
		{
			/*
			buffer stores edited content of file and can be get by GetBuffer() method
			*/
			std::stringstream buffer;
			/*
			hash-table with string aliases. Is used by lexer to restore string constants
			*/
			std::unordered_map<std::string, std::string> replacedStrings;
			/*
			replace all strings with aliases and remove all comments from source code
			aliases can be viewed using hash-table 'replacedStrings'
			*/
			void ReplaceStrings();
		public:
			/*
			string prefix for string alias. User should never use such variable name in MSL cource code
			*/
			const std::string stringPrefix = STRING_PREFIX;
			/*
			token separator is used by lexer to indicate next token
			*/
			const char tokenSeparator = TOKEN_SEPARATOR;
			/*
			reads all contents of std::istream object. Edited content is inserted into StreamReader::buffer
			can be used more than once to read multiple files or input from console
			*/
			void ReadToEnd(std::istream& stream);
			/*
			returns buffer object as a string which can be passed into lexer
			*/
			std::string GetBuffer() const;
			/*
			return hash-table of string aliases which can be passed into lexer to restore string constants
			*/
			const std::unordered_map<std::string, std::string>& GetReplacedStrings() const;
		};
	}
}