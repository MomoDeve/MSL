#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"

using namespace MSL::VM;
using Stack = std::vector<MSL::VM::BaseObject*>;

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define PARAMS MSL::VM::VirtualMachine* vm

DLLEXPORT Collect          (PARAMS);
DLLEXPORT Disable          (PARAMS);
DLLEXPORT Enable           (PARAMS);
DLLEXPORT ReleaseMemory    (PARAMS);
DLLEXPORT SetMinimalMemory (PARAMS);
DLLEXPORT SetMaximalMemory (PARAMS);
DLLEXPORT SetLogPermissions(PARAMS);