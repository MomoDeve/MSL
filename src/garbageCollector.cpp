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
}

void MSL::VM::GarbageCollector::SetLogStream(std::ostream* log)
{
	out = log;
}

void MSL::VM::GarbageCollector::SetInitCapacity(size_t allocSize)
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
	lastIter = std::chrono::system_clock::now();

	for (auto& ns : assembly.namespaces)
	{
		ns.second.wrapper->MarkMembers();
	}
	for (auto& call : callStack)
	{
		Frame* frame = call.GetFrame();
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

	managedObjects = clearedObjects = 0;

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

	auto endTimePoint = std::chrono::system_clock::now();
	auto elapsedTime = endTimePoint - lastIter;
	lastIter = endTimePoint;
	auto msTime = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

	if (out != nullptr)
	{
		*out << std::endl;
		*out << "------------------------------------------\n";
		*out << "[GC]: full garbage collection done in " << msTime << " ms\n";
		*out << "[GC]: collected total of " << clearedObjects << " objects\n";
		*out << "[GC]: still managing " << managedObjects << " objects\n";
		*out << "[GC]: objects allocated:\n";
		*out << "      since last iter: " << GetAllocSinceIter() << '\n';
		*out << "      since start: " << GetTotalAllocCount() << '\n';
		*out << "------------------------------------------\n";
	}
	allocSinceIter = GetTotalAllocCount();
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
}

std::chrono::milliseconds MSL::VM::GarbageCollector::GetTimeSinceLastIteration() const
{
	auto diff = std::chrono::system_clock::now() - lastIter;
	return std::chrono::duration_cast<std::chrono::milliseconds>(diff);
}

size_t MSL::VM::GarbageCollector::GetTotalAllocCount() const
{
	size_t total = 0;
	total += this->arrayAlloc->GetAllocCount();
	total += this->attributeAlloc->GetAllocCount();
	total += this->classObjAlloc->GetAllocCount();
	total += this->classWrapAlloc->GetAllocCount();
	total += this->floatAlloc->GetAllocCount();
	total += this->integerAlloc->GetAllocCount();
	total += this->localObjAlloc->GetAllocCount();
	total += this->nsWrapAlloc->GetAllocCount();
	total += this->stringAlloc->GetAllocCount();
	total += this->unknownObjAlloc->GetAllocCount();

	return total;
}

size_t MSL::VM::GarbageCollector::GetAllocSinceIter() const
{
	return GetTotalAllocCount() - allocSinceIter;
}

size_t MSL::VM::GarbageCollector::GetClearedObjectCount() const
{
	return clearedObjects;
}
