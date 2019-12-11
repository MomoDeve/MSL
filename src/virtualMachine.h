#pragma once

#include "objects.h"
#include "configuration.h"
#include "callPath.h"
#include "assemblyEditor.h"
#include "assemblyType.h"
#include "garbageCollector.h"
#include "ExceptionTrace.h"

#define MSL_DLL_API
//#undef MSL_DLL_API

#ifdef MSL_DLL_API
#include "DllLoader.h"
#undef ERROR
#endif

namespace MSL
{
	namespace VM
	{
		class VirtualMachine
		{
			using CallStack = std::vector<CallPath>;
			using ObjectStack = std::vector<BaseObject*>;
			CallStack callStack;
			ObjectStack objectStack;
			GarbageCollector GC;

			#ifdef MSL_DLL_API
			DllLoader dllLoader;
			#endif

			ExceptionTrace exception;
			AssemblyType assembly;
			Configuration config;
			uint32_t errors;
			bool ALUinIncrMode;

			OPCODE ReadOPCode(const std::vector<uint8_t>& bytes, size_t& offset);
			uint16_t ReadLabel(const std::vector<uint8_t>& bytes, size_t& offset);
			size_t ReadHash(const std::vector<uint8_t>& bytes, size_t& offset);
			template<typename T> T GenericRead(const std::vector<uint8_t>& bytes, size_t& offset);

			const MethodType* GetMethodOrNull(const std::string& _namespace, const std::string& _class, const std::string& _method) const;
			const MethodType* GetMethodOrNull(const ClassType* _class, const std::string& _method) const;
			const ClassType* GetClassOrNull(const std::string& _namespace, const std::string& _class) const;
			const ClassType* GetClassOrNull(const NamespaceType* _namespace, const std::string& _class) const;
			const NamespaceType* GetNamespaceOrNull(const std::string& _namespace) const;
			BaseObject* ResolveReference(BaseObject* object, const Frame::LocalsTable& locals, const MethodType* _method, const BaseObject* _class, const NamespaceType* _namespace, bool checkError);
			ClassWrapper* SearchForClass(const std::string& objectName, const NamespaceType* _namespace);
			BaseObject* GetUnderlyingObject(BaseObject* object) const;
			const std::string* GetObjectName(const BaseObject* object) const;
			void InitializeStaticMembers();
			void AddSystemNamespace();
			void CollectGarbage(bool forceCollection = false);
			bool ValidateHashValue(size_t hashValue, size_t maxHashValue);
			bool AssertType(const BaseObject* object, Type type, const std::string& message, const Frame* frame = nullptr);
			bool LoadDll(const std::string& libName);
			inline bool AssertType(const BaseObject* object, Type type);
			void InvokeObjectMethod(const std::string& methodName, const ClassObject* object);
			void InitializeAttribute(ClassObject* object, const std::string& attribute, BaseObject* value);
			void PrintObjectStack() const;
			std::string OpcodeToMethod(OPCODE op) const;
			std::string ErrorToString(size_t error) const;
			std::string GetFullClassType(const ClassType* type) const;
			std::string GetFullMethodType(const MethodType* type) const;
			std::string GetMethodActualName(const std::string& methodName) const;
			void PerformSystemCall(const ClassType* _class, const MethodType* _method, Frame* frame);
			void PerformALUCall(OPCODE op, size_t parameters, Frame* frame);
			void PerformALUCallIntegers(IntegerObject* int1, const IntegerObject::InnerType* int2, OPCODE op, Frame* frame);
			void PerformALUcallStrings(StringObject* str1, const StringObject::InnerType* str2, OPCODE op, Frame* frame);
			void PerformALUcallStringInteger(StringObject* str, const IntegerObject::InnerType* integer, OPCODE op, Frame* frame);
			void PerformALUcallFloats(FloatObject* f1, const FloatObject::InnerType* f2, OPCODE op, Frame* frame);
			void PerformALUcallClassTypes(ClassWrapper* class1, const ClassType* class2, OPCODE op, Frame* frame);
			void PerformALUCallClassObject(ClassObject* obj, OPCODE op, Frame* frame);
			void PerformALUcallBooleans(bool b1, bool b2, OPCODE op, Frame* frame);

			Frame* AllocFrame();
			UnknownObject* AllocUnknown(const std::string* value);
			NullObject* AllocNull();
			TrueObject* AllocTrue();
			FalseObject* AllocFalse();
			ArrayObject* AllocArray(size_t size = 0);
			StringObject* AllocString(const std::string& value);
			IntegerObject* AllocInteger(const std::string& value);
			IntegerObject* AllocInteger(int64_t value);
			IntegerObject* AllocInteger(const IntegerObject::InnerType& value);
			FloatObject* AllocFloat(const std::string& value);
			FloatObject* AllocFloat(FloatObject::InnerType value);
			ClassWrapper* AllocClassWrapper(const ClassType* _class);
			ClassObject* AllocClassObject(const ClassType* _class);
			NamespaceWrapper* AllocNamespaceWrapper(const NamespaceType* _namespace);
			LocalObject* AllocLocal(const std::string& localName, Local& local);
		public:
			struct ERROR
			{
				enum : uint32_t
				{
					CALLSTACK_EMPTY = 1 << 0,
					OBJECTSTACK_EMPTY = 1 << 1,
					EXCEPTIONSTACK_EMPTY = 1 << 2,
					STACKOVERFLOW = 1 << 3,
					OUT_OF_MEMORY = 1 << 4,
					INVALID_BYTECODE = 1 << 5,
					INVALID_OPERATOR = 1 << 6,
					INVALID_ARGUMENT = 1 << 7,
					INVALID_TYPE = 1 << 8,
					INVALID_METHOD_CALL = 1 << 9,
					DLL_NOT_FOUND = 1 << 10,
					MEMBER_NOT_FOUND = 1 << 11,
					METHOD_NOT_FOUND = 1 << 12,
					PRIVATE_MEMBER_ACCESS = 1 << 13,
					ABSTRACT_MEMBER_ACCESS = 1 << 14,
					CONST_MEMBER_MODIFICATION = 1 << 15,
					AMBIGUOUS_TYPE = 1 << 16,

					TERMINATE_ON_LAUNCH = 1 << 30,
					FATAL_ERROR = 1u << 31,
				};
			};

			VirtualMachine(Configuration config);
			bool AddBytecodeFile(std::istream* binaryFile);
			void Run();
			std::vector<std::string> GetErrorStrings(uint32_t errors) const;

			#ifdef MSL_DLL_API
			public:
			#else
			private:
			#endif
			// methods for DLL API use
			AssemblyType& GetAssembly();
			CallStack& GetCallStack();
			ObjectStack& GetObjectStack();
			GarbageCollector& GetGC();
			uint32_t& GetErrors();
			Configuration& GetConfig();
			ExceptionTrace& GetException();
			void InvokeError(size_t error, const std::string& message, const std::string& arg);
			ClassWrapper* GetClassPrimitive(BaseObject* object);
			BaseObject* GetMemberObject(BaseObject* object, const std::string& memberName);
			void StartNewStackFrame();
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
