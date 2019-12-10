#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>

#include "../src/virtualMachine.h"
#include "../src/objects.cpp"       
#include "../src/classType.cpp"     
#include "../src/attributeType.cpp" 
#include "../src/bigInteger.cpp"
#include "../src/garbageCollector.cpp"
#include "../src/callPath.cpp"

namespace MSL
{
	namespace utils
	{
		void InvokeError(std::ostream* out, const std::string& message, uint32_t* errors, size_t error);
		bool AssertType(MSL::VM::BaseObject* object, MSL::VM::Type type, uint32_t* errors, MSL::VM::Configuration* config);
		VM::BaseObject* GetUnderlyingObject(VM::BaseObject* object);
		const std::string* GetObjectName(const VM::BaseObject* object);
	}
}

namespace MSL
{
	namespace VM
	{
		ClassWrapper* AllocClassWrapper(GarbageCollector* GC, const ClassType* _class);
		ClassObject* AllocClassObject(GarbageCollector* GC, const ClassType* _class);
		NamespaceWrapper* AllocNamespaceWrapper(GarbageCollector* GC, const NamespaceType* _namespace);
		LocalObject* AllocLocal(GarbageCollector* GC, const std::string& localName, Local& local);
		UnknownObject* AllocUnknown(GarbageCollector* GC, const std::string* value);
		NullObject* AllocNull(GarbageCollector* GC);
		TrueObject* AllocTrue(GarbageCollector* GC);
		FalseObject* AllocFalse(GarbageCollector* GC);
		ArrayObject* AllocArray(GarbageCollector* GC, size_t size);
		StringObject* AllocString(GarbageCollector* GC, const std::string& value);
		IntegerObject* AllocInteger(GarbageCollector* GC, const std::string& value);
		FloatObject* AllocFloat(GarbageCollector* GC, const std::string& value);
	}
}