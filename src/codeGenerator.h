#pragma once

#include "assembly.h"
#include "opcode.h"
#include <sstream>

namespace MSL
{
	namespace compiler
	{
		/*
		CodeGenerator class is used to generate actual bytecode for msl program
		it uses assembly which can be get from Parser class and calls GenerateBytecode() methods of expressions to generate instructions
		bytecode is saved in a binary file and can be later read by VM or bytecode reader
		*/
		class CodeGenerator
		{
			/*
			assembly is an object of program, which is can be gained from parser
			*/
			const Assembly& assembly;
			/*
			stream for writing bytecode of assembly
			*/
			std::stringstream out;
			/*
			generates a list of namespaces from assembly provided (see CodeGenerator::assembly)
			*/
			void GenerateNamespacePool();
			/*
			generated a list of classes for a namespace passed as parameter
			*/
			void GenerateClassPool(const Namespace& _namespace);
			/*
			generated a list of attributes for a class passed as parameter
			*/
			void GenetateAttributePool(const Class& _class);
			/*
			generated a list of methods for a class passed as parameter
			*/
			void GenerateMethodPool(const Class& _class);
			/*
			generated method parameters and body for a method passed as parameter
			*/
			void GenerateMethod(const Function& method);
		public:
			/*
			created CodeGenerator object. Assembly is used for code generation
			*/
			CodeGenerator(const Assembly& assembly);
			/*
			Generated bytecode from assembly and writes in to the binary file with name provided
			*/
			void GenerateBytecode();
			/*
			Returns contents of stream with assembly
			*/
			std::string GetBuffer() const;
			/*
			writes binary representation of data to the binary file
			*/
			template<typename T>
			void write(T data);
			/*
			writes string object to the binary file in format [uint8_t size][char[] char_array]
			*/
			void writeString(const std::string& data);
		};

		/*
		generic method for writing binary data to file
		*/
		template<typename T>
		inline void CodeGenerator::write(T data)
		{
			out.write(reinterpret_cast<char*>(&data), sizeof(T));
		}
	}
}