#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>

#include "bigInteger.h"

struct ERROR
{
	enum value : uint32_t
	{
		CALLSTACK_EMPTY = 1,
		INVALID_CALL_ARGUMENT = 1 << 1,
		TERMINATE_ON_LAUNCH = 1 << 2,
		INVALID_OPCODE = 1 << 3,
		INVALID_STACKFRAME_OFFSET = 1 << 4,
		OBJECTSTACK_CORRUPTION = 1 << 5,
		INVALID_METHOD_SIGNATURE = 1 << 6,
		OBJECTSTACK_EMPTY = 1 << 7,
		INVALID_HASH_VALUE = 1 << 8,
		OBJECT_NOT_FOUND = 1 << 9,
		MEMBER_NOT_FOUND = 1 << 10,
		INVALID_STACKOBJECT = 1 << 11,
		STACKOVERFLOW = 1 << 12,
		PRIVATE_MEMBER_ACCESS = 1 << 13,
		CALLSTACK_CORRUPTION = 1 << 14,
		CONST_MEMBER_MODIFICATION = 1 << 15,
		ABSTRACT_MEMBER_CALL = 1 << 16,
		INVALID_METHOD_CALL = 1 << 17,
		OUT_OF_MEMORY = 1 << 18,
		DLL_NOT_FOUND = 1 << 19,
	};
};

namespace MSL
{
	namespace VM
	{
		constexpr uint64_t KB = 1024;
		constexpr uint64_t MB = KB * 1024;
		constexpr uint64_t GB = MB * 1024;

		struct Configuration
		{
			struct
			{
				std::istream* in = &std::cin;
				std::ostream* out = &std::cout;
				std::ostream* error = &std::cerr;
			} streams;
			struct
			{
				uint64_t initCapacity = 1;
				uint64_t minMemory = 4 * MB;
				uint64_t maxMemory = 1 * GB;
				std::ostream* log = nullptr;
				bool allowCollect = true;
			} GC;
			struct
			{
				bool varifyBytecode = true;
				bool allowAssemblyMerge = true;
			} compilation;
			struct
			{
				size_t recursionLimit = 2000;
				bool checkExitCode = true;
				bool useUnicode = true;
				bool safeMode = false;
			} execution;
		};
	}
}

namespace MSL
{
	namespace VM
	{
		struct MethodType
		{
			enum Modifiers
			{
				ABSTRACT = 1,
				STATIC = 2,
				PUBLIC = 4,
				CONSTRUCTOR = 8,
				STATIC_CONSTRUCTOR = 16,
				ENTRY_POINT = 128,
			};

			using StringArray = std::vector<std::string>;
			using ByteArray = std::vector<uint8_t>;
			using LabelOffsetArray = std::vector<size_t>;
			StringArray parameters;
			StringArray dependencies;
			LabelOffsetArray labels;
			ByteArray body;

			std::string name;
			uint8_t modifiers = 0;

			MethodType() = default;
			MethodType(MethodType&&) = default;

			bool isPublic() const;
			bool isAbstract() const;
			bool isStatic() const;
			bool isConstructor() const;
			bool isStaticConstructor() const;
			bool isEntryPoint() const;
		};
	}
}

namespace MSL
{
	namespace VM
	{
		struct BaseObject;

		struct AttributeType
		{
			enum Modifiers
			{
				STATIC = 1,
				CONST = 2,
				PUBLIC = 4,
			};
			std::string name;
			uint8_t modifiers = 0;

			bool isStatic() const;
			bool isConst() const;
			bool isPublic() const;

			AttributeType() = default;
			AttributeType(AttributeType&&) = default;
		};
	}
}

namespace MSL
{
	namespace VM
	{
		struct ClassObject;
		struct BaseObject;
		struct ClassWrapper;

		struct ClassType
		{
			enum Modifiers : uint8_t
			{
				STATIC = 1,
				INTERFACE = 2,
				ABSTRACT = 4,
				CONST = 8,
				INTERNAL = 16,
				STATIC_CONSTRUCTOR = 32,
				SYSTEM = 128
			};
			using AttributeHashTable = std::unordered_map<std::string, AttributeType>;
			using MethodHashTable = std::unordered_map<std::string, MethodType>;
			AttributeHashTable staticAttributes;
			AttributeHashTable objectAttributes;
			MethodHashTable methods;
			ClassObject* staticInstance = nullptr;
			ClassWrapper* wrapper = nullptr;
			mutable bool staticConstructorCalled = false;
			std::string namespaceName;

			bool isStatic() const;
			bool isInterface() const;
			bool isAbstract() const;
			bool isConst() const;
			bool isInternal() const;
			bool hasStaticConstructor() const;
			bool isSystem() const;

			std::string name;
			uint8_t modifiers = 0;
		};
	}
}

namespace MSL
{
	namespace VM
	{
		struct NamespaceWrapper;

		struct NamespaceType
		{
			using HashTable = std::unordered_map<std::string, ClassType>;
			using HashSet = std::unordered_set<std::string>;
			HashSet friendNamespaces;
			HashTable classes;
			std::string name;
			NamespaceWrapper* wrapper = nullptr;

			NamespaceType() = default;
			NamespaceType(NamespaceType&&) = default;
		};
	}
}

namespace MSL
{
	namespace VM
	{
		struct AssemblyType
		{
			using HashTable = std::unordered_map<std::string, NamespaceType>;
			HashTable namespaces;

			AssemblyType() = default;
			AssemblyType(AssemblyType&&) = default;
		};
	}
}

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
			virtual std::string ToString() const = 0;
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
			const ClassType* type;

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
			const ClassType* type;

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

namespace MSL
{
	namespace utils
	{
		void InvokeError(std::ostream* out, const std::string& message, uint32_t* errors, ERROR::value error);
		bool AssertType(MSL::VM::BaseObject* object, MSL::VM::Type type, uint32_t* errors, MSL::VM::Configuration* config);
		VM::BaseObject* GetUnderlyingObject(VM::BaseObject* object);
		const std::string* GetObjectName(const VM::BaseObject* object);
	}
}

#include "garbageCollector.h"

namespace MSL
{
	namespace VM
	{
		ClassWrapper* AllocClassWrapper(GarbageCollector* GC, const ClassType* _class);
		ClassObject* AllocClassObject(GarbageCollector* GC, const ClassType* _class);
		NamespaceWrapper* AllocNamespaceWrapper(GarbageCollector* GC, const NamespaceType* _namespace);
		LocalObject* AllocLocal(GarbageCollector* GC, const std::string& localName, Local& local);
		UnknownObject* AllocUnknown(GarbageCollector* GC, const std::string* value);
		NullObject* AllocNull(GarbageCollector* GC);
		TrueObject* AllocTrue(GarbageCollector* GC);
		FalseObject* AllocFalse(GarbageCollector* GC);
		ArrayObject* AllocArray(GarbageCollector* GC, size_t size);
		StringObject* AllocString(GarbageCollector* GC, const std::string& value);
		IntegerObject* AllocInteger(GarbageCollector* GC, const std::string& value);
		FloatObject* AllocFloat(GarbageCollector* GC, const std::string& value);
	}
}
