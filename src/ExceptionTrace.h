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
			size_t errorType;
		public:
			std::string& GetMessage();
			std::string& GetArgument();
			std::string& GetTraceEntry(size_t index);
			size_t& GetErrorType();
			size_t GetTraceSize();
			void Init(const std::string& message, const std::string& arg, size_t error);
			void AddTraceEntry(const std::string& entry);
		};
	}
}