#pragma once

#include <vector>
#include <memory>

#include "namespaceType.h"
#include "bigInteger.h"

namespace MSL
{
	namespace VM
	{
		struct Local;
		
		enum class Type : uint8_t
		{
			CLASS_OBJECT,
			INTEGER,
			FLOAT,
			STRING,
			NULLPTR,
			TRUE,
			FALSE,
			NAMESPACE,
			CLASS,
			LOCAL,
			ATTRIBUTE,
			UNKNOWN,
			BASE,
		};

		std::string ToString(Type type);

		struct BaseObject
		{
			Type type = Type::BASE;

			BaseObject(Type type);
			virtual BaseObject* GetMember(const std::string& memberName) const = 0;
			virtual const std::string* GetName() const = 0;
			virtual std::string ToString() const  = 0;
			virtual std::string GetExtraInfo() const = 0;
			virtual ~BaseObject() = default;
		};

		struct NullObject : BaseObject
		{
			NullObject();

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct TrueObject : BaseObject
		{
			TrueObject();

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct FalseObject : BaseObject
		{
			FalseObject();

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct ClassWrapper : BaseObject
		{
			const ClassType* type;

			ClassWrapper(const ClassType* type);

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const override;
			virtual std::string GetExtraInfo() const override;
		};

		struct NamespaceWrapper : BaseObject
		{
			const NamespaceType* type;

			NamespaceWrapper(const NamespaceType* type);

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const override;
			virtual std::string GetExtraInfo() const override;
		};

		struct AttributeObject : BaseObject
		{
			const AttributeType* type;
			BaseObject* object = nullptr;

			AttributeObject(const AttributeType* ref);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;

		};

		struct ClassObject : BaseObject
		{
			using AttributeTable = std::unordered_map<std::string, std::unique_ptr<AttributeObject>>;
			AttributeTable attributes;
			const ClassType* type;

			ClassObject(const ClassType* type);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct IntegerObject : BaseObject
		{
			using InnerType = momo::BigInteger;
			InnerType value;

			IntegerObject(InnerType value);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct FloatObject : BaseObject
		{
			using InnerType = double;
			InnerType value;
			
			FloatObject(InnerType value);
			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct StringObject : BaseObject
		{
			using InnerType = std::string;
			InnerType value;

			StringObject(InnerType value);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct Local
		{
			BaseObject* object = nullptr;
			bool isConst = false;
		};

		struct LocalObject : BaseObject
		{
			Local& ref;
			std::string name;

			LocalObject(Local& ref, const std::string& name);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
		};

		struct UnknownObject : BaseObject
		{
			const std::string* ref;

			UnknownObject(const std::string* ref);

			virtual BaseObject * GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;

		};

		struct ArrayObject : BaseObject
		{
			using InnerType = std::vector<Local>;
			InnerType array;

			ArrayObject(Type type, size_t size);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const override;
			virtual std::string GetExtraInfo() const override;
		};
	}
}
