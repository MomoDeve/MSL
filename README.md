# MSL
MSL is a dynamic-type OOP general-purpose programming language

This project is a pure-C/C++ compiler for MSL, and VM for running it

## Compiling your first program
To compile yout first program, firstly you should pass source code to MSL::compiler::StreamReader class:
```cpp
ifstream file("main.msl");
MSL::compiler::StreamReader reader;
reader.ReadToEnd(file);
file.close();
```
StreamReader reads file and removes data that should not be compiled (comments, extra spaces, tabs, empty lines) and also replaces string constants with unique identifiers. 

We can get buffer with source code from StreamReader using GetBuffer() method and pass it to Lexer class, which separate program into tokens:
```cpp
MSL::compiler::Lexer lexer(reader.GetBuffer());
lexer.ReplaceStrings(reader.GetReplacedStrings());
```
Notice that by now string constants are still not exist in lexer token array, so we need to replace them using Lexer::ReplaceStrings() method. All strings can be easily get using StreamReader::GetReplacedStrings() method from StreamReader object.

By now Lexer object contains all necessary information to Parse out program. For that we use MSL::compiler::Parser class:
```cs
MSL::compiler::Parser parser(&lexer, &cout, MSL::compiler::Parser::Mode::FULL_TRACE);
parser.Parse();
```
parser accepts lexer class object as a first parameter, output stream as a second parameter and output mode as a third parameter.

Lexer object is used to retrieve tokens of source program. Note that parser begins reading tokens from current lexer iterator position, so if you made any changes to lexer instance before, call lexer::ToBegin() method and only then start parsing program.

Output stream is used by parser to output any information: errors, warnings, debug messages and etc. If you do not want to get any additional information from parser, set outputStream to nullptr and mode to Parser::Mode::NO_OUTPUT:
```cpp
MSL::compiler::Parser parser(&lexer, nullptr, MSL::compiler::Parser::Mode::NO_OUTPUT);
```
Parser mode specifies which information should be outputted by parser. You can combine different masks using byte-or operator ( | ) or use one of pre-defined constants:

- NO_OUTPUT - parser does not output any information to provided stream
- ERROR_ONLY - only errors are outputted
- WARNING_ONLY - only warnings are outputted
- DEBUG_ONLY - only debug information are outputted (added methods, classes, etc.)
- NO_STACKTRACE - force parser to output only the first line where error occured
- NO_DEBUG - output everything except debug information
- FULL_TRACE - output any information generated by parser

The state of parser can be get using ParsingSuccess() method. Note that if parser did not find Main method, only warning is generated, so be sure to check if Main function defined in the file by calling Parser::HasEntryPoint() method
```cpp
if (parser.ParsingSuccess() && parser.HasEntryPoint())
{
  // program assembly can be safely pulled
}
```
The last thing you need to do with your MSL code is to generate executable bytecode file. There is no way to directly pass Assembly from parser to the VM class as it cannot directly work with AST representation, so the CodeGenerator should be created:
```cpp
// generate bytecode from parsed assembly
MSL::compiler::Assembly assembly = parser.PullAssembly();
MSL::compiler::CodeGenerator generator(assembly);
generator.GenerateBytecode();

// write bytecode to the binary file
ofstream binary(fileName + ".emsl", ios::binary);
std::string bytecode = generator.GetBuffer();
binary.write(bytecode.c_str(), bytecode.size());
```
GenerateBytecode() writes MSL-bytecode to the CodeGenerator inner buffer in a binary format. This buffer is simply a string and can be easily written to a file (as in our example). If you have done all steps correctly and see no errors, this means that you have successfully compiled your MSL program to bytecode language.

