#pragma once

#include <stdint.h>
#include <vector>

namespace MSL
{
	namespace VM
	{
		/*
		list of all bytecode instructions
		*/
		enum OPCODE : uint8_t
		{
			ERROR_SYMBOL = 33,
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
			PUSH_THIS,
			PUSH_NULL,
			PUSH_TRUE,
			PUSH_FALSE,
			POP_TO_RETURN,
			ALLOC_VAR,
			ALLOC_CONST_VAR,
			NEGATION_OP,
			NEGATIVE_OP,
			POSITIVE_OP,
			SUM_OP,
			SUB_OP,
			MULT_OP,
			DIV_OP,
			MOD_OP,
			POWER_OP,
			ASSIGN_OP,
			GET_MEMBER,
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
			CALL_FUNCTION,
			RETURN,
			SET_LABEL,
			JUMP,
			JUMP_IF_TRUE, //jump if pops stack top automatically
			JUMP_IF_FALSE,
			PUSH_STACKFRAME,
			POP_STACK_TOP,
		};

		std::string ToString(OPCODE op);

		typedef std::vector<OPCODE> InstructionVector;
	}
}