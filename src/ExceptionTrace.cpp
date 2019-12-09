#include "ExceptionTrace.h"

std::string& MSL::VM::ExceptionTrace::GetMessage()
{
	return innerArgs[0];
}

std::string& MSL::VM::ExceptionTrace::GetArgument()
{
	return innerArgs[1];
}

std::string& MSL::VM::ExceptionTrace::GetTraceEntry(size_t index)
{
	return innerArgs[index + 2];
}

size_t& MSL::VM::ExceptionTrace::GetErrorType()
{
	return errorType;
}

size_t MSL::VM::ExceptionTrace::GetTraceSize()
{
	return innerArgs.size() - 2;
}

void MSL::VM::ExceptionTrace::Init(const std::string& message, const std::string& arg, size_t error)
{
	innerArgs.clear();
	innerArgs.push_back(message);
	innerArgs.push_back(arg);
	errorType = error;
}

void MSL::VM::ExceptionTrace::AddTraceEntry(const std::string& entry)
{
	innerArgs.push_back(entry);
}
