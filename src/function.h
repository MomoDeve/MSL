#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "baseExpression.h"

class Class;

class Function
{
	using LocalScopeTable = std::unordered_map<std::string, size_t>;
	using VariableArray = std::vector<std::string>;
	using ParameterArray = std::vector<std::string>;
	using DependencyTable = std::unordered_map<std::string, size_t>;

	LocalScopeTable locals;
	DependencyTable dependencies;
	VariableArray variables;

public:	
	enum Modifiers : uint8_t
	{
		_ABSTRACT = 1,
		_STATIC = 2,
		_PUBLIC = 4,
		_ENTRY_POINT = 128,
	};

	ParameterArray params;
	std::unique_ptr<ExpressionList> body;
	uint8_t modifiers;
	std::string name;

	bool isAbstract() const;
	bool isStatic() const;
	bool isPublic() const;
	bool isEntryPoint() const;
	bool hasBody() const;

	std::string toString() const;
	const VariableArray& getVariables() const;

	Function(std::string name);
	Function(Function&& function);
	Function(const Function& function) = delete;

	void InsertLocal(const std::string& localName);
	void InsertDependency(const std::string& dependencyName);
	bool ContainsLocal(const std::string& localName) const;
	bool ContainsDependency(const std::string& dependenctyName) const;

	static std::string GenerateUniqueName(const std::string& name, size_t paramSize);
};
