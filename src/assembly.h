#pragma once

#include <memory>

#include "namespace.h"

struct EntryPoint
{
	std::string namespaceName;
	std::string className;
	std::string methodName;
};

class Assembly
{
	using NamespaceTable = std::unordered_map<std::string, size_t>;
	using NamespaceArray = std::vector<Namespace>;

	NamespaceTable table;
	NamespaceArray namespaces;
public:
	Assembly();
	Assembly(const Assembly& assembly) = delete;
	Assembly(Assembly&& assembly);
	void InsertNamespace(const std::string& name, Namespace&& _namespace);
	
	const NamespaceArray& GetNamespaces() const;
	bool ContainsNamespace(const std::string& namespaceName) const;
};