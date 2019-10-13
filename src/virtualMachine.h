#pragma once

#include <memory>
#include <istream>
#include <stack>
#include <chrono>
#include <iomanip>

#include "objects.h"
#include "configuration.h"
#include "callPath.h"
#include "assemblyEditor.h"
#include "assemblyType.h"

namespace MSL
{
	namespace VM
	{
		using LocalsTable = std::unordered_map<std::string, Local>;
		using LocalStorage = std::vector<std::unique_ptr<std::string>>;

		struct Frame
		{
			LocalsTable locals;
			LocalStorage localStorage;
			const NamespaceType* _namespace = nullptr;
			const ClassType* _class = nullptr;
			const MethodType* _method = nullptr;
			BaseObject* classObject = nullptr;
			size_t offset = 0;
		};

		class VirtualMachine
		{
			using CallStack = std::stack<CallPath>;
			using ObjectStack = std::vector<BaseObject*>;
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
			BaseObject* SearchForObject(const std::string& objectName, const LocalsTable& locals, const MethodType* _method, const BaseObject* _class, const NamespaceType* _namespace, bool checkError);
			ClassWrapper* SearchForClass(const std::string& objectName, const NamespaceType* _namespace);
			BaseObject* GetUnderlyingObject(BaseObject* object) const;
			void StartNewStackFrame();
			void InitializeStaticMembers();
			void AddSystemNamespace();
			bool ValidateHashValue(size_t hashValue, size_t maxHashValue);
			void InvokeObjectMethod(const std::string& methodName, const ClassObject* object);
			void DisplayError(std::string message) const;
			void DisplayExtra(std::string message) const;
			void PrintObjectStack() const;
			std::string GetFullClassType(const ClassType* type) const;
			std::string GetFullMethodType(const MethodType* type) const;
			std::string GetMethodActualName(const std::string& methodName) const;
			void PerformSystemCall(const ClassType* _class, const MethodType* _method, Frame* frame);
			void PerformALUCall(OPCODE op, size_t parameters, Frame* frame);
			void PerformALUcallIntegers(IntegerObject* int1, const IntegerObject::InnerType* int2, OPCODE op, Frame* frame);
			void PerformALUcallStrings(StringObject* str1, const StringObject::InnerType* str2, OPCODE op, Frame* frame);
			void PerformALUcallStringInteger(StringObject* str, const IntegerObject::InnerType* integer, OPCODE op, Frame* frame);
			void PerformALUcallFloats(FloatObject* f1, const FloatObject::InnerType* f2, OPCODE op, Frame* frame);
			void PerformALUcallClassTypes(ClassWrapper* class1, const ClassType* class2, OPCODE op, Frame* frame);
			void PerformALUcallClassObject(ClassObject* obj, OPCODE op, Frame* frame);
			void PerformALUcallBooleans(bool b1, bool b2, OPCODE op, Frame* frame);

			UnknownObject* AllocUnknown(const std::string* value);
			NullObject* AllocNull();
			TrueObject* AllocTrue();
			FalseObject* AllocFalse();
			ArrayObject* AllocArray(size_t size = 0);
			StringObject* AllocString(const std::string& value);
			IntegerObject* AllocInteger(const std::string& value);
			FloatObject* AllocFloat(const std::string& value);
			ClassWrapper* AllocClassWrapper(const ClassType* _class);
			ClassObject* AllocClassObject(const ClassType* _class);
			NamespaceWrapper* AllocNamespaceWrapper(const NamespaceType* _namespace);
			LocalObject* AllocLocal(const std::string& localName, Local& local);
		public:
			enum ERROR
			{
				CALLSTACK_EMPTY = 1,
				INVALID_CALL_ARGUMENT = 1 << 1,
				TERMINATE_ON_LAUNCH = 1 << 2,
				INVALID_OPCODE = 1 << 3,
				INVALID_STACKFRAME_OFFSET = 1 << 4,
				OBJECTSTACK_CORRUPTION = 1 << 5,
				INVALID_METHOD_SIGNATURE = 1 << 6,
				OBJECTSTACK_EMPTY = 1 << 7,
				INVALID_HASH_VALUE = 1 << 8,
				OBJECT_NOT_FOUND = 1 << 9,
				MEMBER_NOT_FOUND = 1 << 10,
				INVALID_STACKOBJECT = 1 << 11,
				STACKOVERFLOW = 1 << 12,
				PRIVATE_MEMBER_ACCESS = 1 << 13,
				CALLSTACK_CORRUPTION = 1 << 14,
				CONST_MEMBER_MODIFICATION = 1 << 15,
				ABSTRACT_MEMBER_CALL = 1 << 16,
				INVALID_METHOD_CALL = 1 << 17,
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
