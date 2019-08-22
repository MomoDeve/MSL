#include "virtualMachine.h"

namespace MSL
{
	namespace VM
	{
		BaseObject* VirtualMachine::AllocClassWrapper(const ClassType* _class)
		{
			return new ClassWrapper(_class);
		}

		BaseObject* VirtualMachine::AllocClassObject(const ClassType* _class)
		{
			ClassObject* object = new ClassObject(_class);
			object->attributes.resize(_class->attributes.size(), AllocNull());
			return object;
		}

		BaseObject* VirtualMachine::AllocNamespaceWrapper(const NamespaceType* _namespace)
		{
			return new NamespaceWrapper(_namespace);
		}

		OPCODE VirtualMachine::ReadOPCode(const std::vector<uint8_t>& bytes, size_t& offset)
		{
			return GenericRead<OPCODE>(bytes, offset);
		}

		uint16_t VirtualMachine::ReadLabel(const std::vector<uint8_t>& bytes, size_t & offset)
		{
			return GenericRead<uint16_t>(bytes, offset);
		}

		size_t VirtualMachine::ReadHash(const std::vector<uint8_t>& bytes, size_t & offset)
		{
			return GenericRead<size_t>(bytes, offset);
		}

		const MethodType* VirtualMachine::GetMethodOrNull(const std::string& _namespace, const std::string& _class, const std::string& _method) const
		{
			return GetMethodOrNull(GetClassOrNull(_namespace, _class), _method);
		}

		const MethodType* VirtualMachine::GetMethodOrNull(const ClassType* _class, const std::string& _method) const
		{
			if (_class == nullptr) return nullptr;
			auto it = _class->methods.find(_method);
			if (it == _class->methods.end()) return nullptr;
			else return &(it->second);
		}

		const ClassType* VirtualMachine::GetClassOrNull(const std::string& _namespace, const std::string& _class) const
		{
			return GetClassOrNull(GetNamespaceOrNull(_namespace), _class);
		}

		const ClassType* VirtualMachine::GetClassOrNull(const NamespaceType* _namespace, const std::string& _class) const
		{
			if (_namespace == nullptr) return nullptr;
			auto it = _namespace->classes.find(_class);
			if (it == _namespace->classes.end()) return nullptr;
			else return &(it->second);
		}

		const NamespaceType* VirtualMachine::GetNamespaceOrNull(const std::string& _namespace) const
		{
			auto it = assembly.namespaces.find(_namespace);
			if (it == assembly.namespaces.end()) return nullptr;
			else return &(it->second);
		}

		BaseObject* VirtualMachine::SearchForObject(const std::string& objectName, const LocalsTable& locals, const MethodType* _method, const BaseObject* _class, const NamespaceType* _namespace)
		{
			auto localsIt = locals.find(objectName);
			if (localsIt != locals.end()) return localsIt->second.object;

			if (_method->modifiers & MethodType::Modifiers::STATIC)
			{
				const ClassType* actualClass = reinterpret_cast<const ClassWrapper*>(_class)->type;
				auto classIt = actualClass->attributes.find(objectName);
				if (classIt != actualClass->attributes.end() && (classIt->second.modifiers & AttributeType::Modifiers::STATIC))
				{
					return actualClass->staticInstance->attributes[classIt->second.offset];
				}
			}
			else
			{
				const ClassObject* thisObject = reinterpret_cast<const ClassObject*>(_class);
				auto classIt = thisObject->type->attributes.find(objectName);
				if (classIt != thisObject->type->attributes.end())
				{
					if (classIt->second.modifiers & AttributeType::Modifiers::STATIC)
					{
						return thisObject->type->staticInstance->attributes[classIt->second.offset];
					}
					else
					{
						return thisObject->attributes[classIt->second.offset];
					}
				}
			}
			auto namespaceIt = _namespace->classes.find(objectName);
			if (namespaceIt != _namespace->classes.end()) return AllocClassWrapper(&namespaceIt->second);
			auto assemblyIt = assembly.namespaces.find(objectName);
			if (assemblyIt != assembly.namespaces.end()) return AllocNamespaceWrapper(&assemblyIt->second);

			errors |= ERROR::OBJECT_NOT_FOUND;
			return nullptr;
		}

