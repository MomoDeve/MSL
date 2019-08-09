#include "namespace.h"

namespace MSL
{
	namespace compiler
	{
		Namespace::Namespace()
			: Namespace("unnamed") { }

		Namespace::Namespace(std::string name)
			: name(std::move(name)) { }

		Namespace::Namespace(Namespace&& other)
			: table(std::move(other.table)), classes(std::move(other.classes)), name(std::move(other.name))
		{
		}

		std::string Namespace::getName() const
		{
			return name;
		}

		std::string Namespace::toString() const
		{
			return std::string("namespace " + getName());
		}

		const Namespace::ClassArray& Namespace::getMembers() const
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