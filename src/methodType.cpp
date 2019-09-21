#include "methodType.h"

bool MSL::VM::MethodType::isPublic() const
{
	return modifiers & Modifiers::PUBLIC;
}

bool MSL::VM::MethodType::isAbstract() const
{
	return modifiers & Modifiers::ABSTRACT;
}

bool MSL::VM::MethodType::isStatic() const
{
	return modifiers & Modifiers::STATIC;
}

bool MSL::VM::MethodType::isConstructor() const
{
	return modifiers & Modifiers::CONSTRUCTOR;
}

bool MSL::VM::MethodType::isStaticConstructor() const
{
	return modifiers & Modifiers::STATIC_CONSTRUCTOR;
}

bool MSL::VM::MethodType::isEntryPoint() const
{
	return modifiers & Modifiers::ENTRY_POINT;
}