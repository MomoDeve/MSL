#pragma once

#include <vector>
#include <string>

namespace MSL
{
	namespace VM
	{
		class CallPath
		{
			using Path = std::vector<const std::string*>;
			Path path = Path(3);
		public:
			const std::string* GetNamespace() const;
			const std::string* GetClass() const;
			const std::string* GetMethod() const;

			void SetNamespace(const std::string* ns);
			void SetClass(const std::string* c);
			void SetMethod(const std::string* method);
		};
	}
}