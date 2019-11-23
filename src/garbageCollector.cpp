#include "garbageCollector.h"
#include "stringExtensions.h"

MSL::VM::GarbageCollector::GarbageCollector(std::ostream* log, size_t allocSize)
{
	out = log;
	Init(this->attributeAlloc, allocSize);
	Init(this->classObjAlloc, allocSize);
	Init(this->classWrapAlloc, allocSize);
	Init(this->floatAlloc, allocSize);
	Init(this->integerAlloc, allocSize);
	Init(this->localObjAlloc, allocSize);
	Init(this->nsWrapAlloc, allocSize);
	Init(this->stringAlloc, allocSize);
	Init(this->unknownObjAlloc, allocSize);
	Init(this->arrayAlloc, allocSize);
	Init(this->frameAlloc, allocSize);
}

void MSL::VM::GarbageCollector::SetLogStream(std::ostream* log)
{
	out = log;
}

void MSL::VM::GarbageCollector::SetInitCapacity(uint64_t allocSize)
{
	Init(this->attributeAlloc, allocSize);
	Init(this->classObjAlloc, allocSize);
	Init(this->classWrapAlloc, allocSize);
	Init(this->floatAlloc, allocSize);
	Init(this->integerAlloc, allocSize);
	Init(this->localObjAlloc, allocSize);
	Init(this->nsWrapAlloc, allocSize);
	Init(this->stringAlloc, allocSize);
	Init(this->unknownObjAlloc, allocSize);
	Init(this->arrayAlloc, allocSize);
	Init(this->frameAlloc, allocSize);
}

void MSL::VM::GarbageCollector::Collect(AssemblyType& assembly, std::vector<CallPath>& callStack, std::vector<BaseObject*> objectStack)
{
	totalIters++;
	lastIter = std::chrono::system_clock::now();

	trueObject.MarkMembers();
	falseObject.MarkMembers();
	nullObject.MarkMembers();

	for (auto& ns : assembly.namespaces)
	{
		ns.second.wrapper->MarkMembers();
	}
	for (auto& call : callStack)
	{
		Frame* frame = call.GetFrame();
		frame->state = GCstate::MARKED;
		if(frame->classObject != nullptr) frame->classObject->MarkMembers();
		for (auto& local : frame->locals)
		{
			local.second.object->MarkMembers();
		}
	}
	for (BaseObject* object : objectStack)
	{
		object->MarkMembers();
	}

	managedObjects = 0;
	clearedObjects = 0;
	clearedMemory = 0;
	managedMemory = 0;

	ReleaseMemory();

	auto endTimePoint = std::chrono::system_clock::now();
	auto elapsedTime = endTimePoint - lastIter;
	lastIter = endTimePoint;
	auto msTime = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

	uint64_t totalMemory = GetTotalMemoryAlloc();

	if (out != nullptr)
	{
		*out << std::endl;
		*out << "------------------------------------------\n";
		*out << "[GC]: finished iteration #" << GetTotalIterations() << '\n';
		*out << "[GC]: full garbage collection done in " << msTime << " ms\n";
		*out << "[GC]: collected total of " << clearedObjects << " objects\n";
		*out << "[GC]: still managing " << managedObjects << " objects\n";
		*out << "[GC]: cleared memory: " << utils::formatBytes(clearedMemory) << '\n';
		*out << "[GC]: managed memory: " << utils::formatBytes(totalMemory) << '\n';
		*out << "------------------------------------------\n";
	}
	allocSinceIter = totalMemory;
}

void MSL::VM::GarbageCollector::ReleaseMemory()
{
	ClearSlabs(this->attributeAlloc);
	ClearSlabs(this->classObjAlloc);
	ClearSlabs(this->classWrapAlloc);
	ClearSlabs(this->floatAlloc);
	ClearSlabs(this->integerAlloc);
	ClearSlabs(this->localObjAlloc);
	ClearSlabs(this->nsWrapAlloc);
	ClearSlabs(this->stringAlloc);
	ClearSlabs(this->unknownObjAlloc);
	ClearSlabs(this->frameAlloc);
	ClearSlabs(this->arrayAlloc);
}

void MSL::VM::GarbageCollector::ReleaseFreeMemory()
{
	this->arrayAlloc->ReleaseFreeSlabs();
	this->attributeAlloc->ReleaseFreeSlabs();
	this->classObjAlloc->ReleaseFreeSlabs();
	this->classWrapAlloc->ReleaseFreeSlabs();
	this->floatAlloc->ReleaseFreeSlabs();
	this->frameAlloc->ReleaseFreeSlabs();
	this->integerAlloc->ReleaseFreeSlabs();
	this->localObjAlloc->ReleaseFreeSlabs();
	this->nsWrapAlloc->ReleaseFreeSlabs();
	this->stringAlloc->ReleaseFreeSlabs();
	this->unknownObjAlloc->ReleaseFreeSlabs();
}

std::chrono::milliseconds MSL::VM::GarbageCollector::GetTimeSinceLastIteration() const
{
	auto diff = std::chrono::system_clock::now() - lastIter;
	return std::chrono::duration_cast<std::chrono::milliseconds>(diff);
}

uint64_t MSL::VM::GarbageCollector::GetTotalMemoryAlloc() const
{
	uint64_t total = this->managedMemory;
	#define COUNT(x) total += x->allocCount * x->GetObjectSize() + x->managedMemory;
	COUNT(classObjAlloc);
	COUNT(classWrapAlloc);
	COUNT(nsWrapAlloc);
	COUNT(unknownObjAlloc);
	COUNT(integerAlloc);
	COUNT(floatAlloc);
	COUNT(stringAlloc);
	COUNT(localObjAlloc);
	COUNT(attributeAlloc);
	COUNT(arrayAlloc);
	COUNT(frameAlloc);
	#undef COUNT

	return total;
}

uint64_t MSL::VM::GarbageCollector::GetMemoryAllocSinceIter() const
{
	return GetTotalMemoryAlloc() - allocSinceIter;
}

uint64_t MSL::VM::GarbageCollector::GetClearedMemorySinceIter() const
{
	return clearedMemory;
}

uint64_t MSL::VM::GarbageCollector::GetClearedObjectCount() const
{
	return clearedObjects;
}

uint64_t MSL::VM::GarbageCollector::GetTotalIterations() const
{
	return totalIters;
}

void MSL::VM::GarbageCollector::PrintLog() const
{
	if (out != nullptr)
	{
		*out << std::endl;
		*out << "------------------------------------------\n";
		*out << "[GC]: iterations finished: " << GetTotalIterations() << '\n';
		*out << "[GC]: managed objects: " << managedObjects << " objects\n";
		*out << "[GC]: managed memory: " << utils::formatBytes(GetTotalMemoryAlloc()) << '\n';
		*out << "------------------------------------------\n";
	}
}
