#include "lexer.h"
#include "streamReader.h"
#include "parser.h"
#include "virtualMachine.h"
#include "bytecodeReader.h"
#include "codeGenerator.h"

using namespace std;

#define MSL_VM_DEBUG

bool createAssembly(string fileName)
{
	ifstream file(fileName + ".msl");
	MSL::compiler::StreamReader reader;

	reader.ReadToEnd(file);
	file.close();

	MSL::compiler::Lexer lexer(reader.GetBuffer());
	lexer.ReplaceStrings(reader.GetReplacedStrings());

	MSL::compiler::Parser parser(&lexer, &cout, MSL::compiler::Parser::MODE::ERROR_ONLY);
	parser.Parse();

	if (!parser.ParsingSuccess())
	{
		return false;
	}
	MSL::compiler::Assembly assembly = parser.PullAssembly();

	MSL::compiler::CodeGenerator generator(assembly);
	generator.GenerateBytecode();

	ofstream binary(fileName + ".emsl", ios::binary);
	auto contents = generator.GetBuffer();
	binary.write(contents.c_str(), contents.size());
	binary.close();

#ifdef MSL_VM_DEBUG
	ofstream out(fileName + ".bmsl");
	MSL::utils::BytecodeReader breader(fileName + ".emsl");
	breader.ReadToEnd(out);
#endif

	return true;
}

void compileASC(stringstream& ss)
{
	MSL::compiler::StreamReader reader;

	reader.ReadToEnd(ss);

	MSL::compiler::Lexer lexer(reader.GetBuffer());
	lexer.ReplaceStrings(reader.GetReplacedStrings());

	MSL::compiler::Parser parser(&lexer, &cout, MSL::compiler::Parser::MODE::ERROR_ONLY);
	parser.Parse();

	if (!parser.ParsingSuccess())
	{
		return;
	}
	MSL::compiler::Assembly assembly = parser.PullAssembly();

	MSL::compiler::CodeGenerator generator(assembly);
	generator.GenerateBytecode();

	stringstream bytecode;
	bytecode << generator.GetBuffer();
	MSL::VM::Configuration config;
	config.streams = { &std::cin, &std::cout, &std::cout };
	config.execution.safeMode = true;
	config.GC.maxMemory = 128 * MSL::VM::MB;
	MSL::VM::VirtualMachine VM(move(config));
	cout << endl;
	if (VM.AddBytecodeFile(&bytecode))
	{
		VM.Run();
	}
}

void compileFromArgs(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "MSL compiler usage:" << endl;
		cout << "> MSL compile [file1.msl] [file2.msl] ... - compiles source files to .emsl" << endl;
		cout << "> MSL run [file1.emsl] [file2.emsl] ... - runs bytecode files in VM" << endl;
		cout << "> MSL vm [file1.msl] [file2.msl] ... - compiles and runs files in VM" << endl;
		return;
	}
	std::string command = argv[1];

	if (command != "compile" && command != "vm" && command != "run")
	{
		cout << "invalid command: " << argv[1] << endl;
		cout << "launch program with no args for full usage documentation" << endl;
		return;
	}

	std::vector<std::string> executables;

	if (command == "compile" || command == "vm")
	{
		for (int i = 2; i < argc; i++)
		{
			std::string fileName = argv[i];
			if (fileName.size() <= 4 || fileName.substr(fileName.size() - 4, fileName.size()) != ".msl")
			{
				cout << "invalid filename: " << fileName << endl;
				return;
			}
			fileName = fileName.substr(0, fileName.size() - 4);
			if (!createAssembly(fileName)) return;
			cout << argv[i] << " compiled" << endl;
			executables.push_back(fileName + ".emsl");
		}
	}
	else if (command == "run")
	{
		for (int i = 2; i < argc; i++)
		{
			std::string fileName = argv[i];
			if (fileName.size() <= 5 || fileName.substr(fileName.size() - 5, fileName.size()) != ".emsl")
			{
				cout << "invalid filename: " << fileName << endl;
				return;
			}
			executables.push_back(fileName);
		}
	}

	if (command == "vm" || command == "run")
	{
		MSL::VM::Configuration config;
		config.streams = { &std::cin, &std::cout, &std::cerr };
		config.GC.maxMemory = 128 * MSL::VM::MB;
		MSL::VM::VirtualMachine VM(move(config));
		for (string& file : executables)
		{
			ifstream executable(file, ios::binary);
			if (!VM.AddBytecodeFile(&executable)) return;
		}
		cout << "launching MSL VM...\n\n";

		// small example of VM C++ function call
		VM.AddExternalFunction("ExternalFunctions", "HelloWorld", [](MSL::VM::VirtualMachine* vm) 
		{
			auto Console = vm->GetClassOrNull("System", "Console");
			vm->GetObjectStack().push_back(Console->wrapper);
			vm->GetObjectStack().push_back(vm->AllocString("hello world!"));
			vm->InvokeStaticMethod("PrintLine_1", Console);
		});
		///////////////////////////////////////////
		VM.Run();

		/*auto errors = VM.GetErrorStrings(VM.GetErrors());
		if (!errors.empty())
		{
			cout << "[VM ERRORS]:\n";
			for (const auto& error : errors)
			{
				cout << error << std::endl;
			}
		}*/
	}
}

int main(int argc, char* argv[])
{
	compileFromArgs(argc, argv); system("pause"); return 0;

	stringstream ss;
	std::string line;
	while (std::getline(std::cin, line))
	{
		if (line == "MSL_END_FILEREAD") break;
		ss << line << '\n';
	}
	compileASC(ss);
}