#include "assembly.h"

namespace MSL
{
	namespace compiler
	{
		Assembly::Assembly(Assembly&& assembly)
			: table(std::move(assembly.table)), namespaces(std::move(assembly.namespaces))
		{
		}

		void Assembly::InsertNamespace(const std::string& name, Namespace&& _namespace)
		{
			size_t index = namespaces.size();
			namespaces.push_back(std::move(_namespace));
			table.insert({ name, index });
		}

		const Assembly::NamespaceArray& Assembly::GetNamespaces() const
		{
			return namespaces;
		}

		bool Assembly::ContainsNamespace(const std::string& namespaceName) const
		{
			return table.find(namespaceName) != table.end();
		}
	}
}