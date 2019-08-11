#pragma once

#include <memory>
#include <istream>
#include <stack>

#include "configuration.h"
#include "callPath.h"
#include "assemblyEditor.h"
#include "assemblyType.h"

namespace MSL
{
	namespace VM
	{
		class VirtualMachine
		{
			std::stack<CallPath> callStack;
			AssemblyType assembly;
			Configuration config;
			uint32_t errors;

			NamespaceType* GetNamespace(AssemblyType* _assembly, const std::string& namespaceName);
			ClassType* GetClass(NamespaceType* _namespace, const std::string& className);
			MethodType* GetMethod(ClassType* _class, const std::string& methodName);
		public:
			enum ERRORS
			{
				EMPTY_CALL_STACK = 1,
				INVALID_CALL_ARGUMENT = 2,
			};
			VirtualMachine(Configuration config);
			bool AddBytecodeFile(std::istream* binaryFile);
			void Run();
			uint32_t GetErrors() const;
		};
	}
}
