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
			if (useFunctionCache)
			{
				if (!functionCache.Has(module) || !functionCache[module].Has(function))
				{
					if (!HasLibrary(module)) return nullptr;
					auto pointer = DllFunction(::GetProcAddress(modules.at(module), function.c_str()));
					if (pointer == nullptr) return nullptr;
					functionCache[module][function] = pointer;
				}
				return functionCache[module][function];
			}
			else
			{
				if (!HasLibrary(module))
					return nullptr;
				return DllFunction(::GetProcAddress(modules.at(module), function.c_str()));
			}
		}

		bool DllLoader::HasLibrary(const std::string& filename) const
		{			
			return modules.find(filename) != modules.end() || (useFunctionCache && functionCache.Has(filename));
		}

		DWORD DllLoader::GetLastError() const
		{
			return ::GetLastError();
		}

		void DllLoader::UseFunctionCache(bool value)
		{
			useFunctionCache = value;
		}

		void DllLoader::AddDllFunction(const std::string& module, const std::string& function, DllFunction pointer)
		{
			if (useFunctionCache)
			{
				functionCache[module][function] = pointer;
			}
		}
	}
}
