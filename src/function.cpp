#include "function.h"

bool Function::isAbstract() const
{
	return modifiers & Modifiers::_ABSTRACT;
}

bool Function::isStatic() const
{
	return modifiers & Modifiers::_STATIC;
}

bool Function::isPublic() const
{
	return modifiers & Modifiers::_PUBLIC;
}

bool Function::isEntryPoint() const
{
	return modifiers & Modifiers::_ENTRY_POINT;
}

bool Function::hasBody() const
{
	return !body->empty();
}

std::string Function::toString() const
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

const Function::VariableArray& Function::getVariables() const
{
	return variables;
}

Function::Function(std::string name)
	: name(std::move(name)), modifiers(0) { }

Function::Function(Function&& function)
	: params(std::move(function.params)), body(std::move(function.body)), modifiers(function.modifiers),
	  name(std::move(function.name)), locals(std::move(function.locals)), 
	  variables(std::move(function.variables)), dependencies(std::move(function.dependencies))
{
}

void Function::InsertLocal(const std::string& localName)
{
	size_t index = variables.size();
	variables.push_back(localName);
	locals.insert({ localName, index });
}

void Function::InsertDependency(const std::string& dependencyName)
{
	if (!ContainsDependency(dependencyName) && !ContainsLocal(dependencyName))
	{
		size_t index = variables.size();
		variables.push_back(dependencyName);
		dependencies.insert({ dependencyName, index });
	}
}
 
bool Function::ContainsLocal(const std::string& localName) const
{
	return locals.find(localName) != locals.end();
}

bool Function::ContainsDependency(const std::string& dependenctyName) const
{
	return dependencies.find(dependenctyName) != dependencies.end();
}

size_t Function::GetHash(const std::string& variableName) const
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

void Function::GenerateBytecode(CodeGenerator& generator) const
{
	if (hasBody())
	{
		for (const auto& expr : *body)
		{
			expr->GenerateBytecode(generator, *this);
		}
	}
}

std::string Function::GenerateUniqueName(const std::string& name, size_t paramSize)
{
	return name + '_' + std::to_string(paramSize);
}
