#pragma once

#include <string>

namespace MSL
{
	namespace VM
	{
		struct BaseObject;

		struct AttributeType
		{
			enum Modifiers
			{
				STATIC = 1,
				CONST = 2,
				PUBLIC = 4,
			};
			std::string name;
			uint8_t modifiers = 0;

			AttributeType() = default;
			AttributeType(AttributeType&&) = default;
		};
	}
}