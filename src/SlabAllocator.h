#pragma once

#include <vector>

// #define MEMORY_CHECK
// uncomment define statement to enable check for memory access

#pragma region Slab
namespace momo
{
	template<typename ElementT, typename IndexT>
	class Slab
	{
	public:
		static constexpr size_t maxSize = 1 << (sizeof(IndexT) * 8); // max indexing size and maximal amount of objects in slab
		using ObjectType = ElementT;
	private:
		#ifdef MEMORY_CHECK
		std::vector<bool> isFree; // bit array to check if object was allocated in slab
		#endif 
		ElementT* memory; // pointer to memory (list of slab blocks)
		IndexT* freeTable; // pointer to table of free objects, such that `freeTable[current_free] = next_free`
		IndexT curFree; // current free object ready for allocation. Must not be accessed if all objects has been allocated
		size_t size; // current size (amount of objects allocated)
	public:
		Slab();
		Slab(const Slab&) = delete;
		Slab(Slab&& slab);
		Slab& operator=(Slab&& slab);
		~Slab();
		/*
		allocates object in slab and constructs it with arguments provided
		*/
		template<typename... Args> ElementT* Alloc(Args&&... args);
		/*
		frees object in slab. behavior is undefined if pointer does not belong to the slab. 
		*/
		void Free(ElementT* value);
		/*
		returns current amount of object being allocated
		*/
		size_t GetSize() const;
		/*
		returns size of object - sizeof(ElementT)
		*/
		size_t GetObjectSize() const;
		/*
		returns pointer to the beginning of memory. This pointer must not be deleted outside of class
		*/
		ElementT* GetNativePointer();
	};

	template<typename ElementT, typename IndexT>
	inline Slab<ElementT, IndexT>::Slab()
		: size(0), curFree(0)
	{
		
		memory = (ElementT*)malloc(maxSize * sizeof(ElementT));
		if(memory != nullptr) memset(memory, 0, maxSize * sizeof(ElementT));
		freeTable = (IndexT*)malloc(maxSize * sizeof(IndexT));

		#ifdef MEMORY_CHECK
		_ASSERTE(memory != nullptr);
		_ASSERTE(freeTable != nullptr);
		#endif			
		for (int i = 0; i < maxSize; i++)
		{
			freeTable[i] = i + 1;
		}

		#ifdef MEMORY_CHECK
		isFree.resize(maxSize, true);
		#endif
	}

	template<typename ElementT, typename IndexT>
	inline Slab<ElementT, IndexT>::Slab(Slab&& slab)
	{
		memory = slab.memory;
		size = slab.size;
		freeTable = slab.freeTable;
		curFree = slab.curFree;

		#ifdef MEMORY_CHECK
		isFree = std::move(slab.isFree);
		#endif

		slab.memory = nullptr;
		slab.freeTable = nullptr;
	}

	template<typename ElementT, typename IndexT>
	inline Slab<ElementT, IndexT>& Slab<ElementT, IndexT>::operator=(Slab&& slab)
	{
		memory = slab.memory;
		size = slab.size;
		freeTable = slab.freeTable;
		curFree = slab.curFree;

		#ifdef MEMORY_CHECK
		isFree = std::move(slab.isFree);
		#endif

		slab.memory = nullptr;
		slab.freeTable = nullptr;
		return *this;
	}

	template<typename ElementT, typename IndexT>
	inline Slab<ElementT, IndexT>::~Slab()
	{
		if (freeTable != nullptr)
			free(freeTable);
		if (memory != nullptr)
			free(memory);
	}

	template<typename ElementT, typename IndexT>
	template<typename... Args>
	inline ElementT* Slab<ElementT, IndexT>::Alloc(Args&&... args)
	{
		if (size == maxSize) return nullptr;
		size++;
		IndexT index = curFree;

		#ifdef MEMORY_CHECK
		_ASSERTE(isFree[index] && "tried to allocate object which was already allocated");
		isFree[index] = false;
		#endif

		ElementT* element = new(memory + index) ElementT(std::forward<Args>(args)...);

		curFree = freeTable[index];
		return element;
	}

	template<typename ElementT, typename IndexT>
	inline void Slab<ElementT, IndexT>::Free(ElementT* block)
	{
		IndexT index = block - memory;

		#ifdef MEMORY_CHECK
		_ASSERTE(memory <= block && "pointer does not belongs to the Slab");
		_ASSERTE((block < memory + maxSize) && "pointer does not belongs to the Slab");
		_ASSERTE(!isFree[index] && "tried to free object which was already freed");
		isFree[index] = true;
		#endif

		memory[index].~ElementT();

		size--;
		freeTable[index] = curFree;
		curFree = index;
	}

