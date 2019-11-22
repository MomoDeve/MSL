#include "DllLoader.h"

namespace MSL
{
	namespace VM
	{
		DllLoader::~DllLoader()
		{
			for (const auto& module : modules)
			{
				::FreeLibrary(module.second);
			}
		}

		void DllLoader::AddLibrary(const std::string& filename)
		{
			modules.insert({ filename, ::LoadLibrary(filename.c_str()) });
		}

		void DllLoader::FreeLibrary(const std::string& filename)
		{
			if (modules.find(filename) != modules.end())
			{
				::FreeLibrary(modules[filename]);
				modules.erase(filename);
			}
		}

		DllLoader::DllFunction DllLoader::GetFunctionPointer(const std::string& module, const std::string& function) const
		{
			if (modules.find(module) == modules.end()) 
				return nullptr;

			return reinterpret_cast<DllFunction>(::GetProcAddress(modules.at(module), function.c_str()));
		}

		DWORD DllLoader::GetLastError() const
		{
			return ::GetLastError();
		}
	}
}
