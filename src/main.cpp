#include "streamReader.h"
#include "lexer.h"
#include "parser.h"
#include "VM.h"
#include "codeGenerator.h"
#include "bytecodeReader.h"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

void createAssembly(string filePath)
{
	ifstream file(filePath + ".msl");
	StreamReader reader;

	reader << file;
	file.close();

	Lexer lexer(reader.GetBuffer());
	lexer.ReplaceStrings(reader.GetReplacedStrings());

	Parser parser(&lexer, &cout, Parser::Mode::NO_DEBUG);
	parser.Parse();

	cout << endl;
	if (!parser.ParsingSuccess())
	{
		return;
	}
	Assembly assembly = parser.PullAssembly();
	for (const Namespace& _namespace : assembly.GetNamespaces())
	{
		cout << _namespace.toString() << "\n\n";
		for (const auto& member : _namespace.getMembers())
		{
			cout << member.toString() << endl;
		}
	}

	CodeGenerator generator(std::move(assembly));
	generator.GenerateBytecode(filePath + ".emsl");

	cout << endl;
}

int main()
{
	#define SIZE(T) cout << #T << ": " << sizeof(T) << endl

	string filePath = "main";
	createAssembly(filePath);
	BytecodeReader reader(filePath + ".emsl");
	std::ofstream binary("main_binary.bmsl");
	reader.Read(binary);
	binary.close();
	system("pause");
}