#include "objects.h"

namespace MSL
{
	namespace VM
	{
		BaseObject::BaseObject(Type type)
			: type(type) { }

		StringObject::StringObject(const std::string& value)
			: value(value), BaseObject(Type::STRING) { }

		BaseObject* StringObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* StringObject::GetName() const
		{
			return nullptr;
		}

		FloatObject::FloatObject(const std::string& value)
			: value(std::stod(value)), BaseObject(Type::FLOAT) { }

		BaseObject* FloatObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* FloatObject::GetName() const
		{
			return nullptr;
		}

		IntegerObject::IntegerObject(const std::string& value)
			: value(value), BaseObject(Type::INTEGER) { }

		BaseObject* IntegerObject::GetMember(const std::string & memberName) const
		{
			return nullptr;
		}

		const std::string* IntegerObject::GetName() const
		{
			return nullptr;
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
		LocalObject::LocalObject(Local& ref, const std::string& nameRef)
			: BaseObject(Type::LOCAL), ref(ref), nameRef(nameRef) { }

		BaseObject* LocalObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string* LocalObject::GetName() const
		{
			return &nameRef;
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
}
}