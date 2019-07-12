#pragma once

#include <istream>
#include <vector>
#include <string>
#include <set>
#include <unordered_map>

#include "stringExtensions.h"
#include "token.h"

class Lexer
{
	std::vector<Token> tokens;
	std::string stream;
	Token EOFtoken;

	int iteratorPos;
	int lineCount;
public:
	Lexer(const std::string& stream);

	Token& Peek();
	void Next();
	void Prev();
	void toBegin();
	std::string ToNextLine();
	std::string ToPrevLine();
	void SetIteratorPos(int pos);
	void ReplaceStrings(const std::unordered_map<std::string, std::string>& stringMap);

	bool End() const;
	bool Begin() const;
	int GetLineCount() const;
	int GetIteratorPos() const;
};