Full program (supposing that main.msl file located in the same folder with the program):
```cpp
#include "streamReader.h"
#include "lexer.h"
#include "codeGenerator.h"
#include "bytecodeReader.h"
#include "parser.h"

int main()
{
    ifstream file("main.msl");
    MSL::compiler::StreamReader reader;
    reader.ReadToEnd(file);
    file.close();
    
    MSL::compiler::Lexer lexer(reader.GetBuffer());
    lexer.ReplaceStrings(reader.GetReplacedStrings());
    
    MSL::compiler::Parser parser(&lexer, &cout, MSL::compiler::Parser::Mode::FULL_TRACE);
    parser.Parse();
    
    if (parser.ParsingSuccess() && parser.HasEntryPoint())
    {
        // generate bytecode from parsed assembly
        MSL::compiler::Assembly assembly = parser.PullAssembly();
        MSL::compiler::CodeGenerator generator(assembly);
        generator.GenerateBytecode();

        // write bytecode to the binary file
        ofstream binary("main.emsl", ios::binary);
        std::string bytecode = generator.GetBuffer();
        binary.write(bytecode.c_str(), bytecode.size());
    }
    return 0;
}
```

## Writing Hello World in MSL
First program that you usually want to write to test new language and its compiler is a simple "Hello World" program. In MSL language this program will look like this:
```cs
// main.msl file
namespace Program
{	
    internal class ProgramClass
    {			
        public static function Main()
        {
            System.Console.PrintLine("Hello World");
        }
   }
}
```
Firstly, we declare namespace, in which our class will be located. Namespaces must be unique among program, but classes can have same names if they are located in different namespaces. We create Program.ProgramClass class and define one function in it - *Main*. It accepts zero parameters and is called by VM when the execution starts. As no instance of our class is implicitly created, entry-point function must be declared as *static*.

MSL VM creates one namespace by default - System. All utility classes such as **IO**, **Reflection**, **Array** and etc. are located in it. For our program we need **Console** class with its method called *PrintLine*, which outputs argument to the standart console. As we just want to output simple text, we can pass string literal as the method argument.

## Running MSL bytecode in VM
Right now we are one step away from displaying result in out console. We have main.emsl file which contains MSL bytecode for virtual machine, but we have not provided it to VM yet. Let's create an instance of MSL VM in our .cpp file:
```cpp
MSL::VM::Configuration config;
MSL::VM::VirtualMachine VM(move(config));
```
VM constuctor accepts config as an argument, but for our purposes we do not have to change it in any way. For more info you can see full documentation. 

Assuming that bytecode located in *main.emsl* file, we can open it with std::ifstream and pass it to AddBytecodeFile() method:
```cpp
std::ifstream fs("main.emsl", std::ios::binary);
if (!VM.AddBytecodeFile(&fs))
{
   VM.Run();
}
```
It returns true as success indicator, and that means that bytecode was loaded and can be executed. To do this, we simply need to call Run() method, and then wait for the result:

![Hello World](https://user-images.githubusercontent.com/40578274/67675950-81840d80-f991-11e9-8fa9-144b89b163df.png)
Notice that VM states that Main function returned 0 even if we did not specify return value. Thats because all functions return null by default, which in this case can be interpreted as 0. 

Any VM errors that have occured during the run of the program, can be retrieved as an array using GetErrors() and GetErrorStrings() methods after end of the execution.
Full code for VM launch with error display:
```cpp
int main()
{
    std::ifstream fs("main.emsl", std::ios::binary);
    MSL::VM::Configuration config;
    MSL::VM::VirtualMachine VM(std::move(config));
    if (VM.AddBytecodeFile(&fs))
    {
        VM.Run();
    }
    auto errors = VM.GetErrorStrings(VM.GetErrors());
    if (!errors.empty())
    {
        std::cerr << "[VM ERRORS]:\n";
        for (const std::string& error : errors)
        {
            std::cerr << error << std::endl;
        }
    }
    return 0;
}
```
Basically, that is the all code that you have to write to compile MSL source code and run it in VM. With this, now you can freely implement multiple file compilation or change output streams of the VM using Configuration class. For more additional info you can refer to full MSL documentation or ask *me* question personally.
