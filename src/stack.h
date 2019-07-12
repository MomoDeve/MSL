#pragma once

#include <stdint.h>

typedef uint8_t Byte;

template<uint64_t stackSize>
class Stack
{
	Byte* stack;
	Byte* stackPtr;
public:
	Stack();
	void Push(Byte* bytes, size_t byteSize);
	void PtrMove(int offset);
	void Clear();
};

template<uint64_t stackSize>
inline Stack<stackSize>::Stack()
	: stackPtr(0)
{
	stack = new Byte[stackSize];
}

template<uint64_t stackSize>
inline void Stack<stackSize>::Push(Byte* bytes, size_t byteSize)
{
	for (int i = 0; i < byteSize; i++, stackPtr++)
	{
		stack[stackPtr] = bytes[byteSize];
	}
}

template<uint64_t stackSize>
inline void Stack<stackSize>::PtrMove(int offset)
{
	stackPtr += offset;
}

template<uint64_t stackSize>
inline void Stack<stackSize>::Clear()
{
	stackPtr = 0;
}
