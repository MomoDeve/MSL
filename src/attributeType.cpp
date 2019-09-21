#include "attributeType.h"

bool MSL::VM::AttributeType::isStatic() const
{
	return modifiers & Modifiers::STATIC;
}

bool MSL::VM::AttributeType::isConst() const
{
	return modifiers & Modifiers::CONST;
}

bool MSL::VM::AttributeType::isPublic() const
{
	return modifiers & Modifiers::PUBLIC;
}
