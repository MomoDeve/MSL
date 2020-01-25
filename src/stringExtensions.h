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
		replaces tokens such as '\n', '\t' with two symbols as '\' 't' etc
		*/
		std::string replaceEscapeTokens(const std::string& str);
		/*
		erases extra spaces in line of code (for example: `Console . Print ( arg ) ;` becomes `Console.Print(arg);`)
		*/
		std::string beautify(const std::string& str);
		/*
		outputs amount of bytes with convertion to KB / MB / GB if nessasary
		*/
		std::string formatBytes(uint64_t bytes);
	}
}