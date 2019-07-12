#pragma once

#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>
#include <regex>

#define OPERATOR_LIST "!~/*+-=%&|^?<>;"

#define STRING_PREFIX "__USER_DEFINED_STRING"
#define TOKEN_SEPARATOR ' '

#define STRING(x) #x
#define BOOL(x) ((x) ? "true" : "false")

void writeToStream(std::ostream* stream, std::string message);

void trim(std::string& str, const std::string& seps);

bool find(const std::vector<std::string>& args, const std::string& arg);

bool contains(const std::string& str, char c);

bool validVariableCharacter(char c);

std::string readIf(const std::string& str, int& offset, std::function<bool(char)> pred);

std::string readWord(const std::string& str, int& offset);

std::string readNum(const std::string& str, int& offset);

std::string readOp(const std::string& str, int& offset);

bool isInteger(const std::string& str);

bool isFloat(const std::string& str);

bool isOperator(const std::string& str);

bool isString(const std::string& str);

bool isPunc(const std::string& str, const::std::vector<std::string>& puncList);

bool isKeyWord(const std::string& str, const::std::vector<std::string>& keyWords);

bool isObject(const std::string& str);