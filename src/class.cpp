#include "class.h"
#include <sstream>

namespace MSL
{
	namespace compiler
	{

		bool Class::IsStatic() const
		{
			return modifiers & Modifiers::_STATIC;
		}

		bool Class::IsAbstract() const
		{
			return modifiers & Modifiers::_ABSTRACT;
		}

		bool Class::IsPrivate() const
		{
			return modifiers & Modifiers::_PRIVATE;
		}

		Class::Class(std::string name)
			: name(std::move(name)), modifiers(0) { }

		void Class::InsertMethod(const std::string& name, Method&& function)
		{
			size_t index = methods.size();
			bool isFunction = true;

			table.insert({ name, TableIndex{isFunction, index} });
			methods.push_back(std::move(function));
		}

		void Class::InsertAttribute(const std::string& name, Attribute&& attribute)
		{
			size_t index = attributes.size();
			bool isFunction = false;
			table.insert({ name, TableIndex{isFunction, index} });
			attributes.push_back(std::move(attribute));
		}

		const Class::AttributeArray& Class::GetAttributes() const
		{
			return attributes;
		}

		const Class::MethodArray& Class::GetMethods() const
		{
			return methods;
		}

		bool Class::ContainsMember(const std::string& memberName) const
		{
			return table.find(memberName) != table.end();
		}

		bool operator==(const Method& f1, const Method& f2)
		{
			return (f1.params.size() == f2.params.size()) && (f1.name == f2.name);
		}
	}
}