#pragma once

#include <stdint.h>
#include <vector>

enum OPCODE : uint8_t
{
	ERROR_SYMBOL = 0,
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
	PUSH_STRING,
	PUSH_INTEGER,
	PUSH_FLOAT,
	PUSH_OBJECT,
	PUSH_FUNCTION, 
	PUSH_THIS,
	PUSH_NULL,
	PUSH_TRUE,
	PUSH_FALSE,
	POP_TO_RETURN,
	POP_STACK_TOP,
	SET_VAR,
	SET_CONST_VAR,
	NEGATION_OP = 27, // 26 means EOF ! okay ...,
	NEGATIVE_OP,
	POSITIVE_OP,
	ALLOC_PUSH,
	SUM_OP,
	SUB_OP,
	MULT_OP,
	DIV_OP,
	MOD_OP,
	POWER_OP,
	ASSIGN_OP,
	GET_MEMBER, // if stack top was function, than call it, after push its return value. Either push member 
	SET_ALU_INCR,
	CMP_EQ,
	CMP_NEQ,
	CMP_L,
	CMP_G,
	CMP_LE,
	CMP_GE,
	CMP_AND,
	CMP_OR, 
	GET_INDEX,
};

struct Instruction
{
	uint8_t OPCode;
	Instruction(OPCODE code);
};

typedef std::vector<Instruction> InstructionVector;