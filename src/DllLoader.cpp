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
			if (HasLibrary(filename)) FreeLibrary(filename); // reload
			modules.insert({ filename, ::LoadLibraryA(filename.c_str()) });
		}

		void DllLoader::FreeLibrary(const std::string& filename)
		{
			if (HasLibrary(filename))
			{
				::FreeLibrary(modules[filename]);
				modules.erase(filename);
			}
		}

		DllLoader::DllFunction DllLoader::GetFunctionPointer(const std::string& module, const std::string& function) const
		{
			if (!HasLibrary(module)) 
				return nullptr;

			return reinterpret_cast<DllFunction>(::GetProcAddress(modules.at(module), function.c_str()));
		}

		bool DllLoader::HasLibrary(const std::string& filename) const
		{
			return modules.find(filename) != modules.end();
		}

		DWORD DllLoader::GetLastError() const
		{
			return ::GetLastError();
		}
	}
}