		void VirtualMachine::StartNewStackFrame()
		{
			if (callStack.size() > config.execution.recursionLimit)
			{
				errors |= ERROR::STACKOVERFLOW;
				return;
			}
			CallPath& frame = callStack.top();
			const NamespaceType* currentNamespace = GetNamespaceOrNull(frame.GetNamespace());
			const ClassType* currentClass = GetClassOrNull(currentNamespace, frame.GetClass());
			const MethodType* currentMethod = GetMethodOrNull(currentClass, frame.GetMethod());
			LocalsTable locals;
			if (currentMethod == nullptr)
			{
				errors |= ERROR::MEMBER_NOT_FOUND;
				return;
			}
			if (currentMethod->body[frame.offset++] != OPCODE::PUSH_STACKFRAME)
			{
				errors |= ERROR::OPERANDSTACK_CORRUPTION;
				return;
			}
			for (auto it = currentMethod->parameters.rbegin(); it != currentMethod->parameters.rend(); it++)
			{
				if (objectStack.empty())
				{
					errors |= ERROR::OBJECTSTACK_EMPTY;
					return;
				}
				locals[*it].object = objectStack.back();
				objectStack.pop_back();
			}
			BaseObject* currentClassObject = nullptr;
			if ((currentMethod->modifiers & MethodType::Modifiers::STATIC) == 0)
			{
				if (currentMethod->parameters.empty() || currentMethod->parameters.front() != "this")
				{
					errors |= ERROR::INVALID_METHOD_SIGNATURE;
				}
				else
				{
					currentClassObject = locals["this"].object;
				}
			}
			else
			{
				currentClassObject = AllocClassWrapper(currentClass);
			}
			while (frame.offset < currentMethod->body.size())
			{
				if (frame.offset >= currentMethod->body.size())
				{
					errors |= ERROR::INVALID_STACKFRAME_OFFSET;
				}
				if (errors != 0) return;
				OPCODE op = ReadOPCode(currentMethod->body, frame.offset);

				switch (op)
				{
				case (OPCODE::PUSH_OBJECT):
				{
					size_t hash = ReadHash(currentMethod->body, frame.offset);
					const std::string& objectName = currentMethod->dependencies[hash];
					objectStack.push_back(AllocUnknown(objectName));
					break;
				}
				case (OPCODE::NEGATION_OP):
					break;
				case (OPCODE::NEGATIVE_OP):
					break;
				case (OPCODE::POSITIVE_OP):
					break;
				case (OPCODE::SUM_OP):
					break;
				case (OPCODE::SUB_OP):
					break;
				case (OPCODE::MULT_OP):
					break;
				case (OPCODE::DIV_OP):
					break;
				case (OPCODE::MOD_OP):
					break;
				case (OPCODE::POWER_OP):
					break;
				case (OPCODE::ASSIGN_OP):
					break;
				case (OPCODE::CMP_EQ):
					break;
				case (OPCODE::CMP_NEQ):
					break;
				case (OPCODE::CMP_L):
					break;
				case (OPCODE::CMP_G):
					break;
				case (OPCODE::CMP_LE):
					break;
				case (OPCODE::CMP_GE):
					break;
				case (OPCODE::CMP_AND):
					break;
				case (OPCODE::CMP_OR):
					break;
				case (OPCODE::GET_INDEX):
					break;
				case (OPCODE::CALL_FUNCTION):
				{
					uint8_t paramSize = ReadOPCode(currentMethod->body, frame.offset);
					if (objectStack.size() < paramSize + 2u) // function object + caller object
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						return;
					}
					if (objectStack.back()->type != Type::UNKNOWN)
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						return;
					}
					CallPath newFrame;
					BaseObject* caller = objectStack[objectStack.size() - paramSize - 2];
					if (caller->type == Type::CLASS_OBJECT)
					{
						ClassObject* object = reinterpret_cast<ClassObject*>(caller);
						newFrame.SetNamespace(object->type->namespaceName);
						newFrame.SetClass(object->type->name);
						newFrame.SetMethod(objectStack.back()->GetName());
						callStack.push(newFrame);
							
					}
					else if (caller->type == Type::CLASS)
					{
						ClassWrapper* object = reinterpret_cast<ClassWrapper*>(caller);
						newFrame.SetNamespace(object->type->namespaceName);
						newFrame.SetClass(object->type->name);
						newFrame.SetMethod(objectStack.back()->GetName());
						callStack.push(newFrame);
					}
					else
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						return;
					}
					objectStack.pop_back(); // remove unknown object (function name)
					StartNewStackFrame();
					break;
				}
				case (OPCODE::GET_MEMBER):
				{
					if (objectStack.size() < 2)
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						return;
					}
					BaseObject* member = objectStack.back();
					objectStack.pop_back();
					BaseObject* calledObject = objectStack.back();
					objectStack.pop_back();
					if (calledObject->type == Type::UNKNOWN)
					{
						calledObject = SearchForObject(calledObject->GetName(), locals, currentMethod, currentClassObject, currentNamespace);
					}
					if (calledObject == nullptr)
					{
						errors |= ERROR::OBJECT_NOT_FOUND;
						return;
					}
					BaseObject* memberObject = calledObject->GetMember(member->GetName());
					if (memberObject == nullptr)
					{
						errors |= ERROR::MEMBER_NOT_FOUND;
					}
					else
					{
						objectStack.push_back(memberObject);
					}
					break;
				}
				case (OPCODE::POP_TO_RETURN):
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
					}
					else callStack.pop();
					return;
				case (OPCODE::ALLOC_VAR):
				{
					size_t hash = ReadHash(currentMethod->body, frame.offset);
					if (ValidateHashValue(hash, currentMethod->dependencies.size()))
					{
						if (objectStack.empty())
						{
							errors |= ERROR::OBJECTSTACK_EMPTY;
						}
						else
						{
							locals[currentMethod->dependencies[hash]].object = objectStack.back();
							objectStack.pop_back();
						}
					}
					break;
				}
				case (OPCODE::ALLOC_CONST_VAR):
				{
					size_t hash = ReadHash(currentMethod->body, frame.offset);
					if (ValidateHashValue(hash, currentMethod->dependencies.size()))
					{
						if (objectStack.empty())
						{
							errors |= ERROR::OBJECTSTACK_EMPTY;
						}
						else
						{
							locals[currentMethod->dependencies[hash]] = { objectStack.back(), true };
							objectStack.pop_back();
						}
					}
					break;
				}
				case (OPCODE::PUSH_TRUE):
					objectStack.push_back(AllocTrue());
					break;
				case (OPCODE::PUSH_FALSE):
					objectStack.push_back(AllocFalse());
					break;
				case (OPCODE::JUMP_IF_TRUE):
				{
					uint16_t label = ReadLabel(currentMethod->body, frame.offset);
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (object->type == Type::TRUE)
						frame.offset = currentMethod->labels[label];
					break;
				}
				case (OPCODE::JUMP_IF_FALSE):
				{
					uint16_t label = ReadLabel(currentMethod->body, frame.offset);
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (object->type == Type::FALSE)
						frame.offset = currentMethod->labels[label];
					break;
				}
				case (OPCODE::PUSH_STRING):
				{
					size_t hash = ReadHash(currentMethod->body, frame.offset);
					if (ValidateHashValue(hash, currentMethod->dependencies.size()))
						objectStack.push_back(AllocString(currentMethod->dependencies[hash]));
					break;
				}
				case (OPCODE::PUSH_INTEGER):
				{
					size_t hash = ReadHash(currentMethod->body, frame.offset);
					if(ValidateHashValue(hash, currentMethod->dependencies.size()))
						objectStack.push_back(AllocInteger(currentMethod->dependencies[hash]));
					break;
				}
				case (OPCODE::PUSH_FLOAT):
				{
					size_t hash = ReadHash(currentMethod->body, frame.offset);
					if(ValidateHashValue(hash, currentMethod->dependencies.size()))
						objectStack.push_back(AllocInteger(currentMethod->dependencies[hash]));
					break;
				}
				case (OPCODE::PUSH_THIS):
					objectStack.push_back(currentClassObject);
					break;
				case (OPCODE::PUSH_NULL):
					objectStack.push_back(AllocNull());
					break;
				case (OPCODE::SET_ALU_INCR):
					ALUinIncrMode = true;
					break;
				case (OPCODE::RETURN):
					objectStack.push_back(AllocNull());
					callStack.pop();
					return;
					break;
				case (OPCODE::JUMP):
					frame.offset = currentMethod->labels[ReadLabel(currentMethod->body, frame.offset)];
					break;
				case (OPCODE::POP_STACK_TOP):
					objectStack.pop_back();
					break;
				default:
					errors |= ERROR::INVALID_OPCODE;
					break;
				}
			}
		}

		void VirtualMachine::InitializeStaticMembers()
		{
			for (auto assemblyIt = assembly.namespaces.begin(); assemblyIt != assembly.namespaces.end(); assemblyIt++)
			{
				NamespaceType& ns = assemblyIt->second;
				for (auto namespaceIt = ns.classes.begin(); namespaceIt != ns.classes.end(); namespaceIt++)
				{
					ClassType& c = namespaceIt->second;
					c.wrapper = AllocClassWrapper(&c);
					c.staticInstance = reinterpret_cast<ClassObject*>(AllocClassObject(&c));
				}
			}
		}

		bool VirtualMachine::ValidateHashValue(size_t hashValue, size_t maxHashValue)
		{
			if (hashValue < maxHashValue)
			{
				return true;
			}
			else
			{
				errors |= ERROR::INVALID_HASH_VALUE;
				return false;
			}
		}

		BaseObject* VirtualMachine::AllocUnknown(const std::string& value)
		{
			return new UnknownObject(value);
		}

		BaseObject* VirtualMachine::AllocNull()
		{
			return &nullObject;
		}

		BaseObject* VirtualMachine::AllocTrue()
		{
			return &trueObject;
		}

		BaseObject* VirtualMachine::AllocFalse()
		{
			return &falseObject;
		}

		BaseObject* VirtualMachine::AllocString(const std::string& value)
		{
			return new StringObject(value);
		}

		BaseObject* VirtualMachine::AllocInteger(const std::string& value)
		{
			return new IntegerObject(value);
		}

		BaseObject* VirtualMachine::AllocFloat(const std::string& value)
		{
			return new FloatObject(value);
		}

		VirtualMachine::VirtualMachine(Configuration config)
			: config(std::move(config)), errors(0), ALUinIncrMode(false) { }

		bool VirtualMachine::AddBytecodeFile(std::istream* binaryFile)
		{
			if (!assembly.namespaces.empty() && !config.compilation.allowAssemblyMerge) return false;

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

		void VirtualMachine::Run()
		{
			InitializeStaticMembers();
			if (callStack.empty())
			{
				errors |= ERROR::CALLSTACK_EMPTY | ERROR::TERMINATE_ON_LAUNCH;
				return;
			}
			const CallPath& path = callStack.top();
			const MethodType* entryPoint = GetMethodOrNull(path.GetNamespace(), path.GetClass(), path.GetMethod());
			if (entryPoint == nullptr)
			{
				errors |= ERROR::INVALID_CALL_ARGUMENT | ERROR::TERMINATE_ON_LAUNCH;
				return;
			}
			for (size_t i = 0; i < entryPoint->parameters.size(); i++)
			{
				objectStack.push_back(AllocNull());
			}
			StartNewStackFrame();
		}

		uint32_t VirtualMachine::GetErrors() const
		{
			return errors;
		}
	}
}