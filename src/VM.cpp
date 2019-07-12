#include "VM.h"

OPCODE VM::GetOPCodeByToken(Token::Type type)
{
	return OPCODE::NAMESPACE_POOL_DECL_SIZE;
}

VM::VM(InstructionVector bytecode)
	: bytecode(std::move(bytecode)) { }

void VM::Start()
{
	INSTR_PTR = 0;
	STACK_PTR = 0;
	while (!(ERROR_PTR & ErrorCodes::TERMINATE))
	{
		ProcessInstruction();
		INSTR_PTR++;
	}
}

void VM::ProcessInstruction()
{
	Instruction instruction = bytecode[INSTR_PTR];
}
