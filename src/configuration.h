#pragma once
#include <iostream>

namespace MSL
{
	namespace VM
	{
		constexpr size_t KB = 1024;
		constexpr size_t MB = KB * 1024;
		constexpr size_t GB = MB * 1024;

		struct Configuration
		{
			struct
			{
				std::istream* in = nullptr;
				std::ostream* out = nullptr;
				std::ostream* error = nullptr;
			} streams;
			struct
			{
				size_t initAllocBytes = 4 * KB;
				size_t maxAllocBytes = 1 * GB;
				size_t allocMultiplier = 2;
			} GC;
			struct
			{
				bool varifyBytecode = true;
				bool allowAssemblyMerge = true;
				bool allowMemoryPreallocation = true;
			} compilation;
			struct
			{
				size_t recursionLimit = 20000;
				bool checkExitCode = true;
			} execution;
		};
	}
}
