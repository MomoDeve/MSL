#include "objects.h"

#define RET_IF_MARKED if(state == GCstate::MARKED) return

namespace MSL
{
	namespace VM
	{
		BaseObject::BaseObject(Type type)
			: type(type) { }

		void BaseObject::MarkMembers()
		{
			state = GCstate::MARKED;
		}

		StringObject::StringObject(StringObject::InnerType value)
			: value(value), BaseObject(Type::STRING) { }

		std::string StringObject::ToString() const
		{
			return value;
		}

		std::string StringObject::GetExtraInfo() const
		{
			return "String";
		}

		size_t StringObject::GetSize() const
		{
			return value.capacity() < 16 ? 0 : value.capacity();
		}

		FloatObject::FloatObject(FloatObject::InnerType value)
			: value((value)), BaseObject(Type::FLOAT) { }

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
			return "Float";
		}

		size_t FloatObject::GetSize() const
		{
			return 0;
		}

		IntegerObject::IntegerObject(IntegerObject::InnerType value)
			: value(value), BaseObject(Type::INTEGER) { }

		std::string IntegerObject::ToString() const
		{
			return value.to_string();
		}

		std::string IntegerObject::GetExtraInfo() const
		{
			return "BigInteger";
		}

		size_t IntegerObject::GetSize() const
		{
			return value.size_bytes() - sizeof(value); // value counts twice because of size_bytes()
		}

		ClassObject::ClassObject(const ClassType* type)
			: type(type), BaseObject(Type::CLASS_OBJECT) { }

		std::string ClassObject::ToString() const
		{
			return type->namespaceName + '.' + type->name;
		}

		std::string ClassObject::GetExtraInfo() const
		{
			return " class instance";
		}

		void ClassObject::MarkMembers()
		{
			RET_IF_MARKED;
			BaseObject::MarkMembers();
			for (auto it = attributes.begin(); it != attributes.end(); it++)
			{
				AttributeObject* attr = it->second;
				attr->MarkMembers();
			}
		}

		size_t ClassObject::GetSize() const
		{
			return attributes.size() * sizeof(std::pair<std::string, AttributeObject*>);
		}
			
		NullObject::NullObject()
			: BaseObject(Type::NULLPTR) { }

		std::string NullObject::ToString() const
		{
			return "null";
		}

		std::string NullObject::GetExtraInfo() const
		{
			return std::string();
		}

		size_t NullObject::GetSize() const
		{
			return 0;
		}

		#undef TRUE // winapi
		TrueObject::TrueObject()
			: BaseObject(MSL::VM::Type::TRUE) { }

		std::string TrueObject::ToString() const
		{
			return "true";
		}

		std::string TrueObject::GetExtraInfo() const
		{
			return " Boolean";
		}

		size_t TrueObject::GetSize() const
		{
			return 0;
		}

		#undef FALSE // winapi
		FalseObject::FalseObject()
			: BaseObject(Type::FALSE) { }

		std::string FalseObject::ToString() const
		{
			return "false";
		}

		std::string FalseObject::GetExtraInfo() const
		{
			return " Boolean";
		}

		size_t FalseObject::GetSize() const
		{
			return 0;
		}

		NamespaceWrapper::NamespaceWrapper(const NamespaceType* type)
			: type(type), BaseObject(Type::NAMESPACE) { }


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

		void NamespaceWrapper::MarkMembers()
		{
			RET_IF_MARKED;
			BaseObject::MarkMembers();
			for (auto it = type->classes.begin(); it != type->classes.end(); it++)
			{
				ClassWrapper* wrapper = it->second.wrapper;
				wrapper->MarkMembers();
			}
		}

		size_t NamespaceWrapper::GetSize() const
		{
			return 0;
		}

		ClassWrapper::ClassWrapper(const ClassType* type)
			: type(type), BaseObject(Type::CLASS) { }

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

		void ClassWrapper::MarkMembers()
		{
			RET_IF_MARKED;
			BaseObject::MarkMembers();
			type->staticInstance->MarkMembers();
		}

		size_t ClassWrapper::GetSize() const
		{
			return 0;
		}

		UnknownObject::UnknownObject(const std::string* ref)
			: BaseObject(Type::UNKNOWN), ref(ref) { }

		std::string UnknownObject::ToString() const
		{
			return *ref;
		}

		std::string UnknownObject::GetExtraInfo() const
		{
			return "unresolved reference";
		}

		size_t UnknownObject::GetSize() const
		{
			return 0;
		}

		LocalObject::LocalObject(Local& ref, const std::string& name)
			: BaseObject(Type::LOCAL), ref(ref), name(name) { }

		std::string LocalObject::ToString() const
		{
			return name;
		}

		std::string LocalObject::GetExtraInfo() const
		{
			return ref.object->ToString();
		}

		void LocalObject::MarkMembers()
		{
			RET_IF_MARKED;
			BaseObject::MarkMembers();
			ref.object->MarkMembers();
		}

		size_t LocalObject::GetSize() const
		{
			return name.capacity() < 16 ? 0 : name.capacity();
		}

		AttributeObject::AttributeObject(const AttributeType* type)
			: BaseObject(Type::ATTRIBUTE), type(type) { }

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

		void AttributeObject::MarkMembers()
		{
			RET_IF_MARKED;
			BaseObject::MarkMembers();
			object->MarkMembers();
		}

		size_t AttributeObject::GetSize() const
		{
			return 0;
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

		ArrayObject::ArrayObject(size_t size)
			: BaseObject(Type::BASE), array(size)
		{

		}

		std::string ArrayObject::ToString() const
		{
			return "Array";
		}

		std::string ArrayObject::GetExtraInfo() const
		{
			return " array size: " + array.size();
		}

		void ArrayObject::MarkMembers()
		{
			RET_IF_MARKED;
			BaseObject::MarkMembers();
			for (Local& member : array)
			{
				member.object->MarkMembers();
			}
		}
		size_t ArrayObject::GetSize() const
		{
			return array.capacity() * sizeof(Local);
		}
	}
}