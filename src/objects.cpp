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

		const std::string& StringObject::GetName() const
		{
			return emptyString;
		}

		FloatObject::FloatObject(const std::string& value)
			: value(std::stod(value)), BaseObject(Type::FLOAT) { }

		BaseObject* FloatObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string& FloatObject::GetName() const
		{
			return emptyString;
		}

		IntegerObject::IntegerObject(const std::string& value)
			: value(value), BaseObject(Type::INTEGER) { }

		BaseObject* IntegerObject::GetMember(const std::string & memberName) const
		{
			return nullptr;
		}

		const std::string& IntegerObject::GetName() const
		{
			return emptyString;
		}

		ClassObject::ClassObject(const ClassType* type)
			: type(type), BaseObject(Type::CLASS_OBJECT) { }

		BaseObject* ClassObject::GetMember(const std::string& memberName) const
		{
			auto it = type->attributes.find(memberName);
			if (it == type->attributes.end()) return nullptr;
			if (it->second.modifiers & AttributeType::Modifiers::STATIC)
			{
				if (this == type->staticInstance)
				{
					return attributes[it->second.offset];
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				if (this == type->staticInstance)
				{
					return nullptr;
				}
				else
				{
					return attributes[it->second.offset];
				}
			}
		}

		const std::string& ClassObject::GetName() const
		{
			return emptyString;
		}
			
		NullObject::NullObject()
			: BaseObject(Type::NULLPTR) { }

		BaseObject* NullObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string& NullObject::GetName() const
		{
			return emptyString;
		}

		TrueObject::TrueObject()
			: BaseObject(Type::TRUE) { }

		BaseObject* TrueObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string& TrueObject::GetName() const
		{
			return emptyString;
		}

		FalseObject::FalseObject()
			: BaseObject(Type::FALSE) { }

		BaseObject* FalseObject::GetMember(const std::string& memberName) const
		{
			return nullptr;
		}

		const std::string& FalseObject::GetName() const
		{
			return emptyString;
		}

		NamespaceWrapper::NamespaceWrapper(const NamespaceType* type)
			: type(type), BaseObject(Type::NAMESPACE) { }

		BaseObject* NamespaceWrapper::GetMember(const std::string& memberName) const
		{
			auto it = type->classes.find(memberName);
			if (it == type->classes.end()) return nullptr;
			else return it->second.wrapper;
		}

		const std::string& NamespaceWrapper::GetName() const
		{
			return type->name;
		}

		ClassWrapper::ClassWrapper(const ClassType* type)
			: type(type), BaseObject(Type::CLASS) { }

		BaseObject* ClassWrapper::GetMember(const std::string& memberName) const
		{
			return type->staticInstance->GetMember(memberName);
		}
		const std::string& ClassWrapper::GetName() const
		{
			return type->name;
		}

		UnknownObject::UnknownObject(const std::string& ref)
			: BaseObject(Type::UNKNOWN), ref(ref) { }

		BaseObject* UnknownObject::GetMember(const std::string & memberName) const
		{
			return nullptr;
		}
		const std::string& UnknownObject::GetName() const
		{
			return ref;
		}
}
}