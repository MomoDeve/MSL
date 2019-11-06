#pragma once
#include <string>

namespace MSL
{
	namespace compiler
	{
		/*
		Attribute class is a member of class with unique name and modifiers
		*/
		class Attribute
		{
		public:
			/*
			bit-masks of attribute modifiers
			*/
			enum Modifiers
			{
				_STATIC = 1,
				_CONST = 2,
				_PUBLIC = 4,
			};
			/*
			unique name of attribute. Class must not contain more than one attribute with the same name
			*/
			std::string name;
			/*
			modifiers of attribute. Each modifier can be checked for existance by a special method Attribute::is[...]()
			*/
			uint8_t modifiers;

			/*
			creates attribute with name provided
			*/
			Attribute(const std::string& name);

			/*
			checks if the attribute is static
			*/
			bool isStatic() const;
			/*
			checks if the attribute is constant
			*/
			bool isConst() const;
			/*
			checks if the attribute is public
			*/
			bool isPublic() const;
		};
	}
}