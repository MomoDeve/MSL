#include "math.h"

using namespace MSL::VM;
using namespace MSL::utils;

void FloatSqrt(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::sqrt(object->value);
}

void FloatAbs(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::abs(object->value);
}

void FloatSin(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::sin(object->value);
}

void FloatCos(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::cos(object->value);
}

void FloatTan(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::tan(object->value);
}

void FloatExp(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::exp(object->value);
}

void FloatAtan(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::atan(object->value);
}

void FloatAcos(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::acos(object->value);
}

void FloatAsin(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::asin(object->value);
}

void FloatLog10(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::log10(object->value);
}

void FloatLog2(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::log2(object->value);
}

void FloatLog(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if(!AssertType(vm, top, MSL::VM::Type::FLOAT)) return; 
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::log(object->value);
}
