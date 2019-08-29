#pragma once

#include <string>
#include <vector>

namespace MSL
{
	namespace VM
	{
		struct MethodType
		{
			enum Modifiers
			{
				ABSTRACT = 1,
				STATIC = 2,
				PUBLIC = 4,
				CONSTRUCTOR = 8,
				STATIC_CONSTRUCTOR = 16,
				ENTRY_POINT = 128,
			};

			using StringArray = std::vector<std::string>;
			using ByteArray = std::vector<uint8_t>;
			using LabelOffsetArray = std::vector<size_t>;
			StringArray parameters;
			StringArray dependencies;
			LabelOffsetArray labels;
			ByteArray body;

			std::string name;
			uint8_t modifiers = 0;

			MethodType() = default;
			MethodType(MethodType&&) = default;
		};
	}
}