#pragma once
#include <iostream>

namespace MSL
{
	namespace VM
	{
		constexpr size_t KB = 1024;
		constexpr size_t MB = 1024 * KB;

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
				size_t msCollectInterval = 250;
				std::ostream* log = nullptr;
				size_t initCapacity = 1;
				size_t initMemory = 4 * MB;
			} GC;
			struct
			{
				bool varifyBytecode = true;
				bool allowAssemblyMerge = true;
				bool allowMemoryPreallocation = true;
			} compilation;
			struct
			{
				size_t recursionLimit = 2000;
				bool checkExitCode = true;
				bool useUnicode = true;
			} execution;
		};
	}
}
