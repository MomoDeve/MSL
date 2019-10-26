#pragma once

#include <array>
#include <limits>
#include <algorithm>
#include <vector>

namespace momo
{
	#define MEMORY_CHECK

	template<typename ElementT, typename IndexT>
	class Slab
	{
	public:
		static constexpr size_t maxSize = 1 << (sizeof(IndexT) * 8);
	private:
		#ifdef MEMORY_CHECK
		std::vector<bool> isFree;
		#endif 
		ElementT* memory;
		IndexT* freeTable;
		IndexT curFree;
		size_t size;
	public:
		Slab();
		Slab(const Slab&) = delete;
		Slab(Slab&& slab);
		Slab& operator=(Slab&& slab);
		~Slab();
		template<typename... Args> ElementT* Alloc(Args&&... args);
		void Free(ElementT* value);
		size_t GetSize() const;
		ElementT* GetNativePointer();
	};

	template<typename ElementT, typename IndexT>
	inline Slab<ElementT, IndexT>::Slab()
		: size(0), curFree(0)
	{
		
		memory = (ElementT*)malloc(maxSize * sizeof(ElementT));
		memset(memory, 0, maxSize * sizeof(ElementT));
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
	ElementT* Slab<ElementT, IndexT>::GetNativePointer()
	{
		return memory;
	}

	template<typename ElementT, typename IndexT = uint8_t>
	class SlabAllocator
	{
		using Slab = Slab<ElementT, IndexT>;
		using SlabIt = typename std::vector<Slab>::iterator;
		std::vector<Slab> busySlabs, partialSlabs, freeSlabs;
		size_t allocSize;
		size_t allocCount = 0;
		void MoveFreeToPartialIfNeed();
		void AllocateFreeIfNeed();
		void MovePartialToBusyIfNeed();
		void MoveBusyToPartial(SlabIt slabIt);
		void MovePartialToFreeIfNeed(SlabIt slabIt);
		bool FreeIfInBusy(ElementT* value);
		bool FreeIfInPartial(ElementT* value);
		bool InRange(ElementT* begin, ElementT* value, ElementT* end) const;
	public:
		SlabAllocator(size_t freeAllocCount);
		
		template<typename... Args> ElementT* Alloc(Args&&... args);
		void Free(ElementT* value);
		void ReallocateSlabs();
		size_t GetAllocCount() const;

		std::vector<Slab>& GetFreeSlabs();
		std::vector<Slab>& GetPartialSlabs();
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
			freeSlabs.resize(allocSize);
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
			FreeIfInBusy();
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

		if (freeSlabs.size() > 2 * busySlabs.size())
		{
			freeSlabs.clear();
		}
	}

	template<typename ElementT, typename IndexT>
	inline size_t SlabAllocator<ElementT, IndexT>::GetAllocCount() const
	{
		return allocCount;
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
		return element;
	}
}