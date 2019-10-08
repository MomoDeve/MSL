#include "objects.h"

namespace MSL
{
	namespace VM
	{
		BaseObject::BaseObject(Type type)
			: type(type) { }

		StringObject::StringObject(StringObject::InnerType value)
			: value(value), BaseObject(Type::STRING) { }

		BaseObject* StringObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* StringObject::GetName() const
		{
			return nullptr;
		}

		std::string StringObject::ToString() const
		{
			return value;
		}

		std::string StringObject::GetExtraInfo() const
		{
			return "String";
		}

		FloatObject::FloatObject(FloatObject::InnerType value)
			: value((value)), BaseObject(Type::FLOAT) { }

		BaseObject* FloatObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* FloatObject::GetName() const
		{
			return nullptr;
		}

		std::string FloatObject::ToString() const
		{
			std::string out;
			out.resize(16, '\0');
			int written = std::snprintf(&out[0], out.size(), "%g", value);
			out.resize(written);
			return out;
		}

		std::string FloatObject::GetExtraInfo() const
		{
			return "Double";
		}

		IntegerObject::IntegerObject(IntegerObject::InnerType value)
			: value(value), BaseObject(Type::INTEGER) { }

		BaseObject* IntegerObject::GetMember(const std::string & memberName) const
		{
			return nullptr;
		}

		const std::string* IntegerObject::GetName() const
		{
			return nullptr;
		}

		std::string IntegerObject::ToString() const
		{
			return value.to_string();
		}

		std::string IntegerObject::GetExtraInfo() const
		{
			return "BigInteger";
		}

		ClassObject::ClassObject(const ClassType* type)
			: type(type), BaseObject(Type::CLASS_OBJECT) { }

		BaseObject* ClassObject::GetMember(const std::string& memberName) const
		{
			auto objectAttr = attributes.find(memberName);
			if (objectAttr != attributes.end())
			{
				return objectAttr->second.get();
			}
			auto staticAttr = type->staticInstance->attributes.find(memberName);
			if (staticAttr != type->staticInstance->attributes.end())
			{
				return staticAttr->second.get();
			}
			else
			{
				return nullptr;
			}
		}

		const std::string* ClassObject::GetName() const
		{
			return nullptr;
		}

		std::string ClassObject::ToString() const
		{
			return type->namespaceName + '.' + type->name;
		}

		std::string ClassObject::GetExtraInfo() const
		{
			return " class instance";
		}
			
		NullObject::NullObject()
			: BaseObject(Type::NULLPTR) { }

		BaseObject* NullObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* NullObject::GetName() const
		{
			return nullptr;
		}

		std::string NullObject::ToString() const
		{
			return "null";
		}

		std::string NullObject::GetExtraInfo() const
		{
			return std::string();
		}

		TrueObject::TrueObject()
			: BaseObject(Type::TRUE) { }

		BaseObject* TrueObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* TrueObject::GetName() const
		{
			return nullptr;
		}

		std::string TrueObject::ToString() const
		{
			return "true";
		}

		std::string TrueObject::GetExtraInfo() const
		{
			return " Boolean";
		}

		FalseObject::FalseObject()
			: BaseObject(Type::FALSE) { }

		BaseObject* FalseObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* FalseObject::GetName() const
		{
			return nullptr;
		}

		std::string FalseObject::ToString() const
		{
			return "false";
		}

		std::string FalseObject::GetExtraInfo() const
		{
			return " Boolean";
		}

		NamespaceWrapper::NamespaceWrapper(const NamespaceType* type)
			: type(type), BaseObject(Type::NAMESPACE) { }

		BaseObject* NamespaceWrapper::GetMember(const std::string& memberName) const
		{
			auto it = type->classes.find(memberName);
			if (it == type->classes.end()) return nullptr;
			else return it->second.wrapper;
		}

		const std::string* NamespaceWrapper::GetName() const
		{
			return &type->name;
		}

