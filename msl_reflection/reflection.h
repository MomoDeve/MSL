#pragma once

#include "../LibMSL/msl_types.h"
#include "../LibMSL/msl_types.cpp"
#include <cmath>

#define DLLEXPORT extern "C" __declspec(dllexport) void _cdecl
#define PARAMS MSL::VM::VirtualMachine* vm

using namespace MSL::VM;

DLLEXPORT GetType          (PARAMS);
DLLEXPORT GetNamespace     (PARAMS);
DLLEXPORT IsNamespaceExists(PARAMS);
DLLEXPORT CreateInstance   (PARAMS);
DLLEXPORT Invoke           (PARAMS);
DLLEXPORT ContainsMethod   (PARAMS);
DLLEXPORT GetMember        (PARAMS);
DLLEXPORT ContainsMember   (PARAMS);