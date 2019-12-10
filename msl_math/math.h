#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"
#include <cmath>

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define PARAMS MSL::VM::VirtualMachine* vm

using namespace MSL::VM;
using Stack = std::vector<MSL::VM::BaseObject*>;

DLLEXPORT FloatSqrt (PARAMS);
DLLEXPORT FloatAbs  (PARAMS);
DLLEXPORT FloatSin  (PARAMS);
DLLEXPORT FloatCos  (PARAMS);
DLLEXPORT FloatTan  (PARAMS);
DLLEXPORT FloatExp  (PARAMS);
DLLEXPORT FloatAtan (PARAMS);
DLLEXPORT FloatAcos (PARAMS);
DLLEXPORT FloatAsin (PARAMS);
DLLEXPORT FloatLog10(PARAMS);
DLLEXPORT FloatLog2 (PARAMS);
DLLEXPORT FloatLog  (PARAMS);