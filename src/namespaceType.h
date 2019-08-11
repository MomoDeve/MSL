#pragma once

#include <unordered_map>

#include "classType.h"

namespace MSL
{
	namespace VM
	{
		struct NamespaceType
		{
			using HashTable = std::unordered_map<std::string, ClassType>;
			HashTable classes;
			std::string name;

			NamespaceType() = default;
			NamespaceType(NamespaceType&&) = default;
		};
	}
}