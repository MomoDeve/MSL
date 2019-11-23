#include "math.h"

using namespace MSL::VM;
using namespace MSL::utils;

void FloatSqrt(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return;
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::sqrt(object->value);
}

void FloatAbs(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return;
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::abs(object->value);
}

void FloatSin(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return;
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::sin(object->value);
}

void FloatCos(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::cos(object->value);
}

void FloatTan(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return;
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::tan(object->value);
}

void FloatExp(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::exp(object->value);
}

void FloatAtan(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::atan(object->value);
}

void FloatAcos(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::acos(object->value);
}

void FloatAsin(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::asin(object->value);
}

void FloatLog10(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::log10(object->value);
}

void FloatLog2(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::log2(object->value);
}

void FloatLog(VM_ATTRIBUTES)
{
	 if(!AssertType(stack->back(), MSL::VM::Type::FLOAT, errors, config)) return; 
	 auto* object = static_cast<MSL::VM::FloatObject*>(stack->back());
	 object->value = std::log(object->value);
}
