#pragma once

#include <vector>
#include <string>

namespace MSL
{
	namespace VM
	{
		class ExceptionTrace
		{
			std::vector<std::string> innerArgs;
		public:
			std::string& GetMessage();
			std::string& GetArgument();
			std::string& GetTraceEntry(size_t index);
			std::string& GetErrorType();
			size_t GetTraceSize();
			void Init(const std::string& message, const std::string& arg, const std::string& error);
			void AddTraceEntry(const std::string& entry);
		};
	}
}