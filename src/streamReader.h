#pragma once

#include <istream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "stringExtensions.h"

#define DOUBLE_OPERATORS "&|=<>%+-*/^!~"

class StreamReader
{
	std::stringstream buffer;
	std::unordered_map<std::string, std::string> replacedStrings;

	void ReplaceStrings();
	void RemoveExtraSpaces();
	void SeparateTokens();
public:
	const std::string stringPrefix = STRING_PREFIX;
	const char tokenSeparator = TOKEN_SEPARATOR;

	StreamReader();
	void ReadToEnd(std::istream& stream);

	StreamReader& operator<<(std::istream& stream);

	std::string GetBuffer() const;
	const std::unordered_map<std::string, std::string>& GetReplacedStrings() const;
};