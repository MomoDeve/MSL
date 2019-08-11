#pragma once

#include <string>

namespace MSL
{
	namespace VM
	{
		struct AttributeType
		{
			std::string name;
			uint8_t modifiers = 0;

			AttributeType() = default;
			AttributeType(AttributeType&&) = default;
		};
	}
}