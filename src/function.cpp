#include "function.h"
#include <sstream>

namespace MSL
{
	namespace compiler
	{
		bool Method::IsAbstract() const
		{
			return modifiers & Modifiers::_ABSTRACT;
		}

		bool Method::IsStatic() const
		{
			return modifiers & Modifiers::_STATIC;
		}

		bool Method::isPublic() const
		{
			return modifiers & Modifiers::_PUBLIC;
		}

		bool Method::IsEntryPoint() const
		{
			return modifiers & Modifiers::_ENTRY_POINT;
		}

		bool Method::isConstructor() const
		{
			return modifiers & Modifiers::_CONSTRUCTOR;
		}

		std::string Method::ToString() const
		{
			std::stringstream out;

			out << "function ";
			out << name;
			out << "(";

			for (int i = 0; i < (int)params.size(); i++)
			{
				out << params[i];
				if (i != (int)params.size() - 1)
				{
					out << ", ";
				}
			}
			out << ")";
			return out.str();
		}

		const Method::VariableArray& Method::GetVariables() const
		{
			return variables;
		}

		Method::Method(std::string name)
			: name(std::move(name)), modifiers(0), labelInnerId(0) { }

		void Method::InsertLocal(const std::string& localName)
		{
			size_t index = variables.size();
			variables.push_back(localName);
			locals.insert({ localName, index });
		}

		void Method::RemoveLocal(const std::string& localName)
		{
			locals.erase(localName);
		}

		void Method::InsertDependency(const std::string& dependencyName)
		{
			if (!ContainsDependency(dependencyName) && !ContainsLocal(dependencyName))
			{
				size_t index = variables.size();
				variables.push_back(dependencyName);
				dependencies.insert({ dependencyName, index });
			}
		}

		bool Method::ContainsLocal(const std::string& localName) const
		{
			return locals.find(localName) != locals.end();
		}

		bool Method::ContainsDependency(const std::string& dependenctyName) const
		{
			return dependencies.find(dependenctyName) != dependencies.end();
		}

		size_t Method::GetHash(const std::string& variableName) const
		{
			auto localIter = locals.find(variableName);
			if (localIter != locals.end())
			{
				return localIter->second;
			}
			else
			{
				return dependencies.find(variableName)->second;
			}
		}

        const Method::BytecodeArray& Method::GetBytecode() const
        {
            return bytecode;
        }

        void Method::PushError(const std::string& error)
        {
            this->errors.push_back(error);
        }

		std::string Method::GenerateUniqueName(const std::string& name, size_t paramSize)
		{
            return name + '_' + std::to_string(paramSize);                
		}
	}
}