#pragma once
#include <iostream>

namespace MSL
{
	namespace VM
	{
		constexpr uint64_t KB = 1024;
		constexpr uint64_t MB = KB * 1024;
		constexpr uint64_t GB = MB * 1024;

		struct Configuration
		{
			struct
			{
				std::istream* in = &std::cin;
				std::ostream* out = &std::cout;
				std::ostream* error = &std::cerr;
			} streams;
			struct
			{
				uint64_t initCapacity = 1;
				uint64_t minMemory = 4 * MB;
				uint64_t maxMemory = 1 * GB;
				std::ostream* log = nullptr;
				bool allowCollect = true;
			} GC;
			struct
			{
				bool varifyBytecode = true;
				bool allowAssemblyMerge = true;
			} compilation;
			struct
			{
				size_t recursionLimit = 2000;
				bool checkExitCode = true;
				bool useUnicode = true;
				bool safeMode = false;
				bool displayInfo = false;
			} execution;
		};
	}
}
