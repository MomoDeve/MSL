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
                ABSTRACT = 2,
                PRIVATE = 4,
                HAS_STATIC_CONSTRUCTOR = 8,
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
			bool isAbstract() const;
			bool IsPrivate() const;
			bool HasStaticConstructor() const;
			bool isSystem() const;

			std::string name;
			uint8_t modifiers = 0;
		};
	}
}
