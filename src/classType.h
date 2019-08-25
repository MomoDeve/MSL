#pragma once

#include <unordered_map>

#include "attributeType.h"
#include "methodType.h"

namespace MSL
{
	namespace VM
	{
		struct ClassObject;
		struct BaseObject;

		struct ClassType
		{
			enum Modifiers : uint8_t
			{
				STATIC = 1,
				INTERFACE = 2,
				ABSTRACT = 4,
				CONST = 8,
				INTERNAL = 16,
				SYSTEM = 128
			};
			using AttributeHashTable = std::unordered_map<std::string, AttributeType>;
			using MethodHashTable = std::unordered_map<std::string, MethodType>;
			AttributeHashTable staticAttributes;
			AttributeHashTable objectAttributes;
			MethodHashTable methods;
			ClassObject* staticInstance = nullptr;
			BaseObject* wrapper = nullptr;
			std::string namespaceName;

			std::string name;
			uint8_t modifiers = 0;
		};
	}
}
