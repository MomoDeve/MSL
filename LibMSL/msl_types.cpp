#include "msl_types.h"

bool MSL::utils::AssertType(MSL::VM::VirtualMachine* vm, MSL::VM::BaseObject* object, MSL::VM::Type type)
{
	using ERROR = MSL::VM::VirtualMachine::ERROR;

	if (object->type == type) return true;

	vm->InvokeError(
		ERROR::INVALID_TYPE,
		"object with type " + MSL::VM::ToString(object->type) + " incopatible with expected type " + MSL::VM::ToString(type),
		object->ToString()
	);
	return false;
}

#undef TRUE
#undef FALSE

MSL::VM::BaseObject* MSL::utils::GetUnderlyingObject(MSL::VM::BaseObject* object)
{
	switch (object->type)
	{
	case MSL::VM::Type::CLASS_OBJECT:
	case MSL::VM::Type::INTEGER:
	case MSL::VM::Type::FLOAT:
	case MSL::VM::Type::STRING:
	case MSL::VM::Type::NULLPTR:
	case MSL::VM::Type::TRUE:
	case MSL::VM::Type::FALSE:
	case MSL::VM::Type::NAMESPACE:
	case MSL::VM::Type::CLASS:
	case MSL::VM::Type::UNKNOWN:
		return object;
	case MSL::VM::Type::LOCAL:
		return static_cast<MSL::VM::LocalObject*>(object)->ref.object;
	case MSL::VM::Type::ATTRIBUTE:
		return static_cast<MSL::VM::AttributeObject*>(object)->object;
	default:
		return nullptr; // hits only if error occured
	}
}

const std::string* MSL::utils::GetObjectName(const MSL::VM::BaseObject* object)
{
	switch (object->type)
	{
	case MSL::VM::Type::UNKNOWN:
		return static_cast<const MSL::VM::UnknownObject*>(object)->ref;
	case MSL::VM::Type::CLASS:
		return &static_cast<const MSL::VM::ClassWrapper*>(object)->type->name;
	case MSL::VM::Type::NAMESPACE:
		return &static_cast<const MSL::VM::NamespaceWrapper*>(object)->type->name;
	case MSL::VM::Type::ATTRIBUTE:
		return &static_cast<const MSL::VM::AttributeObject*>(object)->type->name;
	case MSL::VM::Type::LOCAL:
		return &static_cast<const MSL::VM::LocalObject*>(object)->name;
	case MSL::VM::Type::CLASS_OBJECT:
		return &static_cast<const MSL::VM::ClassObject*>(object)->type->name;
	default:
		return nullptr;
	}
}