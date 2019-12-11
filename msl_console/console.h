#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define PARAMS MSL::VM::VirtualMachine* vm

using namespace MSL::VM;

DLLEXPORT Print    (PARAMS);
DLLEXPORT PrintLine(PARAMS);
DLLEXPORT Read     (PARAMS);
DLLEXPORT ReadInt  (PARAMS);
DLLEXPORT ReadFloat(PARAMS);
DLLEXPORT ReadLine (PARAMS);
DLLEXPORT ReadBool (PARAMS);