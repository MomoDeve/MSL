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
			using AttributeHashTable = std::unordered_map<std::string, AttributeType>;
			using MethodHashTable = std::unordered_map<std::string, MethodType>;
			AttributeHashTable attributes;
			MethodHashTable methods;
			ClassObject* staticInstance = nullptr;
			BaseObject* wrapper = nullptr;
			std::string namespaceName;

			std::string name;
			uint8_t modifiers = 0;
		};
	}
}
