#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"
#include <cmath>

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define VM_ATTRIBUTES Stack* stack, AssemblyType* assembly, uint32_t* errors, Configuration* config, GarbageCollector* gc

using namespace MSL::VM;
using Stack = std::vector<MSL::VM::BaseObject*>;

DLLEXPORT FloatSqrt (VM_ATTRIBUTES);
DLLEXPORT FloatAbs  (VM_ATTRIBUTES);
DLLEXPORT FloatSin  (VM_ATTRIBUTES);
DLLEXPORT FloatCos  (VM_ATTRIBUTES);
DLLEXPORT FloatTan  (VM_ATTRIBUTES);
DLLEXPORT FloatExp  (VM_ATTRIBUTES);
DLLEXPORT FloatAtan (VM_ATTRIBUTES);
DLLEXPORT FloatAcos (VM_ATTRIBUTES);
DLLEXPORT FloatAsin (VM_ATTRIBUTES);
DLLEXPORT FloatLog10(VM_ATTRIBUTES);
DLLEXPORT FloatLog2 (VM_ATTRIBUTES);
DLLEXPORT FloatLog  (VM_ATTRIBUTES);
