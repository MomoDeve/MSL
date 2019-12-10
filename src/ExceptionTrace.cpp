#include "ExceptionTrace.h"

std::string& MSL::VM::ExceptionTrace::GetErrorType()
{
	return innerArgs[0];
}

#undef GetMessage // winapi
std::string& MSL::VM::ExceptionTrace::GetMessage()
{
	return innerArgs[1];
}

std::string& MSL::VM::ExceptionTrace::GetArgument()
{
	return innerArgs[2];
}

std::string& MSL::VM::ExceptionTrace::GetTraceEntry(size_t index)
{
	return innerArgs[index + 3];
}

size_t MSL::VM::ExceptionTrace::GetTraceSize()
{
	return innerArgs.size() - 3;
}

void MSL::VM::ExceptionTrace::Init(const std::string& message, const std::string& arg, const std::string& error)
{
	innerArgs.clear();
	innerArgs.push_back(error);
	innerArgs.push_back(message);
	innerArgs.push_back(arg);
}

void MSL::VM::ExceptionTrace::AddTraceEntry(const std::string& entry)
{
	innerArgs.push_back(entry);
}
