#include "classType.h"

bool MSL::VM::ClassType::isStatic() const
{
	return modifiers & Modifiers::STATIC;
}

bool MSL::VM::ClassType::isAbstract() const
{
	return modifiers & Modifiers::ABSTRACT;
}

#undef CONST // winapi
bool MSL::VM::ClassType::isConst() const
{
	return modifiers & Modifiers::CONST;
}

#undef INTERFACE // winapi
bool MSL::VM::ClassType::isInterface() const
{
	return modifiers & Modifiers::INTERFACE;
}

bool MSL::VM::ClassType::isInternal() const
{
	return modifiers & Modifiers::INTERNAL;
}

bool MSL::VM::ClassType::hasStaticConstructor() const
{
	return modifiers & Modifiers::STATIC_CONSTRUCTOR;
}

bool MSL::VM::ClassType::isSystem() const
{
	return modifiers & Modifiers::SYSTEM;
}