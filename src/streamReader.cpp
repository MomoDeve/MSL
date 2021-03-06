#include "streamReader.h"
#include <unordered_map>

using namespace MSL::utils;

namespace MSL
{
	namespace compiler
	{
		#define DOUBLE_OPERATORS "&|=<>%+-*/^!~"

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
					userString += c;
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
                    else if (c == '\n')
                    {
                        tempBuffer << '\n';
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
	}
}