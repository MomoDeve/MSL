#pragma once
#include <ostream>
#include <vector>
#include <string>
#include <functional>

namespace MSL
{
	namespace utils
	{
		#define STRING_PREFIX "__USER_DEFINED_STRING"
		#define TOKEN_SEPARATOR ' '
		
		#define STRING(x) #x
		#define STRBOOL(x) ((x) ? "true" : "false")
		/*
		writes string object to std::ostream and adds '\n' to the end
		*/
		void writeToStream(std::ostream* stream, std::string message);
		/*
		deletes all symbols from seps string from the beggining and end of str object. Stops when meets different symbol
		*/
		void trim(std::string& str, const std::string& seps);
		/*
		Performs linear search in string array for a specific string. Returns true if match was found, false either
		*/
		bool find(const std::vector<std::string>& args, const std::string& arg);
		/*
		Performs linear search in string for a specific symbol. Returns true if match was found, false either
		*/
		bool contains(const std::string& str, char c);
		/*
		checks if the symbol can be used in variable name in MSL language
		*/
		bool validVariableCharacter(char c);
		/*
		replaces tokens such as '\n', '\t' with two symbols as '\' 't' etc
		*/
		std::string replaceEscapeTokens(const std::string& str);
		/*
		erases extra spaces in line of code (for example: `Console . Print ( arg ) ;` becomes `Console.Print(arg);`)
		*/
		std::string beautify(const std::string& str);
		/*
		move offset of str parameter until pred is true and appends content to the resulting string
		*/
		std::string readIf(const std::string& str, int& offset, std::function<bool(char)> pred);
		/*
		reads word (variable name) moving str offset and appends content to the resulting string
		*/
		std::string readWord(const std::string& str, int& offset);
		/*
		reads numeric constant (float / integer) moving str offset and appends content to the resulting string
		*/
		std::string readNum(const std::string& str, int& offset);
		/*
		reads op (one-symbol or multiple-symbol) moving str offset and appends content to the resulting string
		*/
		std::string readOp(const std::string& str, int& offset);
		/*
		checks if str can be successfully parsed to integer
		*/
		bool isInteger(const std::string& str);
		/*
		checks if str can be successfully parsed to float (not integer)
		*/
		bool isFloat(const std::string& str);
		/*
		checks if str is StreamReader-generated string alias
		*/
		bool isString(const std::string& str);
		/*
		checks if str is a valid variable name in MSL language
		*/
		bool isObject(const std::string& str);
		/*
		outputs amount of bytes with convertion to KB / MB / GB if nessasary
		*/
		std::string formatBytes(uint64_t bytes);
	}
}