#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>

#include "../src/virtualMachine.h"
#include "../src/DllLoader.cpp"
#include "../src/ExceptionTrace.cpp"
#include "../src/assemblyEditor.cpp"
#include "../src/stringExtensions.cpp"
#include "../src/opcode.cpp"
#include "../src/virtualMachine.cpp"
#include "../src/objects.cpp"       
#include "../src/classType.cpp"    
#include "../src/methodType.cpp"
#include "../src/attributeType.cpp" 
#include "../src/bigInteger.cpp"
#include "../src/garbageCollector.cpp"
#include "../src/callPath.cpp"

namespace MSL
{
	namespace utils
	{
		bool AssertType(MSL::VM::VirtualMachine* vm, MSL::VM::BaseObject* object, MSL::VM::Type type);
		VM::BaseObject* GetUnderlyingObject(VM::BaseObject* object);
		const std::string* GetObjectName(const VM::BaseObject* object);
	}
}

namespace MSL
{
	namespace VM
	{
		ClassWrapper* AllocClassWrapper(GarbageCollector& GC, const ClassType* _class);
		ClassObject* AllocClassObject(GarbageCollector& GC, const ClassType* _class);
		NamespaceWrapper* AllocNamespaceWrapper(GarbageCollector& GC, const NamespaceType* _namespace);
		LocalObject* AllocLocal(GarbageCollector& GC, const std::string& localName, Local& local);
		UnknownObject* AllocUnknown(GarbageCollector& GC, const std::string* value);
		NullObject* AllocNull(GarbageCollector& GC);
		TrueObject* AllocTrue(GarbageCollector& GC);
		FalseObject* AllocFalse(GarbageCollector& GC);
		ArrayObject* AllocArray(GarbageCollector& GC, size_t size);
		StringObject* AllocString(GarbageCollector& GC, const std::string& value);
		IntegerObject* AllocInteger(GarbageCollector& GC, const std::string& value);
		IntegerObject* AllocInteger(GarbageCollector& GC, size_t value);
		IntegerObject* AllocInteger(GarbageCollector& GC, IntegerObject::InnerType& value);
		FloatObject* AllocFloat(GarbageCollector& GC, const std::string& value);
	}
}