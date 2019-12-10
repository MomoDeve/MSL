#include "console.h"

using namespace MSL::utils;

void Print(PARAMS)
{
	BaseObject* object = GetUnderlyingObject(vm->GetObjectStack().back());
	
	if (vm->GetConfig().streams.out == nullptr) return;
	std::ostream& out = *vm->GetConfig().streams.out;

	switch (object->type)
	{
	case Type::NAMESPACE:
	{
		out << "namespace " << *GetObjectName(object);
		break;
	}
	case Type::CLASS:
	{
		ClassWrapper* c = static_cast<ClassWrapper*>(object);
		out << "class " << c->type->namespaceName + '.' + c->type->name;
		break;
	}
	case Type::ATTRIBUTE:
	{
		AttributeObject* attr = static_cast<AttributeObject*>(object);
		vm->GetObjectStack().push_back(attr->object);
		Print(vm);
		return; // no PrintLine check, because it will happen inside recursion call
	}
	case Type::CLASS_OBJECT:
	{
		ClassObject* classObject = static_cast<ClassObject*>(object);
		out << classObject->type->namespaceName + '.' + classObject->type->name << " instance";
		break;
	}
	default:
	{
		out << object->ToString();
		break;
	}
	}
}

void PrintLine(PARAMS)
{
	Print(vm);
	if (vm->GetConfig().streams.out != nullptr) *vm->GetConfig().streams.out << std::endl;
}

void Read(PARAMS)
{
	StringObject::InnerType str;
	if (vm->GetConfig().streams.in != nullptr)
		*vm->GetConfig().streams.in >> str;

	vm->GetObjectStack().push_back(AllocString(vm->GetGC(), str));
}

void ReadInt(PARAMS)
{
	std::string str = "0";
	if (vm->GetConfig().streams.in != nullptr)
		*vm->GetConfig().streams.in >> str;

	vm->GetObjectStack().push_back(AllocInteger(vm->GetGC(), str));
}

void ReadFloat(PARAMS)
{
	std::string str = "0.0";
	if (vm->GetConfig().streams.in != nullptr)
		*vm->GetConfig().streams.in >> str;

	vm->GetObjectStack().push_back(AllocFloat(vm->GetGC(), str));
}

void ReadLine(PARAMS)
{
	StringObject::InnerType str;
	if (vm->GetConfig().streams.in != nullptr)
		std::getline(*vm->GetConfig().streams.in, str);

	vm->GetObjectStack().push_back(AllocString(vm->GetGC(), str));
}

void ReadBool(PARAMS)
{
	std::string str;
	if (vm->GetConfig().streams.in != nullptr)
		 *vm->GetConfig().streams.in >> str;

	if (str == "1" || str == "True" || str == "true")
	{
		vm->GetObjectStack().push_back(AllocTrue(vm->GetGC()));
	}
	else
	{
		vm->GetObjectStack().push_back(AllocFalse(vm->GetGC()));
	}
}
