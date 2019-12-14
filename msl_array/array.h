#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"

using namespace MSL::VM;

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define PARAMS MSL::VM::VirtualMachine* vm

DLLEXPORT ArrayConstructor(PARAMS);
DLLEXPORT GetByIndex(PARAMS);
DLLEXPORT Size(PARAMS);
DLLEXPORT ToString(PARAMS);
DLLEXPORT Pop(PARAMS);
DLLEXPORT Append(PARAMS);