#include "objects.h"

namespace MSL
{
	namespace VM
	{
		BaseObject::BaseObject(Type type)
			: type(type) { }

		String::String(const std::string& value)
			: value(value), BaseObject(Type::STRING) { }

		Float::Float(const std::string& value)
			: value(std::stod(value)), BaseObject(Type::FLOAT) { }

		Integer::Integer(const std::string& value)
			: value(value), BaseObject(Type::INTEGER) { }

		Class::Class(const ClassType& type)
			: type(type), BaseObject(Type::CLASS) { }

		Function::Function(const MethodType& type)
			: type(type), BaseObject(Type::FUNCTION) { }
	}
}