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

	std::string replaceEscapeTokens(const std::string& str)
	{
		std::string res;
		if (str.empty()) return res;
		res.reserve(str.size());
		res += str[0];
		for (size_t i = 1; i < str.size(); i++)
		{
			if (str[i - 1] == '\\')
			{
				switch (str[i])
				{
				case 'n':
					res.back() = '\n';
					break;
				case 't':
					res.back() = '\t';
					break;
				case 'r':
					res.back() = '\r';
					break;
				case '\\':
					res.back() = '\\';
					i++;
					if (i < str.size())
						res += str[i];
					break;
				case '\'':
					res.back() = '\'';
					break;
				case '\"':
					res.back() = '\"';
					break;
				default:
					res += str[i];
					break;
				}
			}
			else
			{
				res += str[i];
			}
		}
		res.shrink_to_fit();
		return res;
	}

	std::string beautify(const std::string& str)
	{
		std::stringstream out;
		for (int i = 0; i < int(str.size()); i++)
		{
			if(i + 1 < (int)str.size())
			{
				char cur = str[i];
				char next = str[i + 1];
				if (cur == ' ' && contains(";,.()[]", next))
				{
					continue;
				}
				if (next == ' ' && contains("[(.", cur))
				{
					out << cur;
					i++;
					continue;
				}
			}
			out << str[i];
		}
		return out.str();
		#undef PRED
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
	std::string formatBytes(uint64_t bytes)
	{
		int KB = 1024;
		int MB = 1024 * 1024;
		int GB = 1024 * 1024 * 1024;
		if (bytes / GB > 0)
		{
			int amount = bytes * 100 / GB;
			return std::to_string(amount / 100) + '.' + std::to_string(amount % 100) + " GB";
		}
		else if (bytes / MB > 0)
		{
			int amount = bytes * 100 / MB;
			return std::to_string(amount / 100) + '.' + std::to_string(amount % 100) + " MB";
		}
		else if (bytes / KB > 0)
		{
			int amount = bytes * 100 / KB;
			return std::to_string(amount / 100) + '.' + std::to_string(amount % 100) + " KB";
		}
		else
		{
			return std::to_string(bytes) + " bytes";
		}
	}
}