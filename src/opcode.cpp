#include "opcode.h"
#include "stringExtensions.h"

namespace MSL
{
	namespace VM
	{
		std::string ToString(OPCODE op)
		{
			#define RETURN(op) case op: return STRING(op);
			switch (op)
			{
			RETURN(ERROR_SYMBOL);				
			RETURN(ASSEMBLY_BEGIN_DECL);				
			RETURN(ASSEMBLY_END_DECL);				
			RETURN(NAMESPACE_POOL_DECL_SIZE);				
			RETURN(CLASS_POOL_DECL_SIZE);				
			RETURN(ATTRIBUTE_POOL_DECL_SIZE);				
			RETURN(METHOD_POOL_DECL_SIZE);				
			RETURN(METHOD_PARAMS_DECL_SIZE);			
			RETURN(DEPENDENCY_POOL_DECL_SIZE);				
			RETURN(METHOD_BODY_BEGIN_DECL);				
			RETURN(METHOD_BODY_END_DECL);				
			RETURN(STRING_DECL);				
			RETURN(MODIFIERS_DECL);				
			RETURN(PUSH_STRING);				
			RETURN(PUSH_INTEGER);				
			RETURN(PUSH_FLOAT);				
			RETURN(PUSH_OBJECT);				
			RETURN(PUSH_THIS);				
			RETURN(PUSH_NULL);				
			RETURN(PUSH_TRUE);				
			RETURN(PUSH_FALSE);				
			RETURN(POP_TO_RETURN);			
			RETURN(ALLOC_VAR);				
			RETURN(ALLOC_CONST_VAR);				
			RETURN(NEGATION_OP);
			RETURN(NEGATIVE_OP);				
			RETURN(POSITIVE_OP);				
			RETURN(SUM_OP);				
			RETURN(SUB_OP);				
			RETURN(MULT_OP);				
			RETURN(DIV_OP);				
			RETURN(MOD_OP);
			RETURN(POWER_OP);				
			RETURN(ASSIGN_OP);	
			RETURN(GET_MEMBER);				
			RETURN(SET_ALU_INCR);			
			RETURN(CMP_EQ);
			RETURN(CMP_NEQ);
			RETURN(CMP_L);
			RETURN(CMP_G);
			RETURN(CMP_LE);
			RETURN(CMP_GE);
			RETURN(CMP_AND);
			RETURN(CMP_OR);
			RETURN(GET_INDEX);
			RETURN(CALL_FUNCTION);
			RETURN(SET_LABEL);
			RETURN(JUMP);
			RETURN(JUMP_IF_TRUE);
			RETURN(JUMP_IF_FALSE);
			RETURN(PUSH_STACKFRAME);
			RETURN(POP_STACK_TOP);
#undef RETURN
			case RETURN:
				return STRING(RETURN);
			default:
				return "[[ unresolved symbol ]]";
			}
		}
	}
}