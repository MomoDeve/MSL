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
			object->attributes.reserve(_class->objectAttributes.size());
			for (const auto& attr : _class->objectAttributes)
			{
				object->attributes[attr.second.name] = std::make_unique<AttributeObject>(&attr.second);
			}
			return object;
		}

		BaseObject* VirtualMachine::AllocNamespaceWrapper(const NamespaceType* _namespace)
		{
			return new NamespaceWrapper(_namespace);
		}

		BaseObject* VirtualMachine::AllocLocal(Local& local)
		{
			return new LocalObject(local);
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
				auto classIt = actualClass->staticInstance->attributes.find(objectName);
				if (classIt != actualClass->staticInstance->attributes.end())
				{
					return classIt->second.get();
				}
			}
			else
			{
				const ClassObject* thisObject = reinterpret_cast<const ClassObject*>(_class);
				auto classIt = thisObject->attributes.find(objectName);
				if (classIt != thisObject->attributes.end())
				{
					return classIt->second.get();
				}
			}
			auto namespaceIt = _namespace->classes.find(objectName);
			if (namespaceIt != _namespace->classes.end()) return namespaceIt->second.wrapper;
			auto assemblyIt = assembly.namespaces.find(objectName);
			if (assemblyIt != assembly.namespaces.end()) return AllocNamespaceWrapper(&assemblyIt->second);

			errors |= ERROR::OBJECT_NOT_FOUND;
			return nullptr;
		}

		void VirtualMachine::StartNewStackFrame()
		{
			struct Frame
			{
				LocalsTable locals;
				const NamespaceType* _namespace = nullptr;
				const ClassType* _class = nullptr;
				const MethodType* _method = nullptr;
				BaseObject* classObject = nullptr;
				size_t offset = 0;
			};

			if (callStack.size() > config.execution.recursionLimit)
			{
				errors |= ERROR::STACKOVERFLOW;
				return;
			}
			auto frame = std::make_unique<Frame>();
			frame->_namespace = GetNamespaceOrNull(*callStack.top().GetNamespace());
			frame->_class = GetClassOrNull(frame->_namespace, *callStack.top().GetClass());
			frame->_method = GetMethodOrNull(frame->_class, *callStack.top().GetMethod());
			if (frame->_method == nullptr)
			{
				errors |= ERROR::MEMBER_NOT_FOUND;
				return;
			}
			if (frame->_class->modifiers & ClassType::Modifiers::SYSTEM)
			{
				PerformSystemCall(frame->_class, frame->_method);
				return;
			}
			if (frame->_method->body[frame->offset++] != OPCODE::PUSH_STACKFRAME)
			{
				errors |= ERROR::INVALID_OPCODE;
				return;
			}
			for (auto it = frame->_method->parameters.rbegin(); it != frame->_method->parameters.rend(); it++)
			{
				if (objectStack.empty())
				{
					errors |= ERROR::OBJECTSTACK_EMPTY;
					return;
				}
				frame->locals[*it].object = objectStack.back();
				objectStack.pop_back();
			}
			if ((frame->_method->modifiers & MethodType::Modifiers::STATIC) == 0 && (frame->_method->modifiers & MethodType::Modifiers::CONSTRUCTOR) == 0)
			{
				if (frame->_method->parameters.empty() || frame->_method->parameters.front() != "this")
				{
					errors |= ERROR::INVALID_METHOD_SIGNATURE;
				}
				else
				{
					frame->classObject = frame->locals["this"].object;
				}
			}
			else
			{
				objectStack.pop_back(); // reference to this / class should be popped anyway
				frame->classObject = frame->_class->wrapper;
			}
			if (frame->_method->modifiers & MethodType::Modifiers::CONSTRUCTOR)
			{
				frame->locals["this"].object = AllocClassObject(frame->_class);
			}
			while (frame->offset < frame->_method->body.size())
			{
				if (frame->offset >= frame->_method->body.size())
				{
					errors |= ERROR::INVALID_STACKFRAME_OFFSET;
				}
				if (errors != 0) return;
				OPCODE op = ReadOPCode(frame->_method->body, frame->offset);

				switch (op)
				{
				case (OPCODE::PUSH_OBJECT):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					const std::string* objectName = &frame->_method->dependencies[hash];
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
					if (objectStack.size() < 2)
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
					}
					else
					{
						BaseObject* value = objectStack.back();
						objectStack.pop_back();
						BaseObject* obj = objectStack.back();
						objectStack.pop_back();
						if (obj->type == Type::UNKNOWN)
						{
							obj = SearchForObject(*obj->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
							if (obj == nullptr)
							{
								errors |= ERROR::OBJECT_NOT_FOUND;
								return;
							}
						}
						switch (obj->type)
						{
						case Type::LOCAL:
							reinterpret_cast<LocalObject*>(obj)->ref.object = value;
							objectStack.push_back(obj);
							break;
						case Type::ATTRIBUTE:
							reinterpret_cast<AttributeObject*>(obj)->object = value;
							objectStack.push_back(obj);
							break;
						default:
							errors |= ERROR::INVALID_STACKOBJECT;
							break;
						}
					}
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
					uint8_t paramSize = ReadOPCode(frame->_method->body, frame->offset);
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
					for (size_t i = 0; i < paramSize; i++)
					{
						size_t index = objectStack.size() - i - 2;
						BaseObject* obj = objectStack[index];
						if (obj->type == Type::UNKNOWN)
						{
							objectStack[index] = SearchForObject(*obj->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
							if (objectStack[index] == nullptr)
							{
								errors |= ERROR::INVALID_CALL_ARGUMENT;
								return;
							}
						}
					}
					CallPath newFrame;
					BaseObject* caller = objectStack[objectStack.size() - paramSize - 2];
					if (caller->type == Type::UNKNOWN)
					{
						caller = SearchForObject(*caller->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
					}
					if (caller == nullptr)
					{
						errors |= ERROR::OBJECT_NOT_FOUND;
						return;
					}
					if (caller->type == Type::CLASS_OBJECT)
					{
						ClassObject* object = reinterpret_cast<ClassObject*>(caller);
						newFrame.SetNamespace(&object->type->namespaceName);
						newFrame.SetClass(&object->type->name);
						newFrame.SetMethod(objectStack.back()->GetName());
						callStack.push(newFrame);
					}
					else if (caller->type == Type::CLASS)
					{
						ClassWrapper* object = reinterpret_cast<ClassWrapper*>(caller);
						newFrame.SetNamespace(&object->type->namespaceName);
						newFrame.SetClass(&object->type->name);
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
						calledObject = SearchForObject(*calledObject->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
					}
					if (calledObject == nullptr)
					{
						errors |= ERROR::OBJECT_NOT_FOUND;
						return;
					}
					BaseObject* memberObject = calledObject->GetMember(*member->GetName());
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
					else
					{
						if (objectStack.back()->type == Type::UNKNOWN)
						{
							objectStack.back() = SearchForObject(*objectStack.back()->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
						}
						if (objectStack.back() == nullptr)
						{
							errors |= ERROR::OBJECT_NOT_FOUND;
						}
						callStack.pop();
					}
					return;
				case (OPCODE::ALLOC_VAR):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if (ValidateHashValue(hash, frame->_method->dependencies.size()))
					{
						objectStack.push_back(AllocLocal(
							frame->locals[frame->_method->dependencies[hash]] = { AllocNull(), false }
						));
					}
					break;
				}
				case (OPCODE::ALLOC_CONST_VAR):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if (ValidateHashValue(hash, frame->_method->dependencies.size()))
					{
						objectStack.push_back(AllocLocal(
							frame->locals[frame->_method->dependencies[hash]] = { AllocNull(), true }
						));
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
					uint16_t label = ReadLabel(frame->_method->body, frame->offset);
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (object->type == Type::TRUE)
						frame->offset = frame->_method->labels[label];
					break;
				}
				case (OPCODE::JUMP_IF_FALSE):
				{
					uint16_t label = ReadLabel(frame->_method->body, frame->offset);
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (object->type == Type::FALSE)
						frame->offset = frame->_method->labels[label];
					break;
				}
				case (OPCODE::PUSH_STRING):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if (ValidateHashValue(hash, frame->_method->dependencies.size()))
						objectStack.push_back(AllocString(frame->_method->dependencies[hash]));
					break;
				}
				case (OPCODE::PUSH_INTEGER):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if(ValidateHashValue(hash, frame->_method->dependencies.size()))
						objectStack.push_back(AllocInteger(frame->_method->dependencies[hash]));
					break;
				}
				case (OPCODE::PUSH_FLOAT):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if(ValidateHashValue(hash, frame->_method->dependencies.size()))
						objectStack.push_back(AllocFloat(frame->_method->dependencies[hash]));
					break;
				}
				case (OPCODE::PUSH_THIS):
					objectStack.push_back(frame->classObject);
					break;
				case (OPCODE::PUSH_NULL):
					objectStack.push_back(AllocNull());
					break;
				case (OPCODE::SET_ALU_INCR):
					ALUinIncrMode = true;
					break;
				case (OPCODE::RETURN):
					if (frame->_method->modifiers & MethodType::Modifiers::CONSTRUCTOR)
					{
						objectStack.push_back(frame->locals["this"].object);
					}
					else
					{
						objectStack.push_back(AllocNull());
					}
					callStack.pop();
					return;
					break;
				case (OPCODE::JUMP):
					frame->offset = frame->_method->labels[ReadLabel(frame->_method->body, frame->offset)];
					break;
				case (OPCODE::POP_STACK_TOP):
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
					}
					else
					{
						objectStack.pop_back();
					}
					break;
				default:
					errors |= ERROR::INVALID_OPCODE;
					break;
				}
			}
		}

		void VirtualMachine::PerformSystemCall(const ClassType* _class, const MethodType* _method)
		{
			if (objectStack.empty())
			{
				errors |= ERROR::OBJECTSTACK_EMPTY;
				return;
			}
			if (_class->name == "Console")
			{
				if (_method->name == "Print_1" || _method->name == "PrintLine_1")
				{
					std::ostream& out = *config.streams.out;
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					switch (object->type)
					{
					case Type::STRING:
					{
						StringObject* str = reinterpret_cast<StringObject*>(object);
						out << str->value;
						break;
					}
					case Type::FLOAT:
					{
						FloatObject* f = reinterpret_cast<FloatObject*>(object);
						out << f->value;
						break;
					}
					case Type::INTEGER:
					{
						IntegerObject* integer = reinterpret_cast<IntegerObject*>(object);
						out << integer->value;
						break;
					}
					case Type::NULLPTR:
					{
						out << "null";
						break;
					}
					case Type::TRUE:
					{
						out << "true";
						break;
					}
					case Type::FALSE:
					{
						out << "false";
						break;
					}
					case Type::NAMESPACE:
					{
						out << "namespace " << *object->GetName();
						break;
					}
					case Type::CLASS:
					{
						ClassWrapper* c = reinterpret_cast<ClassWrapper*>(object);
						out << "class " << c->type->namespaceName << '.' << c->type->name;
						break;
					}
					case Type::ATTRIBUTE:
					{
						AttributeObject* attr = reinterpret_cast<AttributeObject*>(object);
						objectStack.push_back(attr->object);
						PerformSystemCall(_class, _method);
						return; // no PrintLine check, because it will happen inside recursion call
					}
					case Type::CLASS_OBJECT:
					{
						ClassObject* classObject = reinterpret_cast<ClassObject*>(object);
						const ClassType* classType = classObject->type;
						auto it = classType->methods.find("ToString_1");
						if (it != classType->methods.end() && (it->second.modifiers & (MethodType::Modifiers::STATIC | MethodType::Modifiers::ABSTRACT)) == 0)
						{
							objectStack.push_back(classObject);
							InvokeObjectMethod("ToString_1", classObject);
							PerformSystemCall(_class, _method);
							return; // no PrintLine check, because it will happen inside recursion call
						}
						else
						{
							out << classType->namespaceName << '.' << classType->name << " object";
						}
						break;
					}
					default:
						errors |= ERROR::INVALID_CALL_ARGUMENT;
						break;
					}
					if (_method->name == "PrintLine_1")
					{
						out << std::endl;
					}
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
					c.staticInstance = new ClassObject(&c);
					c.staticInstance->attributes.reserve(c.staticAttributes.size());
					for (const auto& attr : c.staticAttributes)
					{
						c.staticInstance->attributes[attr.second.name] = std::make_unique<AttributeObject>(&attr.second);
					}
				}
			}
		}

		void VirtualMachine::AddSystemNamespace()
		{
			NamespaceType system;
			system.name = "System";

			ClassType console;
			console.name = "Console";
			console.namespaceName = system.name;
			console.modifiers |= ClassType::Modifiers::STATIC | ClassType::Modifiers::ABSTRACT | ClassType::Modifiers::SYSTEM;

			MethodType print; // outputs value to console
			print.name = "Print_1";
			print.parameters.push_back("value");
			print.modifiers |= MethodType::Modifiers::PUBLIC | MethodType::Modifiers::STATIC;
			console.methods.insert({ "Print_1", std::move(print) });

			MethodType printLine; // outputs line and flushes out stream
			printLine.name = "PrintLine_1";
			printLine.parameters.push_back("value");
			printLine.modifiers |= MethodType::Modifiers::PUBLIC | MethodType::Modifiers::STATIC;
			console.methods.insert({ "PrintLine_1", std::move(printLine) });

			system.classes.insert({ "Console", std::move(console) });

			assembly.namespaces.insert({ "System", std::move(system) });
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

		void VirtualMachine::InvokeObjectMethod(const std::string& methodName, const ClassObject* object)
		{
			auto it = object->type->methods.find(methodName);
			if (it == object->type->methods.end())
			{
				errors |= ERROR::MEMBER_NOT_FOUND;
				return;
			}
			if (it->second.modifiers & (MethodType::Modifiers::ABSTRACT | MethodType::Modifiers::STATIC))
			{
				errors |= ERROR::INVALID_METHOD_SIGNATURE;
				return;
			}
			if ((it->second.modifiers & MethodType::Modifiers::PUBLIC) == 0)
			{
				errors |= ERROR::PRIVATE_MEMBER_ACCESS;
				return;
			}
			CallPath newFrame;
			newFrame.SetMethod(&methodName);
			newFrame.SetClass(&object->type->name);
			newFrame.SetNamespace(&object->type->namespaceName);
			callStack.push(std::move(newFrame));
			StartNewStackFrame();
		}

		BaseObject* VirtualMachine::AllocUnknown(const std::string* value)
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
			AddSystemNamespace();
			InitializeStaticMembers();
			if (callStack.empty())
			{
				errors |= ERROR::CALLSTACK_EMPTY | ERROR::TERMINATE_ON_LAUNCH;
				return;
			}
			const CallPath& path = callStack.top();
			const MethodType* entryPoint = GetMethodOrNull(*path.GetNamespace(), *path.GetClass(), *path.GetMethod());
			if (entryPoint == nullptr)
			{
				errors |= ERROR::INVALID_CALL_ARGUMENT | ERROR::TERMINATE_ON_LAUNCH;
				return;
			}
			objectStack.push_back(AllocNull()); // reference to class, not needed anyway
			for (size_t i = 0; i < entryPoint->parameters.size(); i++)
			{
				objectStack.push_back(AllocNull());
			}
			StartNewStackFrame();
			if (config.execution.checkExitCode)
			{
				if (objectStack.size() != 1)
				{
					errors |= ERROR::OBJECTSTACK_CORRUPTION;
					return;
				}
				if (objectStack.back()->type == Type::INTEGER)
				{
					*config.streams.out << "VM execution finished with exit code " << reinterpret_cast<IntegerObject*>(objectStack.back())->value << std::endl;
				}
				else if (objectStack.back()->type == Type::NULLPTR)
				{
					*config.streams.out << "VM execution finished with exit code 0" << std::endl;
				}
				else
				{
					errors |= ERROR::INVALID_STACKOBJECT;
				}
			}
		}

		uint32_t VirtualMachine::GetErrors() const
		{
			return errors;
		}
	}
}