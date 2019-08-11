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
			struct Streams
			{
				std::istream* in = nullptr;
				std::ostream* out = nullptr;
				std::ostream* error = nullptr;
			} streams;
			struct GC
			{
				size_t initAllocBytes = 4 * KB;
				size_t maxAllocBytes = 1 * GB;
				size_t allocMultiplier = 2;
			} GC;
			struct Compilation
			{
				bool varifyBytecode = true;
				bool allowAssemblyMerge = true;
				bool allowMemoryPreallocation = true;
			} compilation;
		};
	}
}
