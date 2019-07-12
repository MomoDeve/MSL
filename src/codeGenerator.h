#pragma once

#include "assembly.h"
#include "instruction.h"
#include "bufferedFile.h"

class CodeGenerator
{
	Assembly assembly;
	BufferedFile out;

	void GenerateNamespacePool();
	void GenerateClassPool(const Namespace& _namespace);
	void GenetateAttributePool(const Class& _class);
	void GenerateMethod(const Function & method);
	void GenerateMethodPool(const Class& _class);

	template<typename T>
	void write(T data);
	void writeString(const std::string& data);
public:
	CodeGenerator(Assembly&& assembly);

	void GenerateBytecode(const std::string& outFileName);
};

template<typename T>
inline void CodeGenerator::write(T data)
{
	out.Write(reinterpret_cast<char*>(&data), sizeof(T));
}