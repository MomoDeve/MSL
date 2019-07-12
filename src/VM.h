#pragma once

#include "instruction.h"
#include "stack.h"
#include "token.h"

#define INSTR_PTR REG[7]
#define STACK_PTR REG[6]
#define ERROR_PTR REG[5]

class VM
{
	enum ErrorCodes : uint32_t
	{
		TERMINATE = 0x80000000
	};
	static const size_t registerCount = 8;
	static const uint64_t stackSize = 1 << 24;

	Stack<stackSize> stack;
	uint32_t REG[registerCount];
	const InstructionVector bytecode;
public:
	static OPCODE GetOPCodeByToken(Token::Type type);

	VM(InstructionVector bytecode);
	void Start();
	void ProcessInstruction();
};