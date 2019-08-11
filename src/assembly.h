#pragma once

#include <memory>

#include "namespace.h"

namespace MSL
{
	namespace compiler
	{
		/*
		Assembly class is used to store all program namespaces parsed from source code
		Assembly is a root of object hierarchy in MSL language
		*/
		class Assembly
		{
			using NamespaceTable = std::unordered_map<std::string, size_t>;
			using NamespaceArray = std::vector<Namespace>;
			/*
			string hash-table storing namespace name as key and index in NamespaceArray as value
			*/
			NamespaceTable table;
			/*
			array of all namespaces (see Namespace discription)
			*/
			NamespaceArray namespaces;
		public:
			/*
			creates empty assembly
			*/
			Assembly() = default;
			/*
			assembly cannot be copied, consider using move instead
			*/
			Assembly(const Assembly& assembly) = delete;
			/*
			moving all contents of assembly
			*/
			Assembly(Assembly&& assembly);
			/*
			moves namespace to NamespaceArray and adds its name to hash-table
			if namespace name already existed in table, it will be overwriten in hash-table, but not deleted from NamespaceArray
			*/
			void InsertNamespace(const std::string& name, Namespace&& _namespace);
			/*
			returns constant reference to all namespaces in assembly
			*/
			const NamespaceArray& GetNamespaces() const;
			/*
			checks if assembly already contains namespace with given name
			*/
			bool ContainsNamespace(const std::string& namespaceName) const;
		};
	}
}