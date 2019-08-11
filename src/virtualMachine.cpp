#include "virtualMachine.h"

namespace MSL
{
	namespace VM
	{
		NamespaceType* VirtualMachine::GetNamespace(AssemblyType* _assembly, const std::string& namespaceName)
		{
			auto it = _assembly->namespaces.find(namespaceName);
			if (it == _assembly->namespaces.end()) return nullptr;
			return &(it->second);
		}

		ClassType * VirtualMachine::GetClass(NamespaceType* _namespace, const std::string& className)
		{
			auto it = _namespace->classes.find(className);
			if (it == _namespace->classes.end()) return nullptr;
			return &(it->second);
		}

		MethodType* VirtualMachine::GetMethod(ClassType* _class, const std::string& methodName)
		{
			auto it = _class->methods.find(methodName);
			if (it == _class->methods.end()) return nullptr;
			return &(it->second);
		}

		VirtualMachine::VirtualMachine(Configuration config)
			: config(std::move(config)), errors(0) { }

		bool VirtualMachine::AddBytecodeFile(std::istream* binaryFile)
		{
			if (!assembly.namespaces.empty() && !config.compilation.allowAssemblyMerge)
			{
				return false;
			}
			else
			{
				AssemblyEditor editor(binaryFile, config.streams.error);
				CallPath* callPath = nullptr;
				if (callStack.empty())
				{
					callStack.push(CallPath());
					callPath = &callStack.top();
				}
				return editor.MergeAssemblies(
					assembly,
					config.compilation.varifyBytecode,
					config.compilation.allowMemoryPreallocation,
					callPath
				);
			}
		}

		void VirtualMachine::Run()
		{
			if (callStack.empty())
			{
				errors |= ERRORS::EMPTY_CALL_STACK;
				return;
			}
			NamespaceType* ns = GetNamespace(&assembly, callStack.top().path[CallPath::NAMESPACE]);
			if (ns == nullptr)
			{
				errors |= ERRORS::INVALID_CALL_ARGUMENT;
				return;
			}
			ClassType* c = GetClass(ns, callStack.top().path[CallPath::CLASS]);
			if (c == nullptr)
			{
				errors |= ERRORS::INVALID_CALL_ARGUMENT;
				return;
			}
			MethodType* method = GetMethod(c, callStack.top().path[CallPath::METHOD]);
			if (method == nullptr)
			{
				errors |= ERRORS::INVALID_CALL_ARGUMENT;
				return;
			}
		}

		uint32_t VirtualMachine::GetErrors() const
		{
			return errors;
		}
	}
}