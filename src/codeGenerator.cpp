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
			Write(OPCODE::ASSEMBLY_BEGIN_DECL);
			const auto& namespaces = assembly.GetNamespaces();
			Write(OPCODE::NAMESPACE_POOL_DECL_SIZE);
			Write(namespaces.size());
			for (const auto& _namespace : namespaces)
			{
				WriteString(_namespace.getName());
				Write(OPCODE::FRIEND_POOL_DECL_SIZE);
				Write(_namespace.friendNamespaces.size());
				for (const auto& friendNamespace : _namespace.friendNamespaces)
				{
					WriteString(friendNamespace);
				}
				GenerateClassPool(_namespace);
			}
			Write(OPCODE::ASSEMBLY_END_DECL);
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
			Write(OPCODE::CLASS_POOL_DECL_SIZE);
			Write(classes.size());
			for (const auto& _class : classes)
			{
				WriteString(_class.name);

				Write(OPCODE::MODIFIERS_DECL);
				Write(_class.modifiers);

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
			Write(OPCODE::ATTRIBUTE_POOL_DECL_SIZE);
			const auto& attributes = _class.GetAttributes();
			Write(attributes.size());
			for (const auto& attr : attributes)
			{
				WriteString(attr.name);
				Write(OPCODE::MODIFIERS_DECL);
				Write(attr.modifiers);
			}
		}

		void CodeGenerator::GenerateMethod(const Function& method)
		{
			Write(OPCODE::METHOD_PARAMS_DECL_SIZE);
			Write(method.params.size());
			for (const auto& param : method.params)
			{
				WriteString(param);
			}
			Write(OPCODE::DEPENDENCY_POOL_DECL_SIZE);
			const auto& variables = method.getVariables();
			Write(variables.size());
			for (const auto& variable : method.getVariables())
			{
				WriteString(variable);
			}
			Write(OPCODE::METHOD_BODY_BEGIN_DECL);
			method.GenerateBytecode(*this);
			Write(OPCODE::METHOD_BODY_END_DECL);
		}

		void CodeGenerator::GenerateMethodPool(const Class& _class)
		{
			Write(OPCODE::METHOD_POOL_DECL_SIZE);
			const auto& methods = _class.GetMethods();
			Write(methods.size());
			for (const auto& method : methods)
			{
				WriteString(method.name);
				Write(OPCODE::MODIFIERS_DECL);
				Write(method.modifiers);
				GenerateMethod(method);
			}
		}

		void CodeGenerator::WriteString(const std::string& data)
		{
			using StringSize = uint16_t;
			Write(OPCODE::STRING_DECL);
			StringSize stringSize = static_cast<StringSize>(data.size()); // size cannot be more than 0xFFFF !
			Write(stringSize);
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