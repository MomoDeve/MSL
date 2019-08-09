#include "stringExtensions.h"

namespace MSL
{
	void writeToStream(std::ostream* stream, std::string message)
	{
		message += '\n';
		stream->write(message.c_str(), message.size());
	}

	void trim(std::string& str, const std::string& seps)
	{
		int i;
		for (i = 0; i < (int)str.size(); i++)
		{
			if (!contains(seps, str[i])) break;
		}
		if (i > 0) str.erase(0, i);

		for (i = str.size() - 1; i >= 0; i--)
		{
			if (!contains(seps, str[i])) break;
		}

		if (i != str.size() - 1) str.erase(i + 1);
	}

	bool find(const std::vector<std::string>& args, const std::string& arg)
	{
		return std::find(args.begin(), args.end(), arg) != args.end();
	}

	bool contains(const std::string& str, char c)
	{
		return str.find(c) != std::string::npos;
	}

	bool validVariableCharacter(char c)
	{
		return isalnum(c) || (c == '_');
	}

	bool validNumberCharacter(char c)
	{
		return isdigit(c) || contains("+-eE.", c);
	}

	std::string readIf(const std::string& str, int& offset, std::function<bool(char)> pred)
	{
		std::string res;
		for (; offset < (int)str.size(); offset++)
		{
			if (!pred(str[offset]))
				break;
			res += str[offset];
		}
		offset--;
		return res;
	}

	std::string readWord(const std::string& str, int& offset)
	{
		return readIf(str, offset, validVariableCharacter);
	}

	std::string readNum(const std::string& str, int& offset)
	{
		std::string res;
		res += readIf(str, offset, validNumberCharacter);
		return res;
	}

	std::string readOp(const std::string& str, int& offset)
	{
		if (offset < (int)str.size() - 1)
		{
			char c1 = str[offset], c2 = str[offset + 1];
			if (c2 == '=' || // any case such as +=, -=, etc...
				(c2 == c1 && contains("=&|*", c1))) // any case such as ==, &&, etc...
			{
				offset++;
				return std::string({ c1, c2 }); // c1 + c2 as string
			}
			else
			{
				return std::string(1, c1);
			}
		}
		else
		{
			return std::string(1, str[offset]);
		}
	}

	bool isInteger(const std::string& str)
	{
		return std::regex_match(str, std::regex(R"(\d+)"));
	}

	bool isFloat(const std::string& str)
	{
		return std::regex_match(str, std::regex(R"(\d+\.\d+([eE][+-]?\d+)?)"));
	}

	bool isString(const std::string& str)
	{
		return str.find(STRING_PREFIX) != std::string::npos;
	}

	bool isObject(const std::string& str)
	{
		return std::regex_match(str, std::regex(R"(\w+)"));
	}
}