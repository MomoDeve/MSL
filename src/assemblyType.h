#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

#include "namespaceType.h"

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