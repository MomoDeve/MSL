#pragma once

#include <istream>
#include <vector>
#include <string>

#include "callPath.h"
#include "assemblyType.h"
#include "opcode.h"
#include "stringExtensions.h"

namespace MSL
{
	namespace VM
	{
		class AssemblyEditor
		{
			std::istream& file;
			std::ostream& error;
			CallPath* entryPoint = nullptr;
			bool success = true;
			bool performCheck = true;
			uint8_t errors = 0;

			OPCODE ReadOPCode();
			size_t ReadSize();
			uint8_t ReadModifiers();
			uint16_t ReadLabel();
			std::string ReadString();
			AssemblyType ReadAssembly();
			NamespaceType ReadNamespace();
			ClassType ReadClass();
			AttributeType ReadAttribute();
			MethodType ReadMethod();
			void MergeNamespaces(NamespaceType& ns1, NamespaceType& ns2);
			void RegisterLabelInMethod(MethodType& method, uint16_t label);
			template<typename T> T GenericRead();
			template<typename T> void AddIntegerToByteArray(std::vector<uint8_t>& bytes, T integer);
			template<typename T> void ReserveExtraSpace(T& container, size_t additionalSpace);
			void DisplayError(std::string message);
			bool ExpectOpcode(OPCODE expected, OPCODE current);
		public:
			enum ERROR
			{
				INVALID_OPCODE = 1,
				DECLARATION_DUBLICATE = 2,
				INVALID_METHOD_LABEL = 4,
				ENTRY_POINT_DUBLICATE = 8,
			};

			AssemblyEditor(std::istream* binaryFile, std::ostream* errorStream);
			bool MergeAssemblies(AssemblyType& assembly, bool checkErrors, CallPath* callPath);
			uint8_t GetErrors() const;
		};

		template<typename T>
		T AssemblyEditor::GenericRead()
		{
			T variable;
			file.read(reinterpret_cast<char*>(&variable), sizeof(variable));
			return variable;
		}

		template<typename T>
		void AssemblyEditor::AddIntegerToByteArray(std::vector<uint8_t>& bytes, T integer)
		{
			size_t bytesSize = bytes.size();
			bytes.resize(bytesSize + sizeof(T));
			T* ptr = reinterpret_cast<T*>(&(bytes[0]) + bytesSize);
			*ptr = integer;
		}

		template<typename T>
		void AssemblyEditor::ReserveExtraSpace(T& container, size_t additionalSpace)
		{
			size_t newSize = container.size() + additionalSpace;
			container.reserve(2 * newSize);
		}
	}
}