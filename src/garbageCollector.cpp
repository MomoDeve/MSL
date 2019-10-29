#include "garbageCollector.h"

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
}

void MSL::VM::GarbageCollector::Collect(AssemblyType& assembly, std::vector<CallPath>& callStack, std::vector<BaseObject*> objectStack)
{
	totalIters++;
	lastIter = std::chrono::system_clock::now();

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

	ClearSlabs(this->attributeAlloc);
	ClearSlabs(this->classObjAlloc);
	ClearSlabs(this->classWrapAlloc);
	ClearSlabs(this->floatAlloc);
	ClearSlabs(this->integerAlloc);
	ClearSlabs(this->localObjAlloc);
	ClearSlabs(this->nsWrapAlloc);
	ClearSlabs(this->stringAlloc);
	ClearSlabs(this->unknownObjAlloc);
	ClearSlabs(this->arrayAlloc);
	ClearSlabs(this->frameAlloc);

	auto endTimePoint = std::chrono::system_clock::now();
	auto elapsedTime = endTimePoint - lastIter;
	lastIter = endTimePoint;
	auto msTime = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

	if (out != nullptr)
	{
		*out << std::endl;
		*out << "------------------------------------------\n";
		*out << "[GC]: finished iteration #" << GetTotalIterations() << '\n';
		*out << "[GC]: full garbage collection done in " << msTime << " ms\n";
		*out << "[GC]: collected total of " << clearedObjects << " objects\n";
		*out << "[GC]: still managing " << managedObjects << " objects\n";
		*out << "[GC]: objects allocated:\n";
		*out << "      since last iter: " << GetMemoryAllocSinceIter() << '\n';
		*out << "      since start: " << GetTotalMemoryAlloc() << '\n';
		*out << "------------------------------------------\n";
	}
	allocSinceIter = GetTotalMemoryAlloc();
}

void MSL::VM::GarbageCollector::ReleaseMemory()
{
	classObjAlloc.reset();
	classWrapAlloc.reset();
	nsWrapAlloc.reset();
	unknownObjAlloc.reset();
	integerAlloc.reset();
	floatAlloc.reset();
	stringAlloc.reset();
	localObjAlloc.reset();
	attributeAlloc.reset();
	arrayAlloc.reset();
	frameAlloc.reset();
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
	uint64_t total = 0;
	#define COUNT(x) total += x->GetAllocCount() * x->GetObjectSize()
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

uint64_t MSL::VM::GarbageCollector::GetTotalMemoryUsage() const
{
	uint64_t total = 0;
	#define COUNT(x) total += x->GetTotalMemory()
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

	return total;
}
