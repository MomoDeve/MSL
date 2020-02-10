#include "attribute.h"

namespace MSL
{
	namespace compiler
	{
		bool Attribute::IsStatic() const
		{
			return modifiers & Modifiers::_STATIC;
		}

		bool Attribute::isConst() const
		{
			return modifiers & Modifiers::_CONST;
		}

		bool Attribute::isPublic() const
		{
			return modifiers & Modifiers::_PUBLIC;
		}

		Attribute::Attribute(const std::string& name)
			: name(name), modifiers(0) { }
	}
}