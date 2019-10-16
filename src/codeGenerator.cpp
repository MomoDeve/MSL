#include "codeGenerator.h"

namespace MSL
{
	namespace compiler
	{
		using namespace VM;
		/*
		assembly structure:
			NAMESPACE_POOL_DECL_SIZE [namespace pool size] -> namespaces in assembly
			[namespace pool] -> look namespace structure
		*/

		/*
		namespace structure:
			STRING_DECL [string size] [string] -> namespace name
			CLASS_POOL_DECL_SIZE [class pool size] -> classes in namespace
			[class pool] -> look class structure
		*/
		void CodeGenerator::GenerateNamespacePool()
		{
			write(OPCODE::ASSEMBLY_BEGIN_DECL);
			const auto& namespaces = assembly.GetNamespaces();
			write(OPCODE::NAMESPACE_POOL_DECL_SIZE);
			write(namespaces.size());
			for (const auto& _namespace : namespaces)
			{
				writeString(_namespace.getName());
				write(OPCODE::FRIEND_POOL_DECL_SIZE);
				write(_namespace.friendNamespaces.size());
				for (const auto& friendNamespace : _namespace.friendNamespaces)
				{
					writeString(friendNamespace);
				}
				GenerateClassPool(_namespace);
			}
			write(OPCODE::ASSEMBLY_END_DECL);
		}

		/*
		class structure:
			STRING_DECL [string size] [string] -> class name
			MODIFIERS_DECL [modifiers] -> modifiers of class
			[attribute pool] -> look attribute structure
			[method pool] -> look method structure
		*/
		void CodeGenerator::GenerateClassPool(const Namespace& _namespace)
		{
			const auto& classes = _namespace.getMembers();
			write(OPCODE::CLASS_POOL_DECL_SIZE);
			write(classes.size());
			for (const auto& _class : classes)
			{
				writeString(_class.name);

				write(OPCODE::MODIFIERS_DECL);
				write(_class.modifiers);

				GenetateAttributePool(_class);
				GenerateMethodPool(_class);
			}
		}

		/*
		attribute structure:
			STRING_DECL [string size] [string] -> attribute name
			MODIFIERS_DECL [modifiers] -> modifiers of class
		*/
		void CodeGenerator::GenetateAttributePool(const Class& _class)
		{
			write(OPCODE::ATTRIBUTE_POOL_DECL_SIZE);
			const auto& attributes = _class.GetAttributes();
			write(attributes.size());
			for (const auto& attr : attributes)
			{
				writeString(attr.name);
				write(OPCODE::MODIFIERS_DECL);
				write(attr.modifiers);
			}
		}

		void CodeGenerator::GenerateMethod(const Function& method)
		{
			write(OPCODE::METHOD_PARAMS_DECL_SIZE);
			write(method.params.size());
			for (const auto& param : method.params)
			{
				writeString(param);
			}
			write(OPCODE::DEPENDENCY_POOL_DECL_SIZE);
			const auto& variables = method.getVariables();
			write(variables.size());
			for (const auto& variable : method.getVariables())
			{
				writeString(variable);
			}
			write(OPCODE::METHOD_BODY_BEGIN_DECL);
			write(OPCODE::PUSH_STACKFRAME);
			method.GenerateBytecode(*this);
			write(OPCODE::METHOD_BODY_END_DECL);
		}

		void CodeGenerator::GenerateMethodPool(const Class& _class)
		{
			write(OPCODE::METHOD_POOL_DECL_SIZE);
			const auto& methods = _class.GetMethods();
			write(methods.size());
			for (const auto& method : methods)
			{
				writeString(method.name);
				write(OPCODE::MODIFIERS_DECL);
				write(method.modifiers);
				GenerateMethod(method);
			}
		}

		void CodeGenerator::writeString(const std::string& data)
		{
			write(OPCODE::STRING_DECL);
			uint8_t stringSize = static_cast<uint8_t>(data.size()); // size cannot be more than 0xFF !
			write(stringSize);
			out.write(data.c_str(), stringSize);
		}

		CodeGenerator::CodeGenerator(const Assembly& assembly)
			: assembly(assembly) { }

		void CodeGenerator::GenerateBytecode()
		{
			GenerateNamespacePool();
		}

		std::string CodeGenerator::GetBuffer() const
		{
			return out.str();
		}
	}
}