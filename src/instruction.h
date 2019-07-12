#pragma once

#include <stdint.h>
#include <vector>

enum OPCODE : uint8_t
{
	ASSEMBLY_BEGIN_DECL,
	ASSEMBLY_END_DECL,
	NAMESPACE_POOL_DECL_SIZE,
	CLASS_POOL_DECL_SIZE,
	ATTRIBUTE_POOL_DECL_SIZE,
	METHOD_POOL_DECL_SIZE,
	METHOD_PARAMS_DECL_SIZE,
	DEPENDENCY_POOL_DECL_SIZE,
	METHOD_BODY_BEGIN_DECL,
	METHOD_BODY_END_DECL,
	STRING_DECL,
	MODIFIERS_DECL,
};

struct Instruction
{
	uint8_t OPCode;
	Instruction(OPCODE code);
};

typedef std::vector<Instruction> InstructionVector;