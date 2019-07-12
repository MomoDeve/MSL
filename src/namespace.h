#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "Class.h"

class Namespace
{
	using ClassTable = std::unordered_map<std::string, size_t>;
	using ClassArray = std::vector<Class>;

	ClassTable table;
	ClassArray classes;
	std::string name;
public:
	Namespace();
	Namespace(std::string name);
	Namespace(const Namespace& other) = delete;
	Namespace(Namespace&& other);

	std::string getName() const;
	std::string toString() const;
	const ClassArray& getMembers() const;
	bool ContainsClass(const std::string& className) const;

	void addClass(const std::string& name, Class&& _class);
};