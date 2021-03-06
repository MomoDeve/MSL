#pragma once

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

		enum class GCstate : uint8_t
		{
			FREE = 0,
			UNMARKED,
			MARKED,
		};

		std::string ToString(Type type);

		struct BaseObject
		{
			Type type = Type::BASE;
			GCstate state = GCstate::UNMARKED;

			BaseObject(Type type);
			virtual std::string ToString() const  = 0;
			virtual std::string GetExtraInfo() const = 0;
			virtual void MarkMembers();
			virtual size_t GetSize() const = 0;
			virtual ~BaseObject() = default;
		};

		struct NullObject : BaseObject
		{
			NullObject();

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual size_t GetSize() const override;
		};

		struct TrueObject : BaseObject
		{
			TrueObject();

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual size_t GetSize() const override;
		};

		struct FalseObject : BaseObject
		{
			FalseObject();

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual size_t GetSize() const override;
		};

		struct ClassWrapper : BaseObject
		{
			const ClassType* typeInstance;

			ClassWrapper(const ClassType* type);

			virtual std::string ToString() const override;
			virtual std::string GetExtraInfo() const override;
			virtual void MarkMembers() override;
			virtual size_t GetSize() const override;
		};

		struct NamespaceWrapper : BaseObject
		{
			const NamespaceType* type;

			NamespaceWrapper(const NamespaceType* type);

			virtual std::string ToString() const override;
			virtual std::string GetExtraInfo() const override;
			virtual void MarkMembers() override;
			virtual size_t GetSize() const override;
		};

		struct AttributeObject : BaseObject
		{
			const AttributeType* type;
			BaseObject* object = nullptr;

			AttributeObject(const AttributeType* ref);

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual void MarkMembers() override;
			virtual size_t GetSize() const override;
		};

		struct ClassObject : BaseObject
		{
			using AttributeTable = std::unordered_map<std::string, AttributeObject*>;
			AttributeTable attributes;
			const ClassType* typeInstance;

			ClassObject(const ClassType* type);

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual void MarkMembers() override;
			virtual size_t GetSize() const override;
		};

		struct IntegerObject : BaseObject
		{
			using InnerType = momo::BigInteger;
			InnerType value;

			IntegerObject(InnerType value);

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual size_t GetSize() const override;
		};

		struct FloatObject : BaseObject
		{
			using InnerType = double;
			InnerType value;
			
			FloatObject(InnerType value);

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual size_t GetSize() const override;
		};

		struct StringObject : BaseObject
		{
			using InnerType = std::string;
			InnerType value;

			StringObject(InnerType value);

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual size_t GetSize() const override;
		};

		struct Local
		{
			BaseObject* object = nullptr;
			bool isConst = false;
			bool isElement = false;
		};

		struct LocalObject : BaseObject
		{
			Local& ref;
			std::string name;

			LocalObject(Local& ref, const std::string& name);

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual void MarkMembers() override;
			virtual size_t GetSize() const override;
		};

		struct UnknownObject : BaseObject
		{
			const std::string* ref;

			UnknownObject(const std::string* ref);

			virtual std::string ToString() const  override;
			virtual std::string GetExtraInfo() const override;
			virtual size_t GetSize() const override;
		};

		struct ArrayObject : BaseObject
		{
			using InnerType = std::vector<Local>;
			InnerType array;

			ArrayObject(size_t size);

			virtual std::string ToString() const override;
			virtual std::string GetExtraInfo() const override;
			virtual void MarkMembers() override;
			virtual size_t GetSize() const override;
		};
	}
}