	template<typename ElementT, typename IndexT>
	inline size_t Slab<ElementT, IndexT>::GetSize() const
	{
		return size;
	}

	template<typename ElementT, typename IndexT>
	inline size_t Slab<ElementT, IndexT>::GetObjectSize() const
	{
		return sizeof(ElementT);
	}

	template<typename ElementT, typename IndexT>
	ElementT* Slab<ElementT, IndexT>::GetNativePointer()
	{
		return memory;
	}
#pragma endregion
	template<typename ElementT, typename IndexT = uint8_t>
	class SlabAllocator
	{
		using Slab = Slab<ElementT, IndexT>;
		using SlabIt = typename std::vector<Slab>::iterator;
		std::vector<Slab> busySlabs, partialSlabs, freeSlabs; // lists of slabs
		uint64_t allocSize; // amount of new slabs allocated in case that free list become empty
		void MoveFreeToPartialIfNeed(); // moves free slab to partial if there are no partial slabs available
		void AllocateFreeIfNeed(); // allocated [allocSize] free slabs in case all free slabs were moved to partial list
		void MovePartialToBusyIfNeed(); // moves partial slab to busy in case all objects in slab were allocated
		void MoveBusyToPartial(SlabIt slabIt); // moves busy slab to partial by iterator in case any of busy objects was freed
		void MovePartialToFreeIfNeed(SlabIt slabIt); // moves partial slab to free by iterator in case all objects in slab were freed
		bool FreeIfInBusy(ElementT* value); // checks if pointer belongs to any of busy slabs and frees object if it was found
		bool FreeIfInPartial(ElementT* value); // checks if pointer belongs to any of partial slabs and frees object if it was found
		bool InRange(ElementT* begin, ElementT* value, ElementT* end) const; // checks if pointer belongs to [begin; end] interval
	public:
		uint64_t allocCount = 0; // amount of objects allocated in slabs
		uint64_t managedMemory = 0; // MSL GC derived. Count memory (in bytes), allocated by objects
		/*
		construct allocator object with [freeAllocCount] slabs in free list
		*/
		SlabAllocator(size_t freeAllocCount);
		/*
		allocates object in slab. Object is being constructed using args provided. Returns nullptr on failure
		*/
		template<typename... Args> ElementT* Alloc(Args&&... args);
		/*
		frees object in slab. If pointer does not belong to any of the slabs, no action is performed
		*/
		void Free(ElementT* value);
		/*
		forces allocator to move slabs according to their busyness. Note that allocator automatically does that during Free() method
		*/
		void ReallocateSlabs();
		/*
		forces allocator to clear empty slab list and release all its memory. Calling this function can reduce performance
		*/
		void ReleaseFreeSlabs();
		/*
		returns total amount of allocated objects in slabs
		*/
		uint64_t GetAllocCount() const;
		/*
		returns object size in bytes - sizeof(ElementT)
		*/
		size_t GetObjectSize() const;
		/*
		returns total amount of memory occupied by allocator
		*/
		uint64_t GetTotalMemory() const;
		/*
		returns reference to free slab list
		*/
		std::vector<Slab>& GetFreeSlabs();
		/*
		returns reference to partial slab list
		*/
		std::vector<Slab>& GetPartialSlabs();
		/*
		returns reference to busy slab list
		*/
		std::vector<Slab>& GetBusySlabs();
	};

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::MoveFreeToPartialIfNeed()
	{
		if (partialSlabs.empty())
		{
			AllocateFreeIfNeed();
			partialSlabs.push_back(std::move(freeSlabs.back()));
			freeSlabs.pop_back();
		}
	}

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::AllocateFreeIfNeed()
	{
		if (freeSlabs.empty())
		{
			freeSlabs.resize((size_t)allocSize);
		}
	}

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::MovePartialToBusyIfNeed()
	{
		Slab& slab = partialSlabs.back();
		if (slab.GetSize() == slab.maxSize)
		{
			busySlabs.push_back(std::move(slab));
			partialSlabs.pop_back();
		}
	}

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::MoveBusyToPartial(SlabIt slabIt)
	{
		partialSlabs.push_back(std::move(*slabIt));
		busySlabs.erase(slabIt);
	}

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::MovePartialToFreeIfNeed(SlabIt slabIt)
	{
		Slab& slab = *slabIt;
		if (slab.GetSize() == 0)
		{
			freeSlabs.push_back(std::move(slab));
			partialSlabs.erase(slabIt);
		}
	}

