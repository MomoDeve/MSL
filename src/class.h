#pragma once

#include <map>
#include <string>
#include <sstream>
#include <typeinfo>
#include <memory>
#include <unordered_map>

#include "function.h"
#include "attribute.h"
#include "stringExtensions.h"

class Class
{
	struct TableIndex
	{
		bool isFunction;
		size_t index;
	};
	using MemberTable = std::unordered_map<std::string, TableIndex>;
	using AttributeArray = std::vector<Attribute>;
	using MethodArray = std::vector<Function>;

	MemberTable table;
	AttributeArray attributes;
	MethodArray methods;

public:
	enum Modifiers
	{
		_STATIC = 1,
		_INTERFACE = 2,
		_ABSTRACT = 4,
		_CONST = 8,
		_INTERNAL = 16,
	};

	std::string name;
	uint8_t modifiers;

	bool isConst() const;
	bool isStatic() const;
	bool isInterface() const;
	bool isAbstract() const;
	bool isInternal() const;

	Class(std::string name);
	Class(const Class& other) = delete;
	Class(Class&& other);

	void InsertMethod(const std::string& name, Function&& function);
	void InsertAttribute(const std::string& name, Attribute&& attribute);

	const AttributeArray& GetAttributes() const;
	const MethodArray& GetMethods() const;
	bool ContainsMember(const std::string& memberName) const;
	std::string toString() const;
};