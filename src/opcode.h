#pragma once

#include <vector>
#include <string>

namespace MSL
{
	namespace VM
	{
		/*
		list of all bytecode instructions
		*/
		enum OPCODE : uint8_t
		{
			ERROR_SYMBOL = 33, // symbol used to state that bytecode was not properly generated
			ASSEMBLY_BEGIN_DECL, // states that method body has been begun
			ASSEMBLY_END_DECL, // states that assembly has been ended
			NAMESPACE_POOL_DECL_SIZE, // declares namespace pool with size following by this opcode
			FRIEND_POOL_DECL_SIZE, // declares friend namespaces names pool with size following by this opcode
			CLASS_POOL_DECL_SIZE, // declares class pool with size following by this opcode
			ATTRIBUTE_POOL_DECL_SIZE, // declares attribute pool with size following by this opcode
			METHOD_POOL_DECL_SIZE, // declares method pool with size following by this opcode
			METHOD_PARAMS_DECL_SIZE, // declares method parameter array with size following by this opcode
			DEPENDENCY_POOL_DECL_SIZE, // declares string pool with size following by this opcode
			METHOD_BODY_BEGIN_DECL, // states that method body has been begun
			METHOD_BODY_END_DECL, // states that method body has been ended
			STRING_DECL, // states that next byte will declare string size and next N bytes will be string characters
			MODIFIERS_DECL, // states that next byte will declare modifiers
			PUSH_STRING, // pushes STRING object on top of the stack with value provided
			PUSH_INTEGER, // pushes INTEGER object on top of the stack with value provided
			PUSH_FLOAT, // pushes FLOAT object on top of the stack with value provided
			PUSH_OBJECT, // pushes object on top of the stack by name provided (search can be performed later)
			PUSH_THIS, // pushes THIS object on top of the stack
			PUSH_NULL, // pushes NULL object on top of the stack
			PUSH_TRUE, // pushes TRUE object on top of the stack
			PUSH_FALSE, // pushes FALSE object on top of the stack
			POP_TO_RETURN, // use object on stack top to return it from method to caller code
			ALLOC_VAR, // allocates local variable
			ALLOC_CONST_VAR, // allocates local variable which cannot be reassigned (with const modifier)
			NEGATION_OP, // applies operator `!` to object or calls NegationOperator() method for classes
			NEGATIVE_OP, // applies operator `-` to object or calls NegOperator() method for classes
			POSITIVE_OP, // applies operator `+` to object or calls PosOperator() method for classes
			SUM_OP, // computes sum result of two objects or calls SumOperator() method for classes
			SUB_OP, // computes substraction result of two objects or calls SubOperator() method for classes
			MULT_OP, // computes remainder of division of two objects or calls MultOperator() method for classes
			DIV_OP, // computes divised part of division of two objects or calls DivOperator() method for classes
			MOD_OP, // computes remainder of division of two objects or calls ModOperator() method for classes
			POWER_OP, // raises object to the power or calls PowerOperator() method for classes
			ASSIGN_OP, // assigns value of obj2 to obj1: `obj1 = obj2`
			GET_MEMBER, // perform `dot operator` for classes / attributes / methods
			SET_ALU_INCR, // indicates that ALU also shouls assign result of operation to the first object
			CMP_EQ, // compares obj1 == obj2 as primitive types or by calling IsEqual() method for classes
			CMP_NEQ, // compares obj1 != obj2 as primitive types or by calling IsNotEqual() method for classes
			CMP_L, // compares obj1 < obj2 as primitive types or by calling IsLess() method for classes
			CMP_G, // compares obj1 > obj2 as primitive types or by calling IsGreater() method for classes
			CMP_LE, // compares obj1 <= obj2 as primitive types or by calling IsLessEqual() method for classes
			CMP_GE, // compares obj1 >= obj2 as primitive types or by calling IsGreaterEqual() method for classes
			CMP_AND, // AND two boolean values or object with AndOperator() method
			CMP_OR, // OR two boolean values or object with OrOperator() method
			GET_INDEX, // gets object by index calling GetByIndex() method of implementation-defined function for primitive types
			CALL_FUNCTION, // calls fuction by name used from top of the stack. Also pops N arguments, there N - number provided
			RETURN, // returns from method (pop stackframe). Pushes NULL on top of the stack
			SET_LABEL, // sets label index. should be ignored during execution
			JUMP, // jumps by label index
			JUMP_IF_TRUE, // pops stack top. if true -> jumps by label index, else does nothing
			JUMP_IF_FALSE, // pops stack top. if false -> jumps by label index, else does nothing
			POP_STACK_TOP, // pops top of the stack
		};

		/*
		returns string representation of opcode such that ToString(OPCODE::OP) = OP
		*/
		std::string ToString(OPCODE op);
		/*
		Array of opcodes
		*/
		typedef std::vector<OPCODE> InstructionVector;
	}
}