	template<typename ElementT, typename IndexT>
	inline bool SlabAllocator<ElementT, IndexT>::FreeIfInBusy(ElementT* value)
	{
		for(auto it = busySlabs.begin(); it != busySlabs.end(); it++)
		{
			Slab& slab = *it;
			ElementT* begin = slab.GetNativePointer();
			ElementT* end = begin + slab.maxSize;
			if (InRange(begin, value, end))
			{
				slab.Free(value);
				MoveBusyToPartial(it);
				return true;
			}
		}
		return false;
	}

	template<typename ElementT, typename IndexT>
	inline bool SlabAllocator<ElementT, IndexT>::FreeIfInPartial(ElementT* value)
	{
		for (auto it = partialSlabs.begin(); it != partialSlabs.end(); it++)
		{
			Slab& slab = *it;
			ElementT* begin = slab.GetNativePointer();
			ElementT* end = begin + slab.maxSize;
			if (InRange(begin, value, end))
			{
				slab.Free(value);
				MovePartialToFreeIfNeed(it);
				return true;
			}
		}
		return false;
	}

	template<typename ElementT, typename IndexT>
	inline bool SlabAllocator<ElementT, IndexT>::InRange(ElementT* begin, ElementT* value, ElementT* end) const
	{
		return begin <= value && value < end;
	}

	template<typename ElementT, typename IndexT>
	inline SlabAllocator<ElementT, IndexT>::SlabAllocator(size_t freeAllocCount)
	{
		freeSlabs.resize(freeAllocCount);
		allocSize = freeAllocCount;
	}

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::Free(ElementT* value)
	{
		if (!FreeIfInPartial(value))
			FreeIfInBusy(value);
		allocCount--;
	}

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::ReallocateSlabs()
	{
		for (auto it = busySlabs.rbegin(); it != busySlabs.rend(); )
		{
			if (it->GetSize() != it->maxSize)
			{
				partialSlabs.push_back(std::move(*it));
				auto newIt = busySlabs.erase(--it.base());
				it = std::make_reverse_iterator(newIt);
			}
			else it++;
		}

		for (auto it = partialSlabs.rbegin(); it != partialSlabs.rend(); )
		{
			if (it->GetSize() == 0)
			{
				freeSlabs.push_back(std::move(*it));
				auto newIt = partialSlabs.erase(--it.base());
				it = std::make_reverse_iterator(newIt);
			}
			else it++;
		}
	}

	template<typename ElementT, typename IndexT>
	inline void SlabAllocator<ElementT, IndexT>::ReleaseFreeSlabs()
	{
		freeSlabs.clear();
		freeSlabs.shrink_to_fit();
	}

	template<typename ElementT, typename IndexT>
	inline uint64_t SlabAllocator<ElementT, IndexT>::GetAllocCount() const
	{
		return allocCount;
	}

	template<typename ElementT, typename IndexT>
	inline size_t SlabAllocator<ElementT, IndexT>::GetObjectSize() const
	{
		return sizeof(ElementT);
	}

	template<typename ElementT, typename IndexT>
	inline uint64_t SlabAllocator<ElementT, IndexT>::GetTotalMemory() const
	{
		return ((uint64_t)freeSlabs.capacity() + partialSlabs.capacity() + busySlabs.capacity()) *
				sizeof(Slab) * Slab::maxSize * (sizeof(ElementT) + sizeof(IndexT));
	}

	template<typename ElementT, typename IndexT>
	inline std::vector<typename SlabAllocator<ElementT, IndexT>::Slab>& SlabAllocator<ElementT, IndexT>::GetFreeSlabs()
	{
		return freeSlabs;
	}

	template<typename ElementT, typename IndexT>
	inline std::vector<typename SlabAllocator<ElementT, IndexT>::Slab>& SlabAllocator<ElementT, IndexT>::GetPartialSlabs()
	{
		return partialSlabs;
	}

	template<typename ElementT, typename IndexT>
	inline std::vector<typename SlabAllocator<ElementT, IndexT>::Slab>& SlabAllocator<ElementT, IndexT>::GetBusySlabs()
	{
		return busySlabs;
	}

	template<typename ElementT, typename IndexT>
	template<typename... Args>
	inline ElementT* SlabAllocator<ElementT, IndexT>::Alloc(Args&&... args)
	{
		allocCount++;
		MoveFreeToPartialIfNeed();
		Slab& slab = partialSlabs.back();
		ElementT* element = slab.Alloc(std::forward<Args>(args)...);
		MovePartialToBusyIfNeed();
		managedMemory += element->GetSize(); // MSL GC derived
		return element;
	}
}