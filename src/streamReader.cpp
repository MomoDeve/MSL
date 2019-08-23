#include "streamReader.h"

namespace MSL
{
	namespace compiler
	{
		std::string StreamReader::GetBuffer() const
		{
			return buffer.str();
		}

		const std::unordered_map<std::string, std::string>& StreamReader::GetReplacedStrings() const
		{
			return replacedStrings;
		}

		void StreamReader::ReadToEnd(std::istream& stream)
		{
			int c;
			while ((c = stream.get()) != EOF)
				buffer << (char)c;

			ReplaceStrings();
			RemoveExtraSpaces();
			SeparateTokens();
		}

		void StreamReader::RemoveExtraSpaces()
		{
			std::stringstream tempBuffer;
			const std::string spaces = "\t\r ";
			bool space = false;
			std::string str = buffer.str();
			trim(str, spaces);
			for (char c : str)
			{
				if (!contains(spaces, c))
				{
					tempBuffer << c;
					space = false;
				}
				else if (!space)
				{
					space = true;
					tempBuffer << ' ';
				}
			}
			buffer.swap(tempBuffer);
		}

		void StreamReader::SeparateTokens()
		{
			std::stringstream tempBuffer;
			std::string str = buffer.str();
			for (int i = 0; i < (int)str.size(); i++)
			{
				if (isdigit(str[i]))
				{
					tempBuffer << readNum(str, i);
				}
				else if (validVariableCharacter(str[i]))
				{
					tempBuffer << readWord(str, i);
				}
				else if (contains(DOUBLE_OPERATORS, str[i]))
				{
					tempBuffer << readOp(str, i);
				}
				else if (str[i] != ' ')
				{
					tempBuffer << str[i];
				}
				else continue;
				tempBuffer << tokenSeparator;
			}
			buffer.swap(tempBuffer);
		}

		void StreamReader::ReplaceStrings()
		{
			std::stringstream tempBuffer;
			std::string str = buffer.str();
			char prev = '\0';
			char tmp = '\0';
			bool insideString = false;
			bool blockComment = false;
			bool lineComment = false;
			std::string userString;
			for (char c : str)
			{
				if (insideString)
				{
					if (!userString.empty() && userString.back() == '\\')
					{
						switch (c)
						{
						case 'n':
							userString.back() = '\n';
							break;
						case 't':
							userString.back() = '\t';
							break;
						case '"':
							userString.back() = '\"';
							break;
						case 'r':
							userString.back() = '\r';
							break;
						default:
							userString += c;
							break;
						}
					}
					else
					{
						userString += c;
					}
				}
				else if (lineComment)
				{
					if (c == '\n')
					{
						lineComment = false;
					}
				}
				else if (blockComment)
				{
					if (prev == '*' && c == '/')
					{
						blockComment = false;
						c = '\0';
					}
				}
				else if (prev == '/' && c == '*')
				{
					blockComment = true;
					tmp = '\0';
				}
				else if (prev == '/' && c == '/')
				{
					lineComment = true;
					tmp = '\0';
				}
				if (!lineComment && !blockComment)
				{
					if (c == '\"' && prev != '\\')
					{
						insideString = !insideString;
						if (!insideString)
						{
							userString.erase(userString.size() - 1);
							userString.erase(0, 1);

							if (replacedStrings.find(userString) == replacedStrings.end())
							{
								replacedStrings[userString] = stringPrefix + std::to_string(replacedStrings.size());
							}
							tempBuffer << replacedStrings[userString];
							userString.clear();
							c = '\0';
						}
						else
						{
							if (prev == (char)32) prev = ' ';
							userString += prev;
						}
					}
				}
				prev = c;
				if (tmp != '\0')
				{
					tempBuffer << tmp;
					tmp = '\0';
				}
				if (!blockComment && !lineComment && !insideString && prev != '\0')
				{
					if (prev == '/')
					{
						tmp = prev;
					}
					else
					{
						tempBuffer << prev;
					}
				}
			}
			buffer.swap(tempBuffer);
		}

		StreamReader& StreamReader::operator<<(std::istream& stream)
		{
			ReadToEnd(stream);
			return *this;
		}
	}
}