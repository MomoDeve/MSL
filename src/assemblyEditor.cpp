#include "assemblyEditor.h"

namespace MSL
{
	namespace VM
	{
		void AssemblyEditor::DisplayError(std::string message)
		{
			error << "[assembly editor]: " << message << '\n';
			success = false;
		}

		bool AssemblyEditor::ExpectOpcode(OPCODE expected, OPCODE current)
		{
			if (!performCheck) return true;

			if (expected != current)
			{
				DisplayError("Expected opcode: " + ToString(expected) + ", found: " + ToString(current));
				errors |= ERROR::INVALID_OPCODE;
				return false;
			}
			return true;
		}

		AssemblyEditor::AssemblyEditor(std::istream* binaryFile, std::ostream* errorStream)
			: file(*binaryFile), error(*errorStream), success(true) { }

		bool AssemblyEditor::MergeAssemblies(AssemblyType& assembly, bool checkErrors, bool allowExtraAlloc, CallPath* callPath)
		{
			performCheck = checkErrors;
			extraAlloc = allowExtraAlloc;
			entryPoint = callPath;
			AssemblyType secondAssembly = ReadAssembly();

			if (performCheck)
			{
				for (const auto& ns : secondAssembly.namespaces)
				{
					if (assembly.namespaces.find(ns.second.name) != assembly.namespaces.end())
					{
						DisplayError("Trying to add namespace dublicate: " + ns.second.name);
						errors |= ERROR::DECLARATION_DUBLICATE;
						success = false;
						break;
					}
				}
			}
			std::string namespaceEntry;
			if (callPath != nullptr && callPath->GetNamespace() != nullptr)
			{
				namespaceEntry = *callPath->GetNamespace();
			}
			if (extraAlloc)
			{
				ReserveExtraSpace(assembly.namespaces, secondAssembly.namespaces.size());
			}
			if (success)
			{
				for (auto it = secondAssembly.namespaces.begin(); it != secondAssembly.namespaces.end(); it++)
				{
					assembly.namespaces.insert(std::move(*it));
				}
			}
			if (!namespaceEntry.empty())
			{
				callPath->SetNamespace(&assembly.namespaces[namespaceEntry].name);
			}
			return success;
		}

		uint8_t AssemblyEditor::GetErrors() const
		{
			return errors;
		}

		AssemblyType AssemblyEditor::ReadAssembly()
		{
			AssemblyType assembly;
			if (!ExpectOpcode(OPCODE::ASSEMBLY_BEGIN_DECL, ReadOPCode())) return assembly;
			if (!ExpectOpcode(OPCODE::NAMESPACE_POOL_DECL_SIZE, ReadOPCode())) return assembly;
			size_t namespacePoolSize = ReadSize();
			if (extraAlloc) ReserveExtraSpace(assembly.namespaces, namespacePoolSize);
			
			for (size_t i = 0; i < namespacePoolSize; i++)
			{
				NamespaceType ns = ReadNamespace();
				if (!success) return assembly;

				if (performCheck && assembly.namespaces.find(ns.name) != assembly.namespaces.end())
				{
					DisplayError("Trying to add namespace dublicate: " + ns.name);
					errors |= ERROR::DECLARATION_DUBLICATE;
					return assembly;
				}
				std::string namespaceName = ns.name;
				assembly.namespaces.insert({ namespaceName, std::move(ns) });
				if (entryPoint != nullptr && entryPoint->GetNamespace() == nullptr && entryPoint->GetClass() != nullptr)
				{
					entryPoint->SetNamespace(&assembly.namespaces[namespaceName].name);
				}
			}
			if (!ExpectOpcode(OPCODE::ASSEMBLY_END_DECL, ReadOPCode())) return assembly;
			return assembly;
		}

