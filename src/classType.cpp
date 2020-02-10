#include "classType.h"

bool MSL::VM::ClassType::isStatic() const
{
	return modifiers & Modifiers::STATIC;
}

bool MSL::VM::ClassType::isAbstract() const
{
	return modifiers & Modifiers::ABSTRACT;
}

bool MSL::VM::ClassType::IsPrivate() const
{
	return modifiers & Modifiers::PRIVATE;
}

bool MSL::VM::ClassType::HasStaticConstructor() const
{
	return modifiers & Modifiers::HAS_STATIC_CONSTRUCTOR;
}

bool MSL::VM::ClassType::isSystem() const
{
	return modifiers & Modifiers::SYSTEM;
}