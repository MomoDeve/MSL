#include "class.h"

namespace MSL
{
	namespace compiler
	{
		bool Class::IsConst() const
		{
			return modifiers & Modifiers::_CONST;
		}

		bool Class::IsStatic() const
		{
			return modifiers & Modifiers::_STATIC;
		}

		bool Class::IsInterface() const
		{
			return modifiers & Modifiers::_INTERFACE;
		}

		bool Class::IsAbstract() const
		{
			return modifiers & Modifiers::_ABSTRACT;
		}

		bool Class::IsInternal() const
		{
			return modifiers & Modifiers::_INTERNAL;
		}

		Class::Class(std::string name)
			: name(std::move(name)), modifiers(0) { }

		void Class::InsertMethod(const std::string& name, Function&& function)
		{
			size_t index = methods.size();
			bool isFunction = true;

			std::string overloadedFunctionName = Function::GenerateUniqueName(function.name, function.params.size());

			table.insert({ overloadedFunctionName, TableIndex{isFunction, index} });
			if (!ContainsMember(name))
			{
				table.insert({ name, TableIndex{isFunction, index} });
			}
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

		std::string Class::ToString() const
		{
			std::stringstream out;

			out << (IsInternal() ? "internal " : "public ");
			if (IsInterface())
			{
				out << "interface ";
			}
			else
			{
				out << (IsStatic() ? "static " : "");
				out << (IsAbstract() ? "abstract " : "");
				out << "class ";
			}
			out << name << "\n{\n\t";

			out << "modifiers: ";
			#define PRINT(str, val) out << "\n\t\t" str ": " << BOOL(val)
			PRINT("internal", IsInternal());
			PRINT("static", IsStatic());
			PRINT("abstract", IsAbstract());
			#undef PRINT
			out << "\n";

			if (!IsInterface())
			{
				out << "\tattributes: \n";
				for (const auto& attr : attributes)
				{
					out << "\t\t";

					out << (attr.isPublic() ? "public " : "private ");
					out << (attr.isStatic() ? "static " : "");
					out << (attr.isConst() ? "const " : "");

					out << "var ";
					out << attr.name << ";\n";
				}
			}

			out << "\tmethods: \n";
			for (const auto& method : methods)
			{
				out << "\t\t";
				out << (method.isEntryPoint() ? "[[ ENTRY POINT ]] " : "");
				out << (method.isConstructor() ? "[[ CONSTRUCTOR ]] " : "");
				out << (method.isPublic() ? "public " : "private ");
				out << (method.isStatic() ? "static " : "");
				out << (method.hasBody() ? "" : "pure ");
				out << (method.isAbstract() ? "abstract " : "");
				out << method.ToString();

				const int functionBodyDepth = 3;
				if (method.hasBody())
				{
					out << "\n\t\t{\n";
					for (const auto& expr : *method.body)
					{
						expr->Print(out, functionBodyDepth);
						out << '\n';
					}
					out << "\t\t}\n";
				}
				else
				{
					out << ";\n";
				}
			}
			out << "}\n";
			return out.str();
		}

		bool operator==(const Function& f1, const Function& f2)
		{
			return (f1.params.size() == f2.params.size()) && (f1.name == f2.name);
		}
	}
}