		NamespaceType AssemblyEditor::ReadNamespace()
		{
			NamespaceType ns;
			if (!ExpectOpcode(OPCODE::STRING_DECL, ReadOPCode())) return ns;
			ns.name = ReadString();
			if (!ExpectOpcode(OPCODE::CLASS_POOL_DECL_SIZE, ReadOPCode())) return ns;
			size_t classPoolSize = ReadSize();
			if (extraAlloc) ReserveExtraSpace(ns.classes, classPoolSize);

			for (size_t i = 0; i < classPoolSize; i++)
			{
				ClassType c = ReadClass();
				if (!success) return ns;

				if (performCheck && ns.classes.find(c.name) != ns.classes.end())
				{
					DisplayError("Trying to add class dublicate: " + c.name);
					errors |= ERROR::DECLARATION_DUBLICATE;
					return ns;
				}
				std::string className = c.name;
				c.namespaceName = ns.name;
				ns.classes.insert({ className, std::move(c) });
				if (entryPoint != nullptr && entryPoint->GetClass() == nullptr && entryPoint->GetMethod() != nullptr)
				{
					entryPoint->SetClass(&ns.classes[className].name);
				}
			}
			return ns;
		}

		ClassType AssemblyEditor::ReadClass()
		{
			ClassType c;
			if (!ExpectOpcode(OPCODE::STRING_DECL, ReadOPCode())) return c;
			c.name = ReadString();
			if (!ExpectOpcode(OPCODE::MODIFIERS_DECL, ReadOPCode())) return c;
			c.modifiers = ReadModifiers();
			if (!ExpectOpcode(OPCODE::ATTRIBUTE_POOL_DECL_SIZE, ReadOPCode())) return c;
			size_t attributePoolSize = ReadSize();

			for (size_t i = 0; i < attributePoolSize; i++)
			{
				AttributeType attr = ReadAttribute();
				if (!success) return c;

				if (performCheck && (c.staticAttributes.find(attr.name) != c.staticAttributes.end() || 
									 c.objectAttributes.find(attr.name) != c.objectAttributes.end()))
				{
					DisplayError("Trying to add attribute dublicate: " + attr.name);
					errors |= ERROR::DECLARATION_DUBLICATE;
					return c;
				}
				std::string attributeName = attr.name;
				if (attr.modifiers & AttributeType::Modifiers::STATIC)
				{
					c.staticAttributes.insert({ attributeName, std::move(attr) });
				}
				else
				{
					c.objectAttributes.insert({ attributeName, std::move(attr) });
				}
			}

			if (!ExpectOpcode(OPCODE::METHOD_POOL_DECL_SIZE, ReadOPCode())) return c;
			size_t methodPoolSize = ReadSize();
			if (extraAlloc) ReserveExtraSpace(c.methods, methodPoolSize);

			for (size_t i = 0; i < methodPoolSize; i++)
			{
				MethodType method = ReadMethod();
				method.body.shrink_to_fit(); // optimizing memory usage
				if (!success) return c;

				if (performCheck)
				{
					for (size_t i = 0; i < method.labels.size(); i++)
					{
						size_t offset = method.labels[i];
						if (offset >= method.body.size())
						{
							DisplayError("Found invalid label offset (label #" + std::to_string(i) + "): " + std::to_string(offset));
							errors |= ERROR::INVALID_METHOD_LABEL;
							return c;
						}
					}
				}
				if (method.modifiers & MethodType::Modifiers::STATIC_CONSTRUCTOR)
				{
					method.name += "_0static";
				}
				else
				{
					method.name += '_' + std::to_string(method.parameters.size()); // unique name for overloading
				}
				std::string methodName = method.name;
				c.methods.insert({ methodName, std::move(method) });
				if (entryPoint != nullptr && (method.modifiers & MethodType::Modifiers::ENTRY_POINT))
				{
					if (entryPoint->GetMethod() != nullptr)
					{
						DisplayError("Trying to add second entry-point to assembly: " + method.name);
						errors |= ERROR::ENTRY_POINT_DUBLICATE;
					}
					else
					{
						entryPoint->SetMethod(&c.methods[methodName].name);
					}
				}
			}
			return c;
		}

		AttributeType AssemblyEditor::ReadAttribute()
		{
			AttributeType attr;
			if (!ExpectOpcode(OPCODE::STRING_DECL, ReadOPCode())) return attr;
			attr.name = ReadString();
			if (!ExpectOpcode(OPCODE::MODIFIERS_DECL, ReadOPCode())) return attr;
			attr.modifiers = ReadModifiers();
			return attr;
		}

		MethodType AssemblyEditor::ReadMethod()
		{
			MethodType method;
			if (!ExpectOpcode(OPCODE::STRING_DECL, ReadOPCode())) return method;
			method.name = ReadString();
			if (!ExpectOpcode(OPCODE::MODIFIERS_DECL, ReadOPCode())) return method;
			method.modifiers = ReadModifiers();
			if (!ExpectOpcode(OPCODE::METHOD_PARAMS_DECL_SIZE, ReadOPCode())) return method;
			size_t parameterArraySize = ReadSize();
			method.parameters.reserve(parameterArraySize);

			for (size_t i = 0; i < parameterArraySize; i++)
			{
				if (!ExpectOpcode(OPCODE::STRING_DECL, ReadOPCode())) return method;
				method.parameters.push_back(ReadString());
			}

			if (!ExpectOpcode(OPCODE::DEPENDENCY_POOL_DECL_SIZE, ReadOPCode())) return method;
			size_t dependencyPoolSize = ReadSize();
			method.dependencies.reserve(dependencyPoolSize);

			for (size_t i = 0; i < dependencyPoolSize; i++)
			{
				if (!ExpectOpcode(OPCODE::STRING_DECL, ReadOPCode())) return method;
				method.dependencies.push_back(ReadString());
			}

			for (size_t i = 0; i < method.dependencies.size(); i++)
			{
				method.dependencies[i] = replaceEscapeTokens(method.dependencies[i]);
			}

			if (!ExpectOpcode(OPCODE::METHOD_BODY_BEGIN_DECL, ReadOPCode())) return method;
			OPCODE op = OPCODE::METHOD_BODY_BEGIN_DECL;
			if (extraAlloc) ReserveExtraSpace(method.body, dependencyPoolSize);

			while (op != OPCODE::ERROR_SYMBOL)
			{
				#define WRITE_OPCODE(op) method.body.push_back(op)
				#define WRITE_HASH AddIntegerToByteArray(method.body, ReadSize())
				#define WRITE_LABEL AddIntegerToByteArray(method.body, ReadLabel());
				op = ReadOPCode();
				switch (op)
				{
				case (OPCODE::PUSH_STRING):
					WRITE_OPCODE(OPCODE::PUSH_STRING);
					WRITE_HASH;
					break;
				case (OPCODE::PUSH_INTEGER):
					WRITE_OPCODE(OPCODE::PUSH_INTEGER);
					WRITE_HASH;
					break;
				case (OPCODE::PUSH_FLOAT):
					WRITE_OPCODE(OPCODE::PUSH_FLOAT);
					WRITE_HASH;
					break;
				case (OPCODE::PUSH_OBJECT):
					WRITE_OPCODE(OPCODE::PUSH_OBJECT);
					WRITE_HASH;
					break;
				case (OPCODE::PUSH_THIS):
					WRITE_OPCODE(OPCODE::PUSH_THIS);
					break;
				case (OPCODE::PUSH_NULL):
					WRITE_OPCODE(OPCODE::PUSH_NULL);
					break;
				case (OPCODE::PUSH_TRUE):
					WRITE_OPCODE(OPCODE::PUSH_TRUE);
					break;
				case (OPCODE::PUSH_FALSE):
					WRITE_OPCODE(OPCODE::PUSH_FALSE);
					break;
				case (OPCODE::POP_TO_RETURN):
					WRITE_OPCODE(OPCODE::POP_TO_RETURN);
					break;
				case (OPCODE::ALLOC_VAR):
					WRITE_OPCODE(OPCODE::ALLOC_VAR);
					WRITE_HASH;
					break;
				case (OPCODE::ALLOC_CONST_VAR):
					WRITE_OPCODE(OPCODE::ALLOC_CONST_VAR);
					WRITE_HASH;
					break;
				case (OPCODE::NEGATION_OP):
					WRITE_OPCODE(OPCODE::NEGATION_OP);
					break;
				case (OPCODE::NEGATIVE_OP):
					WRITE_OPCODE(OPCODE::NEGATIVE_OP);
					break;
				case (OPCODE::POSITIVE_OP):
					WRITE_OPCODE(OPCODE::POSITIVE_OP);
					break;
				case (OPCODE::SUM_OP):
					WRITE_OPCODE(OPCODE::SUM_OP);
					break;
				case (OPCODE::SUB_OP):
					WRITE_OPCODE(OPCODE::SUB_OP);
					break;
				case (OPCODE::MULT_OP):
					WRITE_OPCODE(OPCODE::MULT_OP);
					break;
				case (OPCODE::DIV_OP):
					WRITE_OPCODE(OPCODE::DIV_OP);
					break;
				case (OPCODE::MOD_OP):
					WRITE_OPCODE(OPCODE::MOD_OP);
					break;
				case (OPCODE::POWER_OP):
					WRITE_OPCODE(OPCODE::POWER_OP);
					break;
				case (OPCODE::ASSIGN_OP):
					WRITE_OPCODE(OPCODE::ASSIGN_OP);
					break;
				case (OPCODE::GET_MEMBER):
					WRITE_OPCODE(OPCODE::GET_MEMBER);
					break;
				case (OPCODE::SET_ALU_INCR):
					WRITE_OPCODE(OPCODE::SET_ALU_INCR);
					break;
				case (OPCODE::CMP_EQ):
					WRITE_OPCODE(OPCODE::CMP_EQ);
					break;
				case (OPCODE::CMP_NEQ):
					WRITE_OPCODE(OPCODE::CMP_NEQ);
					break;
				case (OPCODE::CMP_L):
					WRITE_OPCODE(OPCODE::CMP_L);
					break;
				case (OPCODE::CMP_G):
					WRITE_OPCODE(OPCODE::CMP_G);
					break;
				case (OPCODE::CMP_LE):
					WRITE_OPCODE(OPCODE::CMP_LE);
					break;
				case (OPCODE::CMP_GE):
					WRITE_OPCODE(OPCODE::CMP_GE);
					break;
				case (OPCODE::CMP_AND):
					WRITE_OPCODE(OPCODE::CMP_AND);
					break;
				case (OPCODE::CMP_OR):
					WRITE_OPCODE(OPCODE::CMP_OR);
					break;
				case (OPCODE::GET_INDEX):
					WRITE_OPCODE(OPCODE::GET_INDEX);
					break;
				case (OPCODE::CALL_FUNCTION):
					WRITE_OPCODE(OPCODE::CALL_FUNCTION);
					WRITE_OPCODE(GenericRead<uint8_t>());
					break;
				case (OPCODE::RETURN):
					WRITE_OPCODE(OPCODE::RETURN);
					break;
				case (OPCODE::SET_LABEL):
					// as labels are stored in OffsetArray, opcode is useless and ignored
					RegisterLabelInMethod(method, ReadLabel());
					break;
				case (OPCODE::JUMP):
					WRITE_OPCODE(OPCODE::JUMP);
					WRITE_LABEL;
					break;
				case (OPCODE::JUMP_IF_TRUE):
					WRITE_OPCODE(OPCODE::JUMP_IF_TRUE);
					WRITE_LABEL;
					break;
				case (OPCODE::JUMP_IF_FALSE):
					WRITE_OPCODE(OPCODE::JUMP_IF_FALSE);
					WRITE_LABEL;
					break;
				case (OPCODE::PUSH_STACKFRAME):
					WRITE_OPCODE(OPCODE::PUSH_STACKFRAME);
					break;
				case (OPCODE::POP_STACK_TOP):
					WRITE_OPCODE(OPCODE::POP_STACK_TOP);
					break;
				case (OPCODE::METHOD_BODY_END_DECL):
					return method; // success
				default:
					ExpectOpcode(OPCODE::METHOD_BODY_END_DECL, op); // reaches only if error occured
					return method; // error occured
				}
			}
			#undef WRITE_OPCODE
			#undef WRITE_LABEL
			#undef WRITE_HASH
			ExpectOpcode(OPCODE::METHOD_BODY_END_DECL, op); // reaches only if error occured
			return method;
		}

		void AssemblyEditor::RegisterLabelInMethod(MethodType& method, uint16_t label)
		{
			if (label >= method.labels.size())
			{
				method.labels.resize(label + 1);
			}
			method.labels[label] = method.body.size(); // current offset of the method body
		}

		OPCODE AssemblyEditor::ReadOPCode()
		{
			return GenericRead<VM::OPCODE>();
		}

		size_t AssemblyEditor::ReadSize()
		{
			return GenericRead<size_t>();
		}

		uint8_t AssemblyEditor::ReadModifiers()
		{
			return GenericRead<uint8_t>();
		}

		uint16_t AssemblyEditor::ReadLabel()
		{
			return GenericRead<uint16_t>();
		}

		std::string AssemblyEditor::ReadString()
		{
			uint8_t size = GenericRead<uint8_t>();
			std::string res(size, '?');
			file.read(&res[0], size);
			return res;
		}
	}
}