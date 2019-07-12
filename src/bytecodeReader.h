#pragma once

#include "instruction.h"
#include "stringExtensions.h"

#include <fstream>
#include <bitset>

class BytecodeReader
{
	std::ifstream file;

	OPCODE ReadOPCode();
	size_t ReadSize();
	uint8_t ReadModifiers();
	std::string ReadString(size_t size);
public:
	BytecodeReader(const std::string& fileName);
	~BytecodeReader();

	void Open(const std::string& fileName);
	void Close();
	void Read(std::ostream& out);
	friend std::ostream& operator<<(std::ostream& out, BytecodeReader& reader);

	template<typename T>
	T GenericRead()
	{
		T variable;
		file.read(reinterpret_cast<char*>(&variable), sizeof(variable));
		return variable;
	}
};