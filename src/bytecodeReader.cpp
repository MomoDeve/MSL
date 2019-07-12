#include "bytecodeReader.h"

BytecodeReader::BytecodeReader(const std::string& fileName)
{
	file.open(fileName);
}

BytecodeReader::~BytecodeReader()
{
	Close();
}

void BytecodeReader::Open(const std::string& fileName)
{
	if (!file.is_open())
	{
		file.open(fileName,std::ios::binary);
	}
}

void BytecodeReader::Close()
{
	if (file.is_open())
	{
		file.close();
	}
}

void BytecodeReader::Read(std::ostream& out)
{
	OPCODE op;
	size_t stringSize;
	while (!file.eof())
	{
		op = ReadOPCode();
		#define OPCODE_CASE(OP) case OP: out << STRING(OP) << ' ';
		#define READ_SIZE_OPCODE(OP) OPCODE_CASE(OP) out << ReadSize();
		#define READ_HASH_CASE(OP) OPCODE_CASE(OP) out << '#' << ReadSize();
		#define BINARY(object, size) std::bitset<8 * size>(object)
		switch (op)
		{
		OPCODE_CASE(ERROR_SYMBOL)
			break;
		OPCODE_CASE(PUSH_NULL)
			break;
		OPCODE_CASE(PUSH_TRUE)
			break;
		OPCODE_CASE(PUSH_FALSE)
			break;
		OPCODE_CASE(POP_TO_RETURN)
			break;
		OPCODE_CASE(ASSEMBLY_BEGIN_DECL)
			break;
		OPCODE_CASE(ASSEMBLY_END_DECL)
			return; // no more read after this opcode
		OPCODE_CASE(METHOD_BODY_BEGIN_DECL)
			break;
		OPCODE_CASE(METHOD_BODY_END_DECL)
			break;	
		OPCODE_CASE(NEGATION_OP)
			break;
		OPCODE_CASE(NEGATIVE_OP)
			break;
		OPCODE_CASE(POSITIVE_OP)
			break;
		OPCODE_CASE(ALLOC_PUSH)
			break;
		OPCODE_CASE(SUM_OP)
			break;
		OPCODE_CASE(SUB_OP)
			break;
		OPCODE_CASE(MULT_OP)
			break;
		OPCODE_CASE(DIV_OP)
			break;
		OPCODE_CASE(MOD_OP)
			break;
		OPCODE_CASE(POWER_OP)
			break;
		OPCODE_CASE(ASSIGN_OP)
			break;
		OPCODE_CASE(SET_ALU_INCR)
			break;
		READ_HASH_CASE(PUSH_STRING)
			break;
		READ_HASH_CASE(PUSH_INTEGER)
			break;
		READ_HASH_CASE(PUSH_FLOAT)
			break;
		READ_HASH_CASE(PUSH_OBJECT)
			break;
		READ_HASH_CASE(SET_CONST_VAR)
			break;
		READ_HASH_CASE(SET_VAR)
			break;
		READ_SIZE_OPCODE(NAMESPACE_POOL_DECL_SIZE)
			break;
		READ_SIZE_OPCODE(CLASS_POOL_DECL_SIZE)
			break;
		READ_SIZE_OPCODE(ATTRIBUTE_POOL_DECL_SIZE)
			break;
		READ_SIZE_OPCODE(METHOD_POOL_DECL_SIZE)
			break;
		READ_SIZE_OPCODE(METHOD_PARAMS_DECL_SIZE)
			break;
		READ_SIZE_OPCODE(DEPENDENCY_POOL_DECL_SIZE)
			break;
		case STRING_DECL:
			out << STRING(STRING_DECL) << ' ';
			stringSize = GenericRead<uint8_t>();
			out << (size_t)stringSize << ' ';
			out << ReadString(stringSize);
			break;
		OPCODE_CASE(MODIFIERS_DECL)
			out << BINARY(ReadModifiers(), sizeof(uint8_t));
			break;
		default:
			out << "[[ unresolved instruction ]]: " << BINARY(op, sizeof(OPCODE));
			break;
		}
		out << '\n';
		#undef BINARY
		#undef READ_HASH_CASE
		#undef READ_SIZE_OPCODE
		#undef OPCODE_CASE
	}
}

OPCODE BytecodeReader::ReadOPCode()
{
	return GenericRead<OPCODE>();
}

size_t BytecodeReader::ReadSize()
{
	return GenericRead<size_t>();
}

uint8_t BytecodeReader::ReadModifiers()
{
	return GenericRead<uint8_t>();
}

std::string BytecodeReader::ReadString(size_t size)
{
	std::string res(size, '?');
	file.read(&res[0], size);
	return res;
}

std::ostream& operator<<(std::ostream& out, BytecodeReader& reader)
{
	reader.Read(out);
	return out;
}