		std::string NamespaceWrapper::ToString() const
		{
			return type->name;
		}

		std::string NamespaceWrapper::GetExtraInfo() const
		{
			std::string info = "friends";
			for (const auto& ns : type->friendNamespaces)
			{
				info += ' ' + ns;
			}
			return info;
		}

		ClassWrapper::ClassWrapper(const ClassType* type)
			: type(type), BaseObject(Type::CLASS) { }

		BaseObject* ClassWrapper::GetMember(const std::string& memberName) const
		{
			return type->staticInstance->GetMember(memberName);
		}

		const std::string* ClassWrapper::GetName() const
		{
			return &type->name;
		}

		std::string ClassWrapper::ToString() const
		{
			return type->namespaceName + '.' + type->name;
		}

		std::string ClassWrapper::GetExtraInfo() const
		{
			std::string info;
			if (type->isInternal()) info += " internal";
			if (!type->isInternal()) info += " public";

			if (type->isSystem()) info += " system";
			if (type->isAbstract()) info += " abstract";
			if (type->isConst()) info += " const";
			if (type->isInterface()) info += " interface";
			if (type->isStatic()) info += " static";
			info += " class";
			return info;
		}

		UnknownObject::UnknownObject(const std::string* ref)
			: BaseObject(Type::UNKNOWN), ref(ref) { }

		BaseObject* UnknownObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* UnknownObject::GetName() const
		{
			return ref;
		}

		std::string UnknownObject::ToString() const
		{
			return *ref;
		}

		std::string UnknownObject::GetExtraInfo() const
		{
			return "unresolved reference";
		}

		LocalObject::LocalObject(Local& ref, const std::string& name)
			: BaseObject(Type::LOCAL), ref(ref), name(name) { }

		BaseObject* LocalObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* LocalObject::GetName() const
		{
			return &name;
		}

		std::string LocalObject::ToString() const
		{
			return name;
		}

		std::string LocalObject::GetExtraInfo() const
		{
			return ref.object->ToString();
		}

		AttributeObject::AttributeObject(const AttributeType* type)
			: BaseObject(Type::ATTRIBUTE), type(type) { }

		BaseObject* AttributeObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}
		const std::string* AttributeObject::GetName() const
		{
			return nullptr;
		}

		std::string AttributeObject::ToString() const
		{
			std::string out;
			if (type->isPublic()) out += "public ";
			if (type->isStatic()) out += "static ";
			if (type->isConst()) out += "const ";
			out += type->name + " attribute";
			return out;
		}

		std::string AttributeObject::GetExtraInfo() const
		{
			return " value: " + object->ToString();
		}

		std::string ToString(Type type)
		{
			switch (type)
			{
			case MSL::VM::Type::CLASS_OBJECT:
				return "Class Instance";
			case MSL::VM::Type::INTEGER:
				return "Integer";
			case MSL::VM::Type::FLOAT:
				return "Float";
			case MSL::VM::Type::STRING:
				return "String";
			case MSL::VM::Type::NULLPTR:
				return "Null";
			case MSL::VM::Type::TRUE:
				return "True";
			case MSL::VM::Type::FALSE:
				return "False";
			case MSL::VM::Type::NAMESPACE:
				return "Namespace";
			case MSL::VM::Type::CLASS:
				return "Class";
			case MSL::VM::Type::LOCAL:
				return "Local";
			case MSL::VM::Type::ATTRIBUTE:
				return "Attribute";
			case MSL::VM::Type::UNKNOWN:
				return "Unknown";
			default:
				return "ERROR";
			}
		}

		ArrayObject::ArrayObject(Type type, size_t size)
			: BaseObject(type), array(size)
		{

		}

		BaseObject* ArrayObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* ArrayObject::GetName() const
		{
			return nullptr;
		}

		std::string ArrayObject::ToString() const
		{
			return "Array";
		}

		std::string ArrayObject::GetExtraInfo() const
		{
			return " array size: " + array.size();
		}
}
}