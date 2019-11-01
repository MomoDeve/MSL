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

bool createAssembly(string fileName)
{
	ifstream file(fileName + ".msl");
	MSL::compiler::StreamReader reader;

	reader.ReadToEnd(file);
	file.close();

	MSL::compiler::Lexer lexer(reader.GetBuffer());
	lexer.ReplaceStrings(reader.GetReplacedStrings());

	MSL::compiler::Parser parser(&lexer, &cout, MSL::compiler::Parser::Mode::ERROR_ONLY);
	parser.Parse();

	if (!parser.ParsingSuccess())
	{
		return false;
	}
	MSL::compiler::Assembly assembly = parser.PullAssembly();
	
	#ifdef MSL_VM_DEBUG
	{
		for (const auto& _namespace : assembly.GetNamespaces())
		{
			cout << _namespace.toString() << "\n\n";
			for (const auto& member : _namespace.getMembers())
			{
				cout << member.ToString() << endl;
			}
		}
	}
	#endif

	MSL::compiler::CodeGenerator generator(assembly);
	generator.GenerateBytecode();

	ofstream binary(fileName + ".emsl", ios::binary);
	auto contents = generator.GetBuffer();
	binary.write(contents.c_str(), contents.size());
	return true;
}

int main(int argc, char* argv[])
{
	string fileName = "main";
	if (argc == 2)
	{
		fileName = argv[1];
		fileName = fileName.substr(0, fileName.size() - 4); // delete ".msl" from file name
	}

	if (createAssembly(fileName))
	{
		MSL::BytecodeReader reader(fileName + ".emsl");
		std::ofstream binary(fileName + "_binary.bmsl");
		reader.ReadToEnd(binary);
		binary.close();

		MSL::VM::Configuration config;
		config.streams = { &std::cin, &std::cout, &std::cerr };
		MSL::VM::VirtualMachine VM(move(config));
		std::ifstream executable(fileName + ".emsl", std::ios::binary);
		if (VM.AddBytecodeFile(&executable))
		{
			VM.Run();
		}
		auto errors = VM.GetErrorStrings(VM.GetErrors());
		if (!errors.empty())
		{
			cout << "[VM ERRORS]:\n";
			for (const auto& error : errors)
			{
				cout << error << std::endl;
			}
		}
	}
	int unused = getchar();
}