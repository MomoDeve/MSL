#include "streamReader.h"
#include "lexer.h"
#include "codeGenerator.h"
#include "bytecodeReader.h"
#include "parser.h"
#include "virtualMachine.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

void PrintErrors(uint32_t errors)
{
	cout << "[[ VM ERRORS ]]:" << endl;
	using ERROR = MSL::VM::VirtualMachine::ERROR;
	if (errors & ERROR::CALLSTACK_EMPTY)
		cout << STRING(ERROR::CALLSTACK_EMPTY) << endl; 
	if (errors & ERROR::INVALID_CALL_ARGUMENT)
		cout << STRING(ERROR::INVALID_CALL_ARGUMENT) << endl;
	if (errors & ERROR::INVALID_METHOD_SIGNATURE)
		cout << STRING(ERROR::INVALID_METHOD_SIGNATURE) << endl;
	if (errors & ERROR::INVALID_OPCODE)
		cout << STRING(ERROR::INVALID_OPCODE) << endl;
	if (errors & ERROR::INVALID_STACKFRAME_OFFSET)
		cout << STRING(ERROR::INVALID_STACKFRAME_OFFSET) << endl;
	if (errors & ERROR::OBJECTSTACK_EMPTY)
		cout << STRING(ERROR::OBJECTSTACK_EMPTY) << endl;
	if (errors & ERROR::OPERANDSTACK_CORRUPTION)
		cout << STRING(ERROR::OPERANDSTACK_CORRUPTION) << endl;
	if (errors & ERROR::TERMINATE_ON_LAUNCH)
		cout << STRING(ERROR::TERMINATE_ON_LAUNCH) << endl;
	if (errors & ERROR::OBJECT_NOT_FOUND)
		cout << STRING(ERROR::OBJECT_NOT_FOUND) << endl;
	if (errors & ERROR::INVALID_METHOD_SIGNATURE)
		cout << STRING(ERROR::INVALID_METHOD_SIGNATURE) << endl;
	if (errors & ERROR::MEMBER_NOT_FOUND)
		cout << STRING(ERROR::MEMBER_NOT_FOUND) << endl;
	if (errors & ERROR::INVALID_STACKOBJECT)
		cout << STRING(ERROR::INVALID_STACKOBJECT) << endl;
	if (errors & ERROR::STACKOVERFLOW)
		cout << STRING(ERROR::STACKOVERFLOW) << endl;
}

bool createAssembly(string filePath)
{
	ifstream file(filePath + ".msl");
	MSL::compiler::StreamReader reader;

	reader << file;
	file.close();

	MSL::compiler::Lexer lexer(reader.GetBuffer());
	lexer.ReplaceStrings(reader.GetReplacedStrings());

	MSL::compiler::Parser parser(&lexer, &cout, MSL::compiler::Parser::Mode::NO_DEBUG);
	parser.Parse();

	cout << endl;
	if (!parser.ParsingSuccess())
	{
		return false;
	}
	MSL::compiler::Assembly assembly = parser.PullAssembly();
	for (const auto& _namespace : assembly.GetNamespaces())
	{
		cout << _namespace.toString() << "\n\n";
		for (const auto& member : _namespace.getMembers())
		{
			cout << member.ToString() << endl;
		}
	}
	MSL::compiler::CodeGenerator generator(std::move(assembly));
	generator.GenerateBytecode(filePath + ".emsl");

	cout << endl;
	return true;
}

int main()
{
	#define SIZE(T) cout << #T << ": " << sizeof(T) << endl
	string filePath = "main";
	if (createAssembly(filePath))
	{
		MSL::BytecodeReader reader(filePath + ".emsl");
		std::ofstream binary("main_binary.bmsl");
		reader.ReadToEnd(binary);
		binary.close();

		MSL::VM::Configuration config;
		config.streams = { &std::cin, &std::cout, &std::cout };
		MSL::VM::VirtualMachine VM(move(config));
		std::ifstream executable(filePath + ".emsl", std::ios::binary);
		VM.AddBytecodeFile(&executable);
		VM.Run();
		PrintErrors(VM.GetErrors());
	}
	system("pause");
}