#pragma once

#include <string>

class Attribute
{
public:
	enum Modifiers
	{
		_STATIC = 1,
		_CONST = 2,
		_PUBLIC = 4,
	};
	std::string name;
	uint8_t modifiers;

	Attribute(const std::string& name);

	bool isStatic() const;
	bool isConst() const;
	bool isPublic() const;
};
