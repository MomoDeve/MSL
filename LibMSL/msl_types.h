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