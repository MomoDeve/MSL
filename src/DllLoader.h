#pragma once
#include <unordered_map>
#include <Windows.h>

#include "cacher.h"

namespace MSL
{
	namespace VM
	{
		class DllLoader
		{
			class VirtualMachine;
		public:
			using DllFunction = void(*)();
		private:
			using DllHashMap = std::unordered_map<std::string, HMODULE>;
			using FunctionCache = momo::Cacher<std::string, momo::Cacher<std::string, DllFunction>>;
			mutable FunctionCache functionCache;
			DllHashMap modules;
			bool useFunctionCache = true;
		public:
			~DllLoader();
			void AddLibrary(const std::string& filename);
			void FreeLibrary(const std::string& filename);
			DllFunction GetFunctionPointer(const std::string& module, const std::string& function) const;
			bool HasLibrary(const std::string& filename) const;
			DWORD GetLastError() const;
			void UseFunctionCache(bool value);
			void AddDllFunction(const std::string& module, const std::string& function, DllFunction pointer);
		};
	}
}