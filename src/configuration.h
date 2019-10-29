#pragma once
#include <iostream>

namespace MSL
{
	namespace VM
	{
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
				uint64_t initAlloc = 1024;
				uint64_t maxAlloc = 512 * 1024 * 1024;
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
			} execution;
		};
	}
}
