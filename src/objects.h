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
			UNKNOWN
		};

		std::string ToString(Type type);

		struct BaseObject
		{
			Type type;

			BaseObject(Type type);
			virtual BaseObject* GetMember(const std::string& memberName) const = 0;
			virtual const std::string* GetName() const = 0;
			virtual std::string ToString() const  = 0;
			virtual ~BaseObject() = default;
		};

		struct NullObject : BaseObject
		{
			NullObject();

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
		};

		struct TrueObject : BaseObject
		{
			TrueObject();

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
		};

		struct FalseObject : BaseObject
		{
			FalseObject();

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
		};

		struct ClassWrapper : BaseObject
		{
			const ClassType* type;

			ClassWrapper(const ClassType* type);

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;

			// ������������ ����� BaseObject
			virtual std::string ToString() const  override;
		};

		struct NamespaceWrapper : BaseObject
		{
			const NamespaceType* type;

			NamespaceWrapper(const NamespaceType* type);

			virtual BaseObject* GetMember(const std::string & memberName) const override;
			virtual const std::string* GetName() const override;

			// ������������ ����� BaseObject
			virtual std::string ToString() const  override;
		};

		struct AttributeObject : BaseObject
		{
			const AttributeType* type;
			BaseObject* object = nullptr;

			AttributeObject(const AttributeType* ref);

			virtual BaseObject* GetMember(const std::string& memberName) const override;

			virtual const std::string* GetName() const override;


			// ������������ ����� BaseObject
			virtual std::string ToString() const  override;

		};

		struct ClassObject : BaseObject
		{
			using AttributeTable = std::unordered_map<std::string, std::unique_ptr<AttributeObject>>;
			AttributeTable attributes;
			const ClassType* type;

			ClassObject(const ClassType* type);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;

			// ������������ ����� BaseObject
			virtual std::string ToString() const  override;
		};

		struct IntegerObject : BaseObject
		{
			using InnerType = momo::BigInteger;
			InnerType value;

			IntegerObject(InnerType value);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
		};

		struct FloatObject : BaseObject
		{
			using InnerType = double;
			InnerType value;
			
			FloatObject(InnerType value);
			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;

			virtual std::string ToString() const  override;
		};

		struct StringObject : BaseObject
		{
			using InnerType = std::string;
			InnerType value;

			StringObject(InnerType value);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;
		};

		struct LocalObject : BaseObject
		{
			Local& ref;
			const std::string& nameRef;

			LocalObject(Local& ref, const std::string& nameRef);

			virtual BaseObject* GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;

			// ������������ ����� BaseObject
			virtual std::string ToString() const  override;
		};

		struct UnknownObject : BaseObject
		{
			const std::string* ref;

			UnknownObject(const std::string* ref);

			virtual BaseObject * GetMember(const std::string& memberName) const override;
			virtual const std::string* GetName() const override;
			virtual std::string ToString() const  override;

		};
	}
}
