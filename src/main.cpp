#include "streamReader.h"
#include "lexer.h"
#include "codeGenerator.h"
#include "bytecodeReader.h"
#include "parser.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

void createAssembly(string filePath)
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
		return;
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
}


int main()
{
	#define SIZE(T) cout << #T << ": " << sizeof(T) << endl

	string filePath = "main";
	createAssembly(filePath);
	MSL::BytecodeReader reader(filePath + ".emsl");
	std::ofstream binary("main_binary.bmsl");
	reader.Read(binary);
	binary.close();
	system("pause");
}