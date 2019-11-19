#pragma once
#include <unordered_map>
#include <Windows.h>

namespace MSL
{
	namespace VM
	{
		class DllLoader
		{
			using DllHashMap = std::unordered_map<std::string, HMODULE>;
			DllHashMap modules;
		public:
			using DllFunction = void(*)();

			~DllLoader();
			void AddLibrary(const std::string& filename);
			void FreeLibrary(const std::string& filename);
			DllFunction GetFunctionPointer(const std::string& module, const std::string& function) const;
			DWORD GetLastError() const;
		};
	}
}