#pragma once

#include "msl_types.h"
#include "SlabAllocator.h"
#include <memory>
#include <chrono>

namespace MSL
{
	namespace VM
	{
		class GarbageCollector
		{
			template<typename T>
			using Allocator = std::unique_ptr<momo::SlabAllocator<T>>;

			std::ostream* out;
			uint64_t clearedObjects;
			uint64_t managedObjects;
			uint64_t allocSinceIter = 0;
			uint64_t clearedMemory;
			uint64_t totalIters = 0;
			uint64_t managedMemory = 0;
			std::chrono::time_point<std::chrono::system_clock> lastIter;

		public:
			NullObject nullObject;
			TrueObject trueObject;
			FalseObject falseObject;

			Allocator<ClassObject> classObjAlloc;
			Allocator<ClassWrapper> classWrapAlloc;
			Allocator<NamespaceWrapper> nsWrapAlloc;
			Allocator<UnknownObject> unknownObjAlloc;
			Allocator<IntegerObject> integerAlloc;
			Allocator<FloatObject> floatAlloc;
			Allocator<StringObject> stringAlloc;
			Allocator<LocalObject> localObjAlloc;
			Allocator<AttributeObject> attributeAlloc;
			Allocator<ArrayObject> arrayAlloc;
		};
	}
}