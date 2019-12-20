#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"
#include <cmath>

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define PARAMS MSL::VM::VirtualMachine* vm

using namespace MSL::VM;

// System.Reflection
DLLEXPORT ReflectionGetType(PARAMS);
DLLEXPORT ReflectionGetNamespace(PARAMS);
DLLEXPORT ReflectionIsNamespaceExists(PARAMS);
DLLEXPORT ReflectionCreateInstance(PARAMS);
DLLEXPORT ReflectionInvoke(PARAMS);
DLLEXPORT ReflectionContainsMethod(PARAMS);
DLLEXPORT ReflectionGetMember(PARAMS);
DLLEXPORT ReflectionContainsMember(PARAMS);
// System.Math
DLLEXPORT MathSqrt(PARAMS);
DLLEXPORT MathAbs(PARAMS);
DLLEXPORT MathSin(PARAMS);
DLLEXPORT MathCos(PARAMS);
DLLEXPORT MathTan(PARAMS);
DLLEXPORT MathExp(PARAMS);
DLLEXPORT MathAtan(PARAMS);
DLLEXPORT MathAcos(PARAMS);
DLLEXPORT MathAsin(PARAMS);
DLLEXPORT MathLog10(PARAMS);
DLLEXPORT MathLog2(PARAMS);
DLLEXPORT MathLog(PARAMS);
// System.GC
DLLEXPORT GCCollect(PARAMS);
DLLEXPORT GCDisable(PARAMS);
DLLEXPORT GCEnable(PARAMS);
DLLEXPORT GCReleaseMemory(PARAMS);
DLLEXPORT GCSetMinimalMemory(PARAMS);
DLLEXPORT GCSetMaximalMemory(PARAMS);
DLLEXPORT GCSetLogPermissions(PARAMS);
// System.Console
DLLEXPORT ConsolePrint(PARAMS);
DLLEXPORT ConsolePrintLine(PARAMS);
DLLEXPORT ConsoleRead(PARAMS);
DLLEXPORT ConsoleReadInt(PARAMS);
DLLEXPORT ConsoleReadFloat(PARAMS);
DLLEXPORT ConsoleReadLine(PARAMS);
DLLEXPORT ConsoleReadBool(PARAMS);
// System.Array
DLLEXPORT ArrayConstructor(PARAMS);
DLLEXPORT ArrayGetByIndex(PARAMS);
DLLEXPORT ArraySize(PARAMS);
DLLEXPORT ArrayToString(PARAMS);
DLLEXPORT ArrayPop(PARAMS);
DLLEXPORT ArrayAppend(PARAMS);
DLLEXPORT ArraySort(PARAMS);