#include "namespace.h"

namespace MSL
{
	namespace compiler
	{
		Namespace::Namespace()
			: Namespace("unnamed") { }

		Namespace::Namespace(std::string name)
			: name(std::move(name)) { }

		const std::string& Namespace::GetName() const
		{
			return name;
		}

		std::string Namespace::ToString() const
		{
			return std::string("namespace " + GetName());
		}

		const Namespace::ClassArray& Namespace::GetMembers() const
		{
			return classes;
		}

		bool Namespace::ContainsClass(const std::string& className) const
		{
			return table.find(className) != table.end();
		}

		void Namespace::InsertClass(const std::string& name, Class&& _class)
		{
			size_t index = classes.size();
			classes.push_back(std::move(_class));
			table.insert({ name, index });
		}
	}
}