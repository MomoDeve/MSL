#pragma once

#include "attributeType.h"
#include "methodType.h"

#include <unordered_map>
#include <string>

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
