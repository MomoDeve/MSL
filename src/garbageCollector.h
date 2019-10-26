#pragma once

#include <chrono>

#include "assemblyType.h"
#include "objects.h"
#include "SlabAllocator.h"
#include "callPath.h"

namespace MSL
{
	enum class Type : uint8_t
	{
		INTEGER,
		FLOAT,
		STRING,
		LOCAL,
		ATTRIBUTE,
	};
	namespace VM
	{
		class GarbageCollector
		{
			template<typename T>
			using Allocator = std::unique_ptr<momo::SlabAllocator<T>>;

			std::ostream* out;
			size_t clearedObjects;
			size_t managedObjects;
			size_t allocSinceIter = 0;
			std::chrono::time_point<std::chrono::system_clock> lastIter;

			template<typename T>
			inline void Init(Allocator<T>& allocator, size_t initSize)
			{
				allocator.reset(new momo::SlabAllocator<T>(initSize));
			}

			template<typename T>
			inline void ClearObjectsInSlab(T& slab);

			template<typename T>
			inline void ClearSlabs(Allocator<T>& allocator);
		public:
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

			GarbageCollector(std::ostream* log = nullptr, size_t allocSize = 1);
			void SetLogStream(std::ostream* log);
			void SetInitCapacity(size_t capacity);
			void Collect(AssemblyType& assembly, std::vector<CallPath>& callStack, std::vector<BaseObject*> objectStack);
			void ReleaseMemory();
			std::chrono::milliseconds GetTimeSinceLastIteration() const;
			size_t GetTotalAllocCount() const;
			size_t GetAllocSinceIter() const;
			size_t GetClearedObjectCount() const;
		};

		template<typename T>
		inline void GarbageCollector::ClearObjectsInSlab(T& slab)
		{
			for (int i = 0; i < slab.maxSize; i++)
			{
				auto* objPtr = slab.GetNativePointer() + i;
				switch (objPtr->state)
				{
				case GCstate::UNMARKED:
					slab.Free(objPtr);
					objPtr->state = GCstate::FREE;
					this->clearedObjects++;
					break;
				case GCstate::MARKED:
					objPtr->state = GCstate::UNMARKED;
					this->managedObjects++;
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
			for (auto& slab : allocator->GetBusySlabs())
			{
				ClearObjectsInSlab(slab);
			}
			for (auto& slab : allocator->GetPartialSlabs())
			{
				ClearObjectsInSlab(slab);
			}
			allocator->ReallocateSlabs();
		}
	}
}