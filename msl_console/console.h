#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define VM_ATTRIBUTES Stack* stack, AssemblyType* assembly, uint32_t* errors, Configuration* config, GarbageCollector* gc

using namespace MSL::VM;
using Stack = std::vector<MSL::VM::BaseObject*>;

DLLEXPORT Print    (VM_ATTRIBUTES);
DLLEXPORT PrintLine(VM_ATTRIBUTES);
DLLEXPORT Read     (VM_ATTRIBUTES);
DLLEXPORT ReadInt  (VM_ATTRIBUTES);
DLLEXPORT ReadFloat(VM_ATTRIBUTES);
DLLEXPORT ReadLine (VM_ATTRIBUTES);
DLLEXPORT ReadBool (VM_ATTRIBUTES);