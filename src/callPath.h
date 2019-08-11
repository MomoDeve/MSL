#pragma once

#include <vector>
#include <string>

namespace MSL
{
	namespace VM
	{
		struct CallPath
		{
			using Path = std::vector<std::string>;
			Path path = Path(3);
			enum ArrayIndex
			{
				NAMESPACE = 0,
				CLASS = 1,
				METHOD = 2,
			};
		};
	}
}