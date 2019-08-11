#pragma once

#include "opcode.h"
#include "stringExtensions.h"

#include <fstream>
#include <bitset>

namespace MSL
{
	/*
	BytecodeReader class is used to generate human-read representation of msl binary file
	if reader finds an error in file, it stops and generates last line with content [[ unresolved instruction ]]
	*/
	class BytecodeReader
	{
		/*
		file object for read bytecode output
		*/
		std::ifstream file;
		/*
		reads next opcode in binary file
		*/
		VM::OPCODE ReadOPCode();
		/*
		reads next size_t variable in binary file
		*/
		size_t ReadSize();
		/*
		reads next modifier variable in binary file as uint8_t
		*/
		uint8_t ReadModifiers();
		/*
		reads next string in binary file as byte-array with provided size
		*/
		std::string ReadString(size_t size);
	public:
		/*
		creates BytecodeReader object and opens file with fileName provided
		*/
		BytecodeReader(const std::string& fileName);
		/*
		destroys object and closes binary file passed on construction
		*/
		~BytecodeReader();
		/*
		opens new binary file. Previous file is automatically closed
		*/
		void Open(const std::string& fileName);
		/*
		closes binary file. Close() is automatically called on object destruction
		*/
		void Close();
		/*
		reads binary file. Readable content is passed to ostream provided
		*/
		void ReadToEnd(std::ostream& out);
		/*
		reads binary file. Readable content is passed to ostream provided
		*/
		friend std::ostream& operator<<(std::ostream& out, BytecodeReader& reader);
		/*
		reads T type object from binary file using reinterpret_cast
		*/
		template<typename T>
		T GenericRead()
		{
			T variable;
			file.read(reinterpret_cast<char*>(&variable), sizeof(variable));
			return variable;
		}
	};
}