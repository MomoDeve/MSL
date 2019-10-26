#pragma once

#include <unordered_map>
#include <unordered_set>

#include "classType.h"

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