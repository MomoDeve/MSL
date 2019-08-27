#pragma once

#include <memory>
#include <istream>
#include <stack>
#include <chrono>

#include "objects.h"
#include "configuration.h"
#include "callPath.h"
#include "assemblyEditor.h"
#include "assemblyType.h"

namespace MSL
{
	namespace VM
	{
		struct Local
		{
			BaseObject* object = nullptr;
			bool isConst = false;
		};

		class VirtualMachine
		{
			using CallStack = std::stack<CallPath>;
			using ObjectStack = std::vector<BaseObject*>;
			using LocalsTable = std::unordered_map<std::string, Local>;
			CallStack callStack;
			ObjectStack objectStack;

			AssemblyType assembly;
			Configuration config;
			uint32_t errors;
			bool ALUinIncrMode;

			NullObject nullObject;
			TrueObject trueObject;
			FalseObject falseObject;

			OPCODE ReadOPCode(const std::vector<uint8_t>& bytes, size_t& offset);
			uint16_t ReadLabel(const std::vector<uint8_t>& bytes, size_t& offset);
			size_t ReadHash(const std::vector<uint8_t>& bytes, size_t& offset);
			template<typename T> T GenericRead(const std::vector<uint8_t>& bytes, size_t& offset);

			const MethodType* GetMethodOrNull(const std::string& _namespace, const std::string& _class, const std::string& _method) const;
			const MethodType* GetMethodOrNull(const ClassType* _class, const std::string& _method) const;
			const ClassType* GetClassOrNull(const std::string& _namespace, const std::string& _class) const;
			const ClassType* GetClassOrNull(const NamespaceType* _namespace, const std::string& _class) const;
			const NamespaceType* GetNamespaceOrNull(const std::string& _namespace) const;
			BaseObject* SearchForObject(const std::string& objectName, const LocalsTable& locals, const MethodType* _method, const BaseObject* _class, const NamespaceType* _namespace);
			void StartNewStackFrame();
			void PerformSystemCall(const ClassType* _class, const MethodType* _method);
			void InitializeStaticMembers();
			void AddSystemNamespace();
			bool ValidateHashValue(size_t hashValue, size_t maxHashValue);
			void InvokeObjectMethod(const std::string& methodName, const ClassObject* object);
			void DisplayError(std::string message) const;
			void DisplayExtra(std::string message) const;
			std::string GetFullClassType(const ClassType* type) const;
			std::string GetFullMethodType(const MethodType* type) const;
			std::string GetMethodActualName(const std::string& methodName) const;

			BaseObject* AllocUnknown(const std::string* value);
			BaseObject* AllocNull();
			BaseObject* AllocTrue();
			BaseObject* AllocFalse();
			BaseObject* AllocString(const std::string& value);
			BaseObject* AllocInteger(const std::string& value);
			BaseObject* AllocFloat(const std::string& value);
			BaseObject* AllocClassWrapper(const ClassType* _class);
			BaseObject* AllocClassObject(const ClassType* _class);
			BaseObject* AllocNamespaceWrapper(const NamespaceType* _namespace);
			BaseObject* AllocLocal(const std::string& localName, Local& local);
		public:
			enum ERROR
			{
				CALLSTACK_EMPTY = 1,
				INVALID_CALL_ARGUMENT = 2,
				TERMINATE_ON_LAUNCH = 4,
				INVALID_OPCODE = 8,
				INVALID_STACKFRAME_OFFSET = 16,
				OBJECTSTACK_CORRUPTION = 32,
				INVALID_METHOD_SIGNATURE = 64,
				OBJECTSTACK_EMPTY = 128,
				INVALID_HASH_VALUE = 256,
				OBJECT_NOT_FOUND = 512,
				MEMBER_NOT_FOUND = 1024,
				INVALID_STACKOBJECT = 2048,
				STACKOVERFLOW = 4096,
				PRIVATE_MEMBER_ACCESS = 8192,
				CALLSTACK_CORRUPTION = 16384,
				CONST_MEMBER_MODIFICATION = 32768,
			};
			VirtualMachine(Configuration config);
			bool AddBytecodeFile(std::istream* binaryFile);
			void Run();
			uint32_t GetErrors() const;
		};

		template<typename T>
		T VirtualMachine::GenericRead(const std::vector<uint8_t>& bytes, size_t& offset)
		{
			const T* result = reinterpret_cast<const T*>(&(bytes[offset]));
			offset += sizeof(T);
			return *result;
		}
	}
}
