#pragma once

#include "assemblyType.h"
#include "objects.h"
#include "SlabAllocator.h"
#include "callPath.h"
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

			template<typename T>
			inline void Init(Allocator<T>& allocator, uint64_t initSize)
			{
				allocator.reset(new momo::SlabAllocator<T>((size_t)initSize));
			}

			template<typename T>
			inline void ClearObjectsInSlab(Allocator<T>& allocator, momo::Slab<T, uint8_t>& slab);

			template<typename T>
			inline void ClearSlabs(Allocator<T>& allocator);
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
			Allocator<Frame> frameAlloc;

			GarbageCollector(std::ostream* log = nullptr, size_t allocSize = 1);
			void SetLogStream(std::ostream* log);
			void SetInitCapacity(uint64_t capacity);
			void Collect(AssemblyType& assembly, std::vector<CallPath>& callStack, std::vector<BaseObject*> objectStack);
			void ReleaseMemory();
			void ReleaseFreeMemory();
			std::chrono::milliseconds GetTimeSinceLastIteration() const;
			uint64_t GetTotalMemoryAlloc() const;
			uint64_t GetMemoryAllocSinceIter() const;
			uint64_t GetClearedMemorySinceIter() const;
			uint64_t GetClearedObjectCount() const;
			uint64_t GetTotalIterations() const;
			virtual void PrintLog() const;
		};

		template<typename T>
		inline void GarbageCollector::ClearObjectsInSlab(Allocator<T>& allocator, momo::Slab<T, uint8_t>& slab)
		{
			for (int i = 0; i < slab.maxSize; i++)
			{
				auto* objPtr = slab.GetNativePointer() + i;
				switch (objPtr->state)
				{
				case GCstate::UNMARKED:
					objPtr->state = GCstate::FREE;
					this->clearedObjects++;
					this->clearedMemory += objPtr->GetSize();
					allocator->allocCount--;
					slab.Free(objPtr);
					break;
				case GCstate::MARKED:
					objPtr->state = GCstate::UNMARKED;
					this->managedObjects++;
					this->managedMemory += objPtr->GetSize();
					break;
				case GCstate::FREE:
					// object already has been destroyed
					break;
				}
			}
		}

		template<typename T>
		inline void GarbageCollector::ClearSlabs(Allocator<T>& allocator)
		{
			allocator->managedMemory = 0;
			for (auto& slab : allocator->GetBusySlabs())
			{
				ClearObjectsInSlab(allocator, slab);
			}
			for (auto& slab : allocator->GetPartialSlabs())
			{
				ClearObjectsInSlab(allocator, slab);
			}
			allocator->ReallocateSlabs();
		}
	}
}