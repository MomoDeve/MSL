#include "console.h"

using namespace MSL::utils;

void Print(VM_ATTRIBUTES)
{
	std::ostream& out = *config->streams.out;
	BaseObject* object = GetUnderlyingObject(stack->back());
	
	if (config->streams.out == nullptr) return;

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
		stack->push_back(attr->object);
		Print(stack, assembly, errors, config, gc);
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

void PrintLine(VM_ATTRIBUTES)
{
	Print(stack, assembly, errors, config, gc);
	if (config->streams.out != nullptr) *config->streams.out << std::endl;
}

void Read(VM_ATTRIBUTES)
{
	StringObject::InnerType str;
	if (config->streams.in != nullptr)
		*config->streams.in >> str;

	stack->push_back(AllocString(gc, str));
}

void ReadInt(VM_ATTRIBUTES)
{
	std::string str = "0";
	if (config->streams.in != nullptr)
		*config->streams.in >> str;

	stack->push_back(AllocInteger(gc, str));
}

void ReadFloat(VM_ATTRIBUTES)
{
	std::string str = "0.0";
	if (config->streams.in != nullptr)
		*config->streams.in >> str;

	stack->push_back(AllocFloat(gc, str));
}

void ReadLine(VM_ATTRIBUTES)
{
	if (!AssertType(stack->back(), Type::STRING, errors, config)) return;

	StringObject::InnerType str;
	if (config->streams.in != nullptr)
		std::getline(*config->streams.in, str);

	stack->push_back(AllocString(gc, str));
}

void ReadBool(VM_ATTRIBUTES)
{
	std::string str;
	if (config->streams.in != nullptr)
		 *config->streams.in >> str;

	if (str == "1" || str == "True" || str == "true")
	{
		stack->push_back(AllocTrue(gc));
	}
	else
	{
		stack->push_back(AllocFalse(gc));
	}
}
