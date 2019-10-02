#include "virtualMachine.h"

namespace MSL
{
	#define PRINTFRAME_2(_class, _method) DisplayExtra("current frame: " + GetFullClassType(_class) + '.' + GetFullMethodType(_method))
	#define PRINTFRAME PRINTFRAME_2(frame->_class, frame->_method)
	#define PRINTPREVFRAME CallPath top = std::move(callStack.top()); \
						   callStack.pop(); \
						   if (!callStack.empty()) \
						        DisplayExtra("called from frame: " +  \
								*callStack.top().GetNamespace() + \
								'.' + *callStack.top().GetClass() + \
								'.' + *callStack.top().GetMethod()); \
						   callStack.push(std::move(top))

	namespace VM
	{
		ClassWrapper* VirtualMachine::AllocClassWrapper(const ClassType* _class)
		{
			return new ClassWrapper(_class);
		}

		ClassObject* VirtualMachine::AllocClassObject(const ClassType* _class)
		{
			if (!_class->staticConstructorCalled && _class->hasStaticConstructor())
			{
				std::string staticConstructor = _class->name + "_0_static";
				CallPath newFrame;
				newFrame.SetNamespace(&_class->namespaceName);
				newFrame.SetClass(&_class->name);
				newFrame.SetMethod(&staticConstructor);
				callStack.push(std::move(newFrame));
				objectStack.push_back(_class->wrapper);
				StartNewStackFrame();
				objectStack.pop_back();
			}
			ClassObject* object = new ClassObject(_class);
			object->attributes.reserve(_class->objectAttributes.size());
			for (const auto& attr : _class->objectAttributes)
			{
				std::unique_ptr<AttributeObject> objectAttr = std::make_unique<AttributeObject>(&attr.second);
				objectAttr->object = AllocNull();
				object->attributes[attr.second.name] = std::move(objectAttr);
			}
			return object;
		}

		NamespaceWrapper* VirtualMachine::AllocNamespaceWrapper(const NamespaceType* _namespace)
		{
			return new NamespaceWrapper(_namespace);
		}

		LocalObject* VirtualMachine::AllocLocal(const std::string& localName, Local& local)
		{
			return new LocalObject(local, localName);
		}

		OPCODE VirtualMachine::ReadOPCode(const std::vector<uint8_t>& bytes, size_t& offset)
		{
			return GenericRead<OPCODE>(bytes, offset);
		}

		uint16_t VirtualMachine::ReadLabel(const std::vector<uint8_t>& bytes, size_t& offset)
		{
			return GenericRead<uint16_t>(bytes, offset);
		}

		size_t VirtualMachine::ReadHash(const std::vector<uint8_t>& bytes, size_t& offset)
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
			// search for local variable in method
			auto localsIt = locals.find(objectName);
			if (localsIt != locals.end()) return localsIt->second.object;

			// search for attribute in class object
			const ClassType* actualClass = nullptr;
			if (!_method->isStatic())
			{
				const ClassObject* thisObject = reinterpret_cast<const ClassObject*>(_class);
				actualClass = thisObject->type;
				auto classIt = thisObject->attributes.find(objectName);
				if (classIt != thisObject->attributes.end())
				{
					return classIt->second.get();
				}
			}
			else
			{
				actualClass = reinterpret_cast<const ClassWrapper*>(_class)->type;
			}
			// search for static attribute in class
			auto classIt = actualClass->staticInstance->attributes.find(objectName);
			if (classIt != actualClass->staticInstance->attributes.end())
			{
				return classIt->second.get();
			}

			// search for class in namespace / friend namespaces
			BaseObject* classWrap = SearchForClass(objectName, _namespace);
			if (classWrap != nullptr) return classWrap;

			// search for namespace in assembly
			const auto ns = GetNamespaceOrNull(objectName);
			if (ns != nullptr) return AllocNamespaceWrapper(ns);

			errors |= ERROR::OBJECT_NOT_FOUND;
			DisplayError("object with name: `" + objectName + "` was not found");
			std::string className;
			if (_method->isStatic())
			{
				className = reinterpret_cast<const ClassWrapper*>(_class)->type->name + "[static]";
			}
			else
			{
				className = reinterpret_cast<const ClassObject*>(_class)->type->name + "[this]";
			}
			DisplayExtra("current frame: " + _namespace->name + '.' + className + '.' + GetFullMethodType(_method));
			return nullptr;
		}

		ClassWrapper* VirtualMachine::SearchForClass(const std::string& objectName, const NamespaceType* _namespace)
		{
			// search for class in current namespace
			const auto _class = GetClassOrNull(_namespace, objectName);
			if (_class != nullptr) return _class->wrapper;
			
			// search for classes in friend namespaces
			ClassWrapper* classWrap = nullptr;
			for (const auto& ns : _namespace->friendNamespaces)
			{
				const auto otherNamespace = GetNamespaceOrNull(ns);
				if (otherNamespace != nullptr) // no error if namespace does not exist
				{
					const auto otherClass = GetClassOrNull(otherNamespace, objectName);
					if (otherClass != nullptr && !otherClass->isInternal()) // class must be public
					{
						if (classWrap == nullptr)
						{
							classWrap = otherClass->wrapper;
						}
						else
						{
							errors |= ERROR::INVALID_CALL_ARGUMENT;
							DisplayError("find two or more matching classes while resolving object type: " + objectName);
							return nullptr;
						}
					}
				}
			}
			return classWrap;
		}

		BaseObject* VirtualMachine::GetUnderlyingObject(BaseObject* object) const
		{
			switch (object->type)
			{
			case MSL::VM::Type::CLASS_OBJECT:
			case MSL::VM::Type::INTEGER:
			case MSL::VM::Type::FLOAT:
			case MSL::VM::Type::STRING:
			case MSL::VM::Type::NULLPTR:
			case MSL::VM::Type::TRUE:
			case MSL::VM::Type::FALSE:
			case MSL::VM::Type::NAMESPACE:
			case MSL::VM::Type::CLASS:
			case MSL::VM::Type::UNKNOWN:
				return object;
			case MSL::VM::Type::LOCAL:
				return reinterpret_cast<LocalObject*>(object)->ref.object;
			case MSL::VM::Type::ATTRIBUTE:
				return reinterpret_cast<AttributeObject*>(object)->object;
			default:
				return nullptr; // hits only if error occured
			}
		}

		void VirtualMachine::StartNewStackFrame()
		{
			// recursion limit check
			if (callStack.size() > config.execution.recursionLimit)
			{
				errors |= ERROR::STACKOVERFLOW;
				return;
			}
			// getting frame arguments
			auto frame = std::make_unique<Frame>();
			frame->_namespace = GetNamespaceOrNull(*callStack.top().GetNamespace());
			frame->_class = GetClassOrNull(frame->_namespace, *callStack.top().GetClass());
			frame->_method = GetMethodOrNull(frame->_class, *callStack.top().GetMethod());

			// in case method was not found
			if (frame->_method == nullptr)
			{
				// probably class constructor was called (class name => method name)
				if (frame->_namespace != nullptr)
				{
					std::string methodName = *callStack.top().GetMethod();
					std::string className = GetMethodActualName(methodName);
					ClassWrapper* wrapper = SearchForClass(className, frame->_namespace);
					if (wrapper != nullptr)
					{
						const ClassType* classType = wrapper->type;
						if (classType->methods.find(methodName) != classType->methods.end()) // constructor was found, delegating call to new frame
						{
							CallPath newFrame;
							newFrame.SetNamespace(&classType->namespaceName);
							newFrame.SetClass(&className);
							newFrame.SetMethod(&methodName);
							callStack.pop();
							callStack.push(std::move(newFrame));
							StartNewStackFrame();
							return;
						}
						else if (classType->isAbstract()) // if class is abstract, constructor is not found too
						{
							DisplayError("cannot create instance of abstract class: " + GetFullClassType(classType));
							errors |= ERROR::ABSTRACT_MEMBER_CALL;
						}
						else if (classType->isStatic())
						{
							DisplayError("cannot create instance of static class: " + GetFullClassType(classType));
							errors |= ERROR::MEMBER_NOT_FOUND;
						}
						else // probably constructor is just missing
						{
							DisplayError("could not call class " + GetFullClassType(classType) + " constructor: " + methodName);
							DisplayExtra("available constructors of this class:");
							for (int i = 0; i < 17; i++) // find all possible class constructors (with less than 17 parameters, at least)
							{
								std::string methodName = classType->name + '_' + std::to_string(i);
								const auto method = GetMethodOrNull(classType, methodName);
								if (method != nullptr)
								{
									DisplayExtra('\t' + GetFullClassType(classType) + '.' + GetFullMethodType(method));
								}
							}
						}
					}
					else
					{
						errors |= ERROR::MEMBER_NOT_FOUND;
						DisplayError("could not find class `" + className + "` in namespace: " + frame->_namespace->name);
					}
				}
				DisplayError("method passed to frame was not found: " + *callStack.top().GetNamespace() + '.' + *callStack.top().GetClass() + '.' + *callStack.top().GetMethod());

				// displaying caller frame for debug
				errors |= ERROR::MEMBER_NOT_FOUND;
				PRINTPREVFRAME;
				return;
			}
			// if member if private, and it is called from another class, call is invalid
			if (frame->_method->isPublic())
			{
				if (callStack.empty() || (*callStack.top().GetNamespace() != frame->_namespace->name || *callStack.top().GetClass() != frame->_class->name))
				{
					errors |= ERROR::PRIVATE_MEMBER_ACCESS;
					DisplayError("trying to call private method: " + GetFullClassType(frame->_class) + '.' + GetFullMethodType(frame->_method));
					PRINTPREVFRAME;
					return;
				}
			}
			// if method is abstract, it cannot be called
			if (frame->_method->isAbstract())
			{
				errors |= ERROR::ABSTRACT_MEMBER_CALL;
				DisplayError("trying to call abstract method: " + GetFullClassType(frame->_class) + '.' + GetFullMethodType(frame->_method));
				PRINTPREVFRAME;
				return;
			}
			if (frame->_method->isStaticConstructor())
			{
				if (frame->_class->staticConstructorCalled)
				{
					errors |= ERROR::INVALID_METHOD_CALL;
					DisplayError("static constructor of class cannot be called: " + GetFullClassType(frame->_class) + '.' + GetFullMethodType(frame->_method));
					PRINTPREVFRAME;
					return;
				}
				else
				{
					frame->_class->staticConstructorCalled = true;
				}
			}
			if (frame->_class->hasStaticConstructor() && !frame->_class->staticConstructorCalled)
			{
				std::string constructor = frame->_class->name + "_0static";
				CallPath staticConstructorFrame;
				staticConstructorFrame.SetNamespace(&frame->_namespace->name);
				staticConstructorFrame.SetClass(&frame->_class->name);
				staticConstructorFrame.SetMethod(&constructor);
				callStack.push(std::move(staticConstructorFrame));
				objectStack.push_back(frame->_class->wrapper);
				StartNewStackFrame();
				if (errors != 0)
				{
					DisplayError("static constructor caused a fatal error: " + GetFullClassType(frame->_class) + '.' + GetMethodActualName(constructor) + "()");
					return;
				}
				objectStack.pop_back();
			}

			// if class is system class, call must be delegated to SystemCall function
			if (frame->_class->isSystem())
			{
				PerformSystemCall(frame->_class, frame->_method, frame.get());
				return;
			}

			// each method body begins with this opcode
			if (frame->_method->body[frame->offset++] != OPCODE::PUSH_STACKFRAME)
			{
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("PUSH_STACKFRAME opcode always expected in the beginning of method body");
				PRINTFRAME;
				return;
			}

			// read all parameters of method and add them to frame as const locals
			for (auto it = frame->_method->parameters.rbegin(); it != frame->_method->parameters.rend(); it++)
			{
				if (objectStack.empty())
				{
					errors |= ERROR::OBJECTSTACK_EMPTY;
					DisplayError("object stack does not contain enough parameters for method call");
					PRINTFRAME;
					return;
				}
				frame->locals[*it] = { objectStack.back(), false };
				objectStack.pop_back();
			}
			// `this` must be first parameter of any non-static non-constructor method (is added by compiler)
			if (!frame->_method->isStatic() && !frame->_method->isConstructor())
			{
				if (frame->_method->parameters.empty() || frame->_method->parameters.front() != "this")
				{
					errors |= ERROR::INVALID_METHOD_SIGNATURE;
					DisplayError("first parameter of non-static method must always be equal to `this`");
					PRINTFRAME;
					return;
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
			if (frame->_method->isConstructor())
			{
				// constructor cannot be called if the class is statics
				if (frame->_class->isStatic())
				{
					errors |= ERROR::INVALID_METHOD_CALL;
					DisplayError("can not create instance of static class: " + GetFullClassType(frame->_class));
					PRINTPREVFRAME;
					return;
				}
				else
				{
					frame->classObject = AllocClassObject(frame->_class);
					frame->locals["this"].object = frame->classObject;
				}
			}
			while (frame->offset < frame->_method->body.size())
			{
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
				#define ALU_1(op) case op: PerformALUCall(op, 1, frame.get()); break
				ALU_1(OPCODE::NEGATION_OP);
				ALU_1(OPCODE::NEGATIVE_OP);
				ALU_1(OPCODE::POSITIVE_OP);
				#define ALU_2(op) case op: PerformALUCall(op, 2, frame.get()); break
				ALU_2(OPCODE::SUM_OP);
				ALU_2(OPCODE::SUB_OP);
				ALU_2(OPCODE::MULT_OP);
				ALU_2(OPCODE::DIV_OP);
				ALU_2(OPCODE::MOD_OP);
				ALU_2(OPCODE::POWER_OP);
				ALU_2(OPCODE::CMP_EQ);
				ALU_2(OPCODE::CMP_NEQ);
				ALU_2(OPCODE::CMP_L);
				ALU_2(OPCODE::CMP_G);
				ALU_2(OPCODE::CMP_LE);
				ALU_2(OPCODE::CMP_GE);
				ALU_2(OPCODE::CMP_AND);
				ALU_2(OPCODE::CMP_OR);
				ALU_2(OPCODE::ASSIGN_OP);
				#undef ALU_2
				#undef ALU_1

				case (OPCODE::GET_INDEX): // to do
				{
					if (objectStack.size() < 2)
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						DisplayError("not enough parameters in stack for get_index call: " + (!objectStack.empty() ? objectStack.back()->ToString() : ""));
						PRINTFRAME;
						return;
					}
					BaseObject* object = objectStack.back();
					if (object->type == Type::UNKNOWN) object = SearchForObject(*object->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (object == nullptr) return;
					object = GetUnderlyingObject(object);

					objectStack.pop_back();
					BaseObject* index = objectStack.back();
					if(index->type == Type::UNKNOWN) index = SearchForObject(*index->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (index == nullptr) return;

					objectStack.back() = object;
					objectStack.push_back(index);
					if (object->type == Type::CLASS_OBJECT)
					{
						InvokeObjectMethod("GetByIndex_2", reinterpret_cast<ClassObject*>(object));
					}
					break;
				}
				case (OPCODE::CALL_FUNCTION):
				{
					uint8_t paramSize = ReadOPCode(frame->_method->body, frame->offset);
					if (objectStack.size() < paramSize + 2u) // function object + caller object
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						DisplayError("not enough parameters in stack for function call: " + (!objectStack.empty() ? objectStack.back()->ToString() : ""));
						PRINTFRAME;
						return;
					}
					if (objectStack.back()->type != Type::UNKNOWN)
					{
						BaseObject* obj = objectStack.back();
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("expected method name, found object: " + obj->ToString());
						return;
					}
					for (size_t i = 0; i < paramSize; i++)
					{
						size_t index = objectStack.size() - i - 2;
						BaseObject* obj = objectStack[index];
						if (obj->type == Type::UNKNOWN)
						{
							objectStack[index] = SearchForObject(*obj->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
							if (objectStack[index] == nullptr) return;
						}
					}
					CallPath newFrame;
					BaseObject* caller = objectStack[objectStack.size() - paramSize - 2];
					if (caller->type == Type::UNKNOWN)
					{
						caller = SearchForObject(*caller->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
					}
					if (caller == nullptr) return;

					if (caller->type == Type::LOCAL)
					{
						caller = reinterpret_cast<LocalObject*>(caller)->ref.object;
					}
					if (caller->type == Type::CLASS_OBJECT)
					{
						ClassObject* object = reinterpret_cast<ClassObject*>(caller);
						UnknownObject* function = reinterpret_cast<UnknownObject*>(objectStack.back());
						std::string objectFunctionName = GetMethodActualName(*objectStack.back()->GetName()) + '_' + std::to_string(paramSize + 1);
						if (object->type->methods.find(objectFunctionName) != object->type->methods.end())
						{
							objectStack[objectStack.size() - paramSize - 2] = object;
							frame->localStorage.push_back(std::make_unique<std::string>(objectFunctionName));
							function->ref = frame->localStorage.back().get();
						}
						newFrame.SetNamespace(&object->type->namespaceName);
						newFrame.SetClass(&object->type->name);
						newFrame.SetMethod(objectStack.back()->GetName());
						callStack.push(newFrame);
					}
					else if (caller->type == Type::CLASS)
					{
						ClassWrapper* object = reinterpret_cast<ClassWrapper*>(caller);
						objectStack[objectStack.size() - paramSize - 2] = object;
						newFrame.SetNamespace(&object->type->namespaceName);
						newFrame.SetClass(&object->type->name);
						newFrame.SetMethod(objectStack.back()->GetName());
						callStack.push(newFrame);
					}
					else if (caller->type == Type::NAMESPACE)
					{
						UnknownObject* function = reinterpret_cast<UnknownObject*>(objectStack.back());
						std::string className = GetMethodActualName(*objectStack.back()->GetName());
						const NamespaceType* ns = reinterpret_cast<NamespaceWrapper*>(caller)->type;
						auto classIt = ns->classes.find(className);
						if (classIt == ns->classes.end())
						{
							errors |= ERROR::INVALID_STACKOBJECT;
							DisplayError("class `" + className + "` was not found in namespace: " + ns->name);
							PRINTFRAME;
							return;
						}
						else if (classIt->second.isInternal() && ns->name != frame->_namespace->name)
						{
							errors |= ERROR::PRIVATE_MEMBER_ACCESS;
							DisplayError("trying to access namespace internal member: " + GetFullClassType(&classIt->second));
							PRINTFRAME;
							return;
						}
						objectStack[objectStack.size() - paramSize - 2] = classIt->second.wrapper;
						newFrame.SetNamespace(caller->GetName());
						newFrame.SetClass(&classIt->second.name);
						newFrame.SetMethod(objectStack.back()->GetName());
						callStack.push(newFrame);
					}
					else
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("caller of method was neither class object nor class type");
						DisplayExtra("called method name: " + *objectStack.back()->GetName());
						DisplayExtra("caller was: " + caller->ToString());
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
						DisplayError("not enough objects in stack to get member");
						if (!objectStack.empty()) DisplayExtra("last one is: " + objectStack.back()->ToString());
						PRINTFRAME;
						return;
					}
					BaseObject* member = objectStack.back();
					objectStack.pop_back();
					BaseObject* calledObject = objectStack.back();
					objectStack.pop_back();
					const std::string& callerName = *calledObject->GetName();
					if (calledObject->type == Type::UNKNOWN)
					{
						calledObject = SearchForObject(callerName, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					}
					if (calledObject == nullptr) return;

					BaseObject* memberObject = calledObject->GetMember(*member->GetName());
					if (memberObject == nullptr)
					{
						errors |= ERROR::MEMBER_NOT_FOUND;
						DisplayError("member was not found: " + callerName + '.' + *member->GetName());
						PRINTFRAME;
						return;
					}
					if (memberObject->type == Type::ATTRIBUTE)
					{
						const AttributeType* type = reinterpret_cast<AttributeObject*>(memberObject)->type;
						if (!type->isPublic() && reinterpret_cast<ClassObject*>(calledObject)->type != frame->_class)
						{
							const ClassType* classType = nullptr;
							if (type->isStatic())
							{
								classType = reinterpret_cast<ClassWrapper*>(calledObject)->type;
							}
							else
							{
								classType = reinterpret_cast<ClassObject*>(calledObject)->type;
							}
							errors |= ERROR::PRIVATE_MEMBER_ACCESS;
							DisplayError("trying to access class private member: " + GetFullClassType(classType) + '.' + type->name);
							PRINTFRAME;
							return;
						}
					}
					objectStack.push_back(memberObject);
					break;
				}
				case (OPCODE::POP_TO_RETURN):
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						DisplayError("object stack is empty, but `return` instruction called");
						PRINTFRAME;
					}
					else
					{
						BaseObject* obj = objectStack.back();
						if (obj->type == Type::UNKNOWN)
						{
							obj = SearchForObject(*objectStack.back()->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
						}
						if (obj == nullptr)
						{
							errors |= ERROR::OBJECT_NOT_FOUND;
							DisplayError("cannot return object from function: object not found: " + objectStack.back()->ToString());
							PRINTFRAME;
							return;
						}
						objectStack.back() = obj;
					}
					callStack.pop();
					return;
				case (OPCODE::ALLOC_VAR):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if (ValidateHashValue(hash, frame->_method->dependencies.size()))
					{
						objectStack.push_back(AllocLocal(
							frame->_method->dependencies[hash],
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
							frame->_method->dependencies[hash],
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
						DisplayError("object stack is empty, but jump_if_true needs boolean");
						PRINTFRAME;
						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (object->type == Type::CLASS_OBJECT)
					{
						InvokeObjectMethod("ToBoolean_1", reinterpret_cast<ClassObject*>(object));
						if (errors != 0)
						{
							DisplayError("could not convert class object into boolean: " + GetFullClassType(reinterpret_cast<ClassObject*>(object)->type));
							PRINTFRAME;
							return;
						}
						object = objectStack.back();
						objectStack.pop_back();
					}
					if (object->type == Type::TRUE)
						frame->offset = frame->_method->labels[label];
					else if (object->type != Type::FALSE && object->type != Type::NULLPTR)
					{
						DisplayError("object cannot be implicitly converted to boolean");
						PRINTFRAME;
						return;
					}
					break;
				}
				case (OPCODE::JUMP_IF_FALSE):
				{
					uint16_t label = ReadLabel(frame->_method->body, frame->offset);
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						DisplayError("object stack is empty, but jump_if_false needs boolean");
						PRINTFRAME;
						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (object->type == Type::CLASS_OBJECT)
					{
						InvokeObjectMethod("ToBoolean_1", reinterpret_cast<ClassObject*>(object));
						if (errors != 0)
						{
							DisplayError("could not convert class object into boolean: " + GetFullClassType(reinterpret_cast<ClassObject*>(object)->type));
							PRINTFRAME;
							return;
						}
						object = objectStack.back();
						objectStack.pop_back();
					}
					if (object->type == Type::FALSE || object->type == Type::NULLPTR)
						frame->offset = frame->_method->labels[label];
					else if (object->type != Type::TRUE)
					{
						DisplayError("object cannot be implicitly converted to boolean");
						PRINTFRAME;
						return;
					}
					break;
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
					if (frame->_method->isConstructor())
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
						DisplayError("POP_STACK_TOP instruction called, but object stack was empty");
						PRINTFRAME;
						return;
					}
					else if (objectStack.back()->type == Type::UNKNOWN)
					{
						const std::string& name = *objectStack.back()->GetName();
						objectStack.back() = SearchForObject(name, frame->locals, frame->_method, frame->classObject, frame->_namespace);
						if (objectStack.back() == nullptr) return;
					}
					objectStack.pop_back();
					break;
				default:
					errors |= ERROR::INVALID_OPCODE;
					DisplayError("opcode " + ToString(op) + " was found, but not expected");
					break;
				}
			}
			errors |= ERROR::INVALID_STACKFRAME_OFFSET;
			DisplayError("execution of method went out of frame");
			PRINTFRAME;
		}

		void VirtualMachine::PerformSystemCall(const ClassType* _class, const MethodType* _method, Frame* frame)
		{
			if (objectStack.empty())
			{
				errors |= ERROR::OBJECTSTACK_EMPTY;
				DisplayError("object stack was empty but expected to have SystemCall arguments");
				DisplayExtra("execution interrupted in method: " + GetFullClassType(_class) + '.' + GetFullMethodType(_method));
				return;
			}
			if (_class->name == "Console")
			{
				if (_method->name == "Print_1" || _method->name == "PrintLine_1")
				{
					std::ostream& out = *config.streams.out;
					BaseObject* object = objectStack.back();
					object = GetUnderlyingObject(object);
					objectStack.pop_back();
					switch (object->type)
					{
					case Type::NAMESPACE:
					{
						out << "namespace " << *object->GetName();
						break;
					}
					case Type::CLASS:
					{
						ClassWrapper* c = reinterpret_cast<ClassWrapper*>(object);
						out << "class " << GetFullClassType(c->type);
						break;
					}
					case Type::ATTRIBUTE:
					{
						AttributeObject* attr = reinterpret_cast<AttributeObject*>(object);
						objectStack.push_back(attr->object);
						PerformSystemCall(_class, _method, frame);
						return; // no PrintLine check, because it will happen inside recursion call
					}
					case Type::CLASS_OBJECT:
					{
						ClassObject* classObject = reinterpret_cast<ClassObject*>(object);
						if (GetMethodOrNull(classObject->type, "ToString_1") != nullptr)
						{
							objectStack.push_back(object);
							InvokeObjectMethod("ToString_1", classObject);
							if (errors == 0) PerformSystemCall(_class, _method, frame);
							return; // no PrintLine check, because it will happen inside recursion call
						}
						else
						{
							out << GetFullClassType(classObject->type) << " instance";
							break;
						}
					}
					default:
						out << object->ToString();
						break;
					}
					if (_method->name == "PrintLine_1")
					{
						out << std::endl;
					}
					objectStack.pop_back();
					objectStack.push_back(AllocTrue());
				}
				else if (_method->name == "Read_0")
				{
					objectStack.pop_back(); // delete console reference
					StringObject::InnerType str;
					*config.streams.in >> str;
					StringObject* strObj = reinterpret_cast<StringObject*>(AllocString(""));
					strObj->value = std::move(str);
					objectStack.push_back(strObj);
				}
				else if (_method->name == "ReadInt_0")
				{
					objectStack.pop_back(); // delete console reference
					std::string str;
					*config.streams.in >> str;
					BaseObject* intObj = AllocInteger(str);
					objectStack.push_back(intObj);
				}
				else if (_method->name == "ReadFloat_0")
				{
					objectStack.pop_back();  // delete console reference
					std::string str;
					*config.streams.in >> str;
					BaseObject* floatObj = AllocFloat(str);
					objectStack.push_back(floatObj);
				}
				else if (_method->name == "ReadLine_0")
				{
					objectStack.pop_back();  // delete console reference
					std::string str;
					std::getline(*config.streams.in, str);
					StringObject* strObj = reinterpret_cast<StringObject*>(AllocString(""));
					strObj->value = std::move(str);
					objectStack.push_back(strObj);
				}
				else if (_method->name == "ReadBool_0")
				{
					objectStack.pop_back();  // delete console reference
					std::string str;
					*config.streams.in >> str;
					if (str == "1" || str == "True" || str == "true")
					{
						objectStack.push_back(AllocTrue());
					}
					else
					{
						objectStack.push_back(AllocFalse());
					}
				}
			}
			else if (_class->name == "Reflection")
			{
				if (_method->name == "GetType_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // deleting reflection class reference
					switch (object->type)
					{
					case Type::CLASS_OBJECT:
						objectStack.push_back(reinterpret_cast<ClassObject*>(object)->type->wrapper);
						break;
					case Type::CLASS:
						objectStack.push_back(object);
						break;
					case Type::ATTRIBUTE:
						objectStack.push_back(nullptr);
						objectStack.push_back(reinterpret_cast<AttributeObject*>(object)->object);
						callStack.push(CallPath());
						PerformSystemCall(_class, _method, frame);
						break;
					case Type::INTEGER:
						objectStack.push_back(assembly.namespaces["System"].classes["Integer"].wrapper);
						break;
					case Type::FLOAT:
						objectStack.push_back(assembly.namespaces["System"].classes["Float"].wrapper);
						break;
					case Type::STRING:
						objectStack.push_back(assembly.namespaces["System"].classes["String"].wrapper);
						break;
					case Type::TRUE:
						objectStack.push_back(assembly.namespaces["System"].classes["True"].wrapper);
						break;
					case Type::FALSE:
						objectStack.push_back(assembly.namespaces["System"].classes["False"].wrapper);
						break;
					case Type::NULLPTR:
						objectStack.push_back(assembly.namespaces["System"].classes["Null"].wrapper);
						break;
					default:
						errors |= ERROR::INVALID_CALL_ARGUMENT;
						DisplayError("class object expected as a parameter");
						PRINTFRAME_2(_class, _method);
						break;
					}
				}
				else if (_method->name == "CreateInstance_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // deleting reflection class reference
					if (object->type != Type::CLASS)
					{
						errors |= ERROR::INVALID_CALL_ARGUMENT;
						DisplayError("class type expected as a parameter");
						PRINTFRAME_2(_class, _method);
						return;
					}
					std::string constructor = *object->GetName() + "_0";
					const ClassType* classType = reinterpret_cast<ClassWrapper*>(object)->type;
					auto methodIt = classType->methods.find(constructor);
					if (methodIt == classType->methods.end())
					{
						errors |= ERROR::MEMBER_NOT_FOUND;
						DisplayError("class type provided does not have constructor with no parameters: " + GetFullClassType(classType));
						if (classType->isStatic())
						{
							DisplayExtra(GetFullClassType(classType) + " is static class, so its instance cannot be created");
						}
						PRINTFRAME_2(_class, _method);
					}
					else
					{
						CallPath newFrame;
						newFrame.SetNamespace(&classType->namespaceName);
						newFrame.SetClass(&classType->name);
						newFrame.SetMethod(&constructor);
						callStack.push(std::move(newFrame));
						objectStack.push_back(classType->wrapper);
						StartNewStackFrame();
					}
				}
			} 
			else if (_class->name == "Array")
			{
				if (_method->name == "Array_0" || _method->name == "Array_1")
				{
					size_t arraySize = 0;
					BaseObject* size = objectStack.back();
					objectStack.pop_back();
					if (size->type == Type::INTEGER)
					{
						objectStack.pop_back();
						IntegerObject::InnerType& value = reinterpret_cast<IntegerObject*>(size)->value;
						if (value >= 0 && value < (unsigned long long)std::numeric_limits<size_t>::max())
							arraySize = std::stol(value.to_string());
						else
						{
							errors |= ERROR::INVALID_CALL_ARGUMENT;
							DisplayError("cannot create Array instance with size: " + value.to_string());
							PRINTFRAME_2(_class, _method);
						}
					}
					ClassObject* arr = AllocClassObject(_class);
					arr->attributes["array"]->object = AllocArray(arraySize);
					objectStack.push_back(arr);
				}
				else if (_method->name == "GetByIndex_2" || _method->name == "GetByIter_2")
				{
					size_t idx = 0;
					BaseObject* index = objectStack.back();
					objectStack.pop_back(); // pop index
					ClassObject* arrayClass = reinterpret_cast<ClassObject*>(objectStack.back());
					ArrayObject* arrayObject = reinterpret_cast<ArrayObject*>(arrayClass->attributes["array"]->object);
					objectStack.pop_back(); // pop array object

					if (index->type == Type::INTEGER)
					{
						IntegerObject::InnerType& value = reinterpret_cast<IntegerObject*>(index)->value;
						if (value >= 0 && value < (unsigned long long)arrayObject->array.size())
							idx = std::stol(value.to_string());
						else
						{
							errors |= ERROR::INVALID_CALL_ARGUMENT;
							DisplayError("cannot access Array member with index = " + value.to_string());
							PRINTFRAME_2(_class, _method);
						}
					}
					else
					{
						errors |= ERROR::INVALID_CALL_ARGUMENT;
						DisplayError("invalid argument was passed as array index: " + index->ToString());
						PRINTFRAME_2(_class, _method);
					}
					objectStack.push_back(AllocLocal(arrayClass->type->name, arrayObject->array[idx]));
				}
				else if (_method->name == "Size_1")
				{
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = reinterpret_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = reinterpret_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
					objectStack.push_back(AllocInteger(std::to_string(objects.size())));
				}
				else if (_method->name == "ToString_1")
				{
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = reinterpret_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = reinterpret_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;

					objectStack.push_back(AllocString("["));
					for (int i = 0; i < int(objects.size()); i++)
					{
						objectStack.push_back(objects[i].object);
						ALUinIncrMode = true;
						PerformALUCall(OPCODE::SUM_OP, 2, frame);
						
						if (i != int(objects.size()) - 1)
						{
							reinterpret_cast<StringObject*>(objectStack.back())->value += ", ";
						}
					}
					reinterpret_cast<StringObject*>(objectStack.back())->value += ']';
				}
				else if (_method->name == "Begin_1")
				{
					objectStack.pop_back(); // array object
					objectStack.push_back(AllocInteger("0"));
				}
				else if (_method->name == "End_1")
				{
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = reinterpret_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = reinterpret_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
					objectStack.push_back(AllocInteger(std::to_string(objects.size())));
				}
				else if (_method->name == "Append_2")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back(); // pop object
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = reinterpret_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = reinterpret_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;

					objects.push_back({ object, false }); // OBJECT MUST NOT BE DESTROYED
					objectStack.push_back(object);
				}
				else if (_method->name == "Next_2")
				{
					BaseObject* iter = objectStack.back(); 
					objectStack.pop_back(); // pop iter
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					if (iter->type != Type::INTEGER)
					{
						DisplayError("Invalid iterator was passed to Array.Next(this, iter) method: " + iter->ToString());
						PRINTFRAME_2(_class, _method);
						return;
					}
					IntegerObject::InnerType& iterValue = reinterpret_cast<IntegerObject*>(iter)->value;
					objectStack.push_back(AllocInteger((iterValue + 1).to_string()));
				}
				else
				{
					DisplayError("Array class does not contains method: " + GetFullMethodType(_method));
					PRINTFRAME_2(_class, _method);
				}
			}
			else if (_class->name == "Integer" && _method->name == "Integer_0")
			{
				objectStack.pop_back();
				objectStack.push_back(AllocInteger("0"));
			}
			else if (_class->name == "Float" && _method->name == "Float_0")
			{
				objectStack.pop_back();
				objectStack.push_back(AllocFloat("0.0"));
			}
			else if (_class->name == "String" && _method->name == "String_0")
			{
				objectStack.pop_back();
				objectStack.push_back(AllocString(""));
			}
			else if (_class->name == "True" && _method->name == "True_0")
			{
				objectStack.pop_back();
				objectStack.push_back(AllocTrue());
			}
			else if (_class->name == "False" && _method->name == "False_0")
			{
				objectStack.pop_back();
				objectStack.push_back(AllocFalse());
			}
			else if (_class->name == "Null" && _method->name == "Null_0")
			{
				objectStack.pop_back();
				objectStack.push_back(AllocNull());
			}
			else
			{
				errors |= ERROR::INVALID_METHOD_CALL;
				DisplayError("Invalid method was passed to PerformSystemCall() method: " + GetFullClassType(_class) + '.' + GetFullMethodType(_method));
			}
			callStack.pop();
		}

		void VirtualMachine::PerformALUCall(OPCODE op, size_t parameters, Frame* frame)
		{
			if (objectStack.size() < parameters)
			{
				errors |= ERROR::OBJECTSTACK_EMPTY;
				DisplayError("object stack was empty on VM ALU call");
				PRINTFRAME;
				return;
			}	

			BaseObject* object = nullptr;
			BaseObject* value = nullptr;
			if (parameters == 2)
			{
				value = objectStack.back();
				objectStack.pop_back();
				if (value->type == Type::UNKNOWN)
				{
					const std::string* valueName = value->GetName();
					value = SearchForObject(*value->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (value == nullptr) return;
				}
				value = GetUnderlyingObject(value);
			}
			object = objectStack.back();
			objectStack.pop_back();
			if (object->type == Type::UNKNOWN)
			{
				auto localIt = frame->locals.find(*object->GetName());
				if (localIt != frame->locals.end())
				{
					object = AllocLocal(localIt->first, localIt->second);
				}
				else
				{
					const std::string* objectName = object->GetName();
					object = SearchForObject(*object->GetName(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (object == nullptr) return;
				}
			}
			BaseObject** objectReference = nullptr;
			switch (object->type)
			{
			case Type::LOCAL:
			{
				LocalObject* local = reinterpret_cast<LocalObject*>(object);
				if (local->ref.isConst && local->ref.object->type != Type::NULLPTR)
				{
					errors |= ERROR::CONST_MEMBER_MODIFICATION;
					DisplayError("trying to modify const local variable: " + local->nameRef + " = " + value->ToString());
					PRINTFRAME;
					return;
				}
				objectReference = &local->ref.object;
				break;
			}
			case Type::ATTRIBUTE:
			{
				AttributeObject* attr = reinterpret_cast<AttributeObject*>(object);
				if (attr->type->isConst() && attr->object->type != Type::NULLPTR)
				{
					errors |= ERROR::CONST_MEMBER_MODIFICATION;
					DisplayError("trying to modify const class attribute: " + attr->type->name);
					PRINTFRAME;
					return;
				}
				objectReference = &attr->object;
				break;
			}
			case Type::INTEGER:
			case Type::STRING:
			case Type::FLOAT:
			case Type::CLASS:
				objectReference = &object;
				break;
			default:
				errors |= ERROR::INVALID_STACKOBJECT;
				DisplayError("trying to assign value to object with invalid type: <" + object->ToString() + "> = " + value->ToString());
				PRINTFRAME;
				return;
			}
			if (op == OPCODE::ASSIGN_OP)
			{
				*objectReference = value;
				objectStack.push_back(object);
				return;
			}

			// call assign after operations in this method
			if (ALUinIncrMode)
			{
				objectStack.push_back(object);
			}

			switch ((*objectReference)->type)
			{
			case Type::CLASS_OBJECT:
			{
				objectStack.push_back(*objectReference);
				if (parameters == 2) objectStack.push_back(value);
				ClassObject* classObject = reinterpret_cast<ClassObject*>(*objectReference);
				PerformALUcallClassObject(classObject, op, frame);
			}
			break;
			case Type::INTEGER:
			{
				IntegerObject* integer = reinterpret_cast<IntegerObject*>(*objectReference);
				IntegerObject::InnerType* integerValue = nullptr;
				if (parameters == 2)
				{
					value = GetUnderlyingObject(value);
					if (value->type == Type::CLASS_OBJECT)
					{
						ClassObject* valueClassObject = reinterpret_cast<ClassObject*>(value);
						objectStack.push_back(value);
						InvokeObjectMethod("ToInteger_1", valueClassObject);
						if (errors == 0 && objectStack.back()->type == Type::INTEGER)
						{
							value = objectStack.back();
							objectStack.pop_back();
						}
						else
						{
							errors |= ERROR::INVALID_METHOD_CALL;
							DisplayError("cannot convert class object to integer: " + GetFullClassType(valueClassObject->type) + ".ToInteger() method not found");
							PRINTFRAME;
							return;
						}
					}
					if (value->type == Type::INTEGER)
					{
						integerValue = &reinterpret_cast<IntegerObject*>(value)->value;
					}
					else if (value->type == Type::FLOAT)
					{
						FloatObject::InnerType* floatValue = &reinterpret_cast<FloatObject*>(value)->value;
						FloatObject* intConverted = reinterpret_cast<FloatObject*>(AllocFloat(std::to_string(integer->value.to_double())));
						PerformALUcallFloats(intConverted, floatValue, op, frame);
						break;
					}
					else
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("cannot convert object passed to ALU to integer: " + value->ToString());
						PRINTFRAME;
						return;
					}
				}
				PerformALUcallIntegers(integer, integerValue, op, frame);
			}
			break;
			case Type::STRING:
			{
				StringObject::InnerType tmpString;
				StringObject* str = reinterpret_cast<StringObject*>(*objectReference);
				StringObject::InnerType* stringValue = nullptr;
				if (parameters == 2)
				{
					value = GetUnderlyingObject(value);
					if (value->type == Type::CLASS_OBJECT)
					{
						ClassObject* valueClassObject = reinterpret_cast<ClassObject*>(value);
						objectStack.push_back(value);
						InvokeObjectMethod("ToString_1", valueClassObject);
						if (errors == 0 && objectStack.back()->type == Type::STRING)
						{
							value = objectStack.back();
							objectStack.pop_back();
						}
						else
						{
							errors |= ERROR::INVALID_METHOD_CALL;
							DisplayError("cannot convert class object to string: " + GetFullClassType(valueClassObject->type) + ".ToString() method not found");
							PRINTFRAME;
							return;
						}
					}
					if (value->type == Type::STRING)
					{
						stringValue = &reinterpret_cast<StringObject*>(value)->value;
					}
					else if (value->type == Type::INTEGER)
					{
						IntegerObject::InnerType* integer = &reinterpret_cast<IntegerObject*>(value)->value;
						PerformALUcallStringInteger(str, integer, op, frame);
						break;
					}
					else if (value->type == Type::FLOAT)
					{
						tmpString = value->ToString();
						stringValue = &tmpString;
					}
					else if (value->type == Type::TRUE)
					{
						std::string val = "true";
						StringObject::InnerType* strTrue = &val;
						PerformALUcallStrings(str, strTrue, op, frame);
						break;
					}
					else if (value->type == Type::FALSE)
					{
						std::string val = "false";
						StringObject::InnerType* strFalse = &val;
						PerformALUcallStrings(str, strFalse, op, frame);
						break;
					}
					else if (value->type == Type::NULLPTR)
					{
						std::string val = "null";
						StringObject::InnerType* strNull = &val;
						PerformALUcallStrings(str, strNull, op, frame);
						break;
					}
					else
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("cannot convert object passed to ALU to string: " + value->ToString());
						PRINTFRAME;
						return;
					}
				}
				PerformALUcallStrings(str, stringValue, op, frame);
			}
			break;
			case Type::FLOAT:
			{
				FloatObject* floatObject = reinterpret_cast<FloatObject*>(*objectReference);
				FloatObject::InnerType* floatValue = nullptr;
				FloatObject::InnerType tmpFloat;
				if (parameters == 2)
				{
					value = GetUnderlyingObject(value);
					if (value->type == Type::CLASS_OBJECT)
					{
						ClassObject* valueClassObject = reinterpret_cast<ClassObject*>(value);
						objectStack.push_back(value);
						InvokeObjectMethod("ToFloat_1", valueClassObject);
						if (errors == 0 && objectStack.back()->type == Type::FLOAT)
						{
							value = objectStack.back();
							objectStack.pop_back();
						}
						else
						{
							errors |= ERROR::INVALID_METHOD_CALL;
							DisplayError("cannot convert class object to float: " + GetFullClassType(valueClassObject->type) + ".ToFloat() method not found");
							PRINTFRAME;
							return;
						}
					}
					if (value->type == Type::FLOAT)
					{
						floatValue = &reinterpret_cast<FloatObject*>(value)->value;
					}
					else if (value->type == Type::INTEGER)
					{
						IntegerObject* integer = reinterpret_cast<IntegerObject*>(value);
						tmpFloat = integer->value.to_double();
						floatValue = &tmpFloat;
					}
					else
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("cannot convert object passed to ALU to string: " + value->ToString());
						PRINTFRAME;
						return;
					}
				}
				PerformALUcallFloats(floatObject, floatValue, op, frame);
			}
			break;
			case Type::CLASS:
			{
				ClassWrapper* classWrap = reinterpret_cast<ClassWrapper*>(*objectReference);
				const ClassType* classType = nullptr;
				if (parameters == 2)
				{
					if (value->type == Type::ATTRIBUTE)
					{
						value = reinterpret_cast<AttributeObject*>(value)->object;
					}
					if (value->type == Type::CLASS)
					{
						classType = reinterpret_cast<ClassWrapper*>(value)->type;
					}
					else
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("cannot convert object passed to ALU to string: " + value->ToString());
						PRINTFRAME;
						return;
					}
				}
				PerformALUcallClassTypes(classWrap, classType, op, frame);
			}
			break;
			}

			if (ALUinIncrMode)
			{
				PerformALUCall(OPCODE::ASSIGN_OP, 2, frame);
				ALUinIncrMode = false; // resets after call
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
						std::unique_ptr<AttributeObject> staticAttr = std::make_unique<AttributeObject>(&attr.second);
						staticAttr->object = AllocNull();
						c.staticInstance->attributes[attr.second.name] = std::move(staticAttr);
					}
				}
			}
		}

		void VirtualMachine::AddSystemNamespace()
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			#define BEGIN_NAMESPACE(_name) NamespaceType _name; \
			_name.name = #_name; \
			NamespaceType* CURRENT_NAMESPACE = &_name; \
			ClassType* CURRENT_CLASS = nullptr

			#define BEGIN_CLASS(_name) ClassType _name; \
			_name.name = #_name; \
			_name.namespaceName = CURRENT_NAMESPACE->name; \
			_name.modifiers |= ClassType::Modifiers::STATIC | ClassType::Modifiers::SYSTEM; \
			CURRENT_CLASS = &_name

			#define METHOD(_name, _method, params) MethodType _method; \
			_method.name = #_name "_" #params; \
			_method.modifiers |= MethodType::Modifiers::PUBLIC;

			#define INSERT_METHOD(_name, _method, params) CURRENT_CLASS->methods.insert({ #_name "_" #params, std::move(_method) })

			#define CONSTRUCTOR(_name, params) METHOD(_name, _name##params, params); \
			_name##params.modifiers |= MethodType::Modifiers::CONSTRUCTOR

			#define CONSTRUCTOR_0(_name) CONSTRUCTOR(_name, 0); \
			INSERT_METHOD(_name, _name##0, 0)

			#define CONSTRUCTOR_1(_name, param1) CONSTRUCTOR(_name, 1); \
			_name##1.parameters.push_back(#param1); \
			INSERT_METHOD(_name, _name##1, 1)

			#define STATIC_METHOD(_name, _method, params) METHOD(_name, _method, params); \
			_method.modifiers |= MethodType::Modifiers::STATIC; \
			INSERT_METHOD(_name, _name##params, params)

			#define END_CLASS(_class) CURRENT_NAMESPACE->classes.insert({ #_class, std::move(_class) }); CURRENT_CLASS = nullptr

			#define END_NAMESPACE(_name) auto assemblyIt = assembly.namespaces.find(#_name); \
			if(assemblyIt != assembly.namespaces.end()) assembly.namespaces.erase(assemblyIt); \
			assembly.namespaces.insert({ #_name, std::move(_name) }); CURRENT_NAMESPACE = nullptr

			#define STATIC_METHOD_0(_method) STATIC_METHOD(_method, _method##0, 0); \
			INSERT_METHOD(_method, _method##0, 0)

			#define STATIC_METHOD_1(_method, param1) STATIC_METHOD(_method, _method##1, 1); \
			_method##1.parameters.push_back(#param1); \
			INSERT_METHOD(_method, _method##1, 1)

			#define STATIC_METHOD_2(_method, param1, param2) STATIC_METHOD(_method, _method##2, 2); \
			_method##2.parameters.push_back(#param1); \
			_method##2.parameters.push_back(#param2); \
			INSERT_METHOD(_method, _method##2, 2)

			#define METHOD_0(_method) METHOD(_method, _method##0, 0); \
			INSERT_METHOD(_method, _method##0, 0)

			#define METHOD_1(_method, param1) METHOD(_method, _method##1, 1); \
			_method##1.parameters.push_back(#param1); \
			INSERT_METHOD(_method, _method##1, 1)

			#define METHOD_2(_method, param1, param2) METHOD(_method, _method##2, 2); \
			_method##2.parameters.push_back(#param1); \
			_method##2.parameters.push_back(#param2); \
			INSERT_METHOD(_method, _method##2, 2)

			#define ATTRIBUTE(attr) AttributeType attr; attr.name = #attr; CURRENT_CLASS->objectAttributes.insert({ #attr, std::move(attr) })

			#define PRIMITIVE_CLASS(_name) BEGIN_CLASS(_name); \
			STATIC_METHOD(_name, _name##0, 0); \
			INSERT_METHOD(_name, _name##0, 0); \
			END_CLASS(_name)
			/////////////////////////////////////////////////////////////////////////////////////////////////////

			BEGIN_NAMESPACE(System);

				BEGIN_CLASS(Console);
					STATIC_METHOD_1(Print, value); // outputs value to console
					STATIC_METHOD_1(PrintLine, value); // outputs line and flushes out stream
					STATIC_METHOD_0(Read); // reads string from console
					STATIC_METHOD_0(ReadInt); // reads integer from console
					STATIC_METHOD_0(ReadFloat); // reads float from console
					STATIC_METHOD_0(ReadBool); // reads bool from console (as True/true/1 and False/false/0)
					STATIC_METHOD_0(ReadLine); // reads line as string from console
				END_CLASS(Console);

				BEGIN_CLASS(Reflection);
					STATIC_METHOD_1(GetType, object); // gets object type
					STATIC_METHOD_1(CreateInstance, type); // creates class instance using class type
				END_CLASS(Reflection);

				PRIMITIVE_CLASS(Integer);
				PRIMITIVE_CLASS(Float);
				PRIMITIVE_CLASS(String);
				PRIMITIVE_CLASS(True);
				PRIMITIVE_CLASS(False);
				PRIMITIVE_CLASS(Null);

				BEGIN_CLASS(Array);
					Array.modifiers &= ~ClassType::Modifiers::STATIC;
					CONSTRUCTOR_0(Array);
					CONSTRUCTOR_1(Array, size);
					METHOD_2(Append, this, object);
					METHOD_2(GetByIndex, this, index);
					METHOD_2(GetByIter, this, iter);
					METHOD_2(Next, this, iter);
					METHOD_1(Size, this);
					METHOD_1(ToString, this);
					METHOD_1(Begin, this);
					METHOD_1(End, this);
					ATTRIBUTE(array);
				END_CLASS(Array);

			END_NAMESPACE(System);
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
				DisplayError("hash value of dependency object is invalid");
				return false;
			}
		}

		void VirtualMachine::InvokeObjectMethod(const std::string& methodName, const ClassObject* object)
		{
			const auto method = GetMethodOrNull(object->type, methodName);
			if (method == nullptr)
			{
				errors |= ERROR::MEMBER_NOT_FOUND;
				DisplayError("method name provided to InvokeObjectMethod() function not found");
				DisplayExtra("with " + GetFullClassType(object->type) + " object and method name: " + methodName);
				return;
			}
			if (method->isAbstract() || method->isStatic())
			{
				errors |= ERROR::INVALID_METHOD_SIGNATURE;
				DisplayError("trying to access abstract or static method in InvokeObjectMethod() function");
				DisplayExtra("with " + GetFullClassType(object->type) + " object and method name: " + methodName);
				return;
			}
			if (!method->isPublic())
			{
				errors |= ERROR::PRIVATE_MEMBER_ACCESS;
				DisplayError("trying to access private method in InvokeObjectMethod() function");
				DisplayExtra("with " + GetFullClassType(object->type) + " object and method name: " + methodName);
				return;
			}
			CallPath newFrame;
			newFrame.SetMethod(&methodName);
			newFrame.SetClass(&object->type->name);
			newFrame.SetNamespace(&object->type->namespaceName);
			callStack.push(std::move(newFrame));
			StartNewStackFrame();
		}

		void VirtualMachine::DisplayError(std::string message) const
		{
			*config.streams.error << "[error]: " <<  message << std::endl;
			PrintObjectStack();
		}

		void VirtualMachine::DisplayExtra(std::string message) const
		{
			*config.streams.error << "         " << message << std::endl;
		}

		void VirtualMachine::PrintObjectStack() const
		{
			if (config.execution.allowDebug)
			{
				*config.streams.error << "---------------------------STACK---------------------------\n";
				size_t count = 0;
				for (auto it = objectStack.rbegin(); it != objectStack.rend(); it++, count++)
				{
					*config.streams.error << '[' << count << "] " << (*it != nullptr ? (*it)->ToString() : "<error type>") << std::endl;
				}
				*config.streams.error << "-----------------------------------------------------------\n";
			}
		}

		std::string VirtualMachine::GetFullClassType(const ClassType* type) const
		{
			return type->namespaceName + '.' + type->name;
		}

		std::string VirtualMachine::GetFullMethodType(const MethodType* type) const
		{
			std::string name = GetMethodActualName(type->name) + '(';
			for (int i = 0; i < (int)type->parameters.size() - 1; i++)
			{
				name += type->parameters[i] + ", ";
			}
			if (!type->parameters.empty())
			{
				name += type->parameters.back();
			}
			name += ')';
			return name;
		}

		std::string VirtualMachine::GetMethodActualName(const std::string& methodName) const
		{
			if (methodName.empty()) return "[unnamed]";
			int i = 0;
			for (i = methodName.size() - 1; i >= 0; i--)
			{
				if (methodName[i] == '_') break;
			}
			return std::string(methodName.begin(), methodName.begin() + i);
		}

		void VirtualMachine::PerformALUcallIntegers(IntegerObject* int1, const IntegerObject::InnerType* int2, OPCODE op, Frame* frame)
		{
			IntegerObject* result = reinterpret_cast<IntegerObject*>(AllocInteger("0"));
			switch (op)
			{
			case OPCODE::SUM_OP:
				result->value = int1->value + *int2;
				objectStack.push_back(result);
				break;
			case OPCODE::SUB_OP:
				result->value = int1->value - *int2;
				objectStack.push_back(result);
				break;
			case OPCODE::MULT_OP:
				result->value = int1->value * *int2;
				objectStack.push_back(result);
				break;
			case OPCODE::DIV_OP:
				result->value = int1->value / *int2;
				objectStack.push_back(result);
				break;
			case OPCODE::MOD_OP:
				result->value = int1->value % *int2;
				objectStack.push_back(result);
				break;
			case OPCODE::POWER_OP:
				result->value = momo::pow(int1->value, *int2);
				objectStack.push_back(result);
				break;
			case OPCODE::CMP_EQ:
				if (int1->value == *int2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_NEQ:
				if (int1->value != *int2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_L:
				if (int1->value < *int2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_G:
				if (int1->value > *int2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_LE:
				if (int1->value <= *int2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_GE:
				if (int1->value >= *int2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			default:
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with two integers: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		void VirtualMachine::PerformALUcallStrings(StringObject* str1, const StringObject::InnerType* str2, OPCODE op, Frame* frame)
		{
			switch (op)
			{
			case OPCODE::SUM_OP:
			{
				StringObject* result = reinterpret_cast<StringObject*>(AllocString(""));
				result->value = str1->value + *str2;
				objectStack.push_back(result);
				break;
			}
			case OPCODE::CMP_EQ:
				if (str1->value == *str2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_NEQ:
				if (str1->value != *str2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_L:
				if (str1->value < *str2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_G:
				if (str1->value > *str2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_LE:
				if (str1->value <= *str2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_GE:
				if (str1->value >= *str2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			default:
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with two strings: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		void VirtualMachine::PerformALUcallStringInteger(StringObject* str, const IntegerObject::InnerType* integer, OPCODE op, Frame* frame)
		{
			StringObject* result = reinterpret_cast<StringObject*>(AllocString(""));
			switch (op)
			{
			case OPCODE::MULT_OP:
				for (momo::big_integer i = 0; i < *integer; i += 1)
				{
					result->value += str->value;
				}
				objectStack.push_back(result);
				break;
			case OPCODE::SUM_OP:
				result->value = str->value + integer->to_string();
				objectStack.push_back(result);
				break;
			default:
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with string and integer: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		void VirtualMachine::PerformALUcallClassObject(ClassObject* obj, OPCODE op, Frame* frame)
		{
			switch (op)
			{
			case MSL::VM::NEGATION_OP:
				InvokeObjectMethod("NegationOperator_1", obj);
				break;
			case MSL::VM::NEGATIVE_OP:
				InvokeObjectMethod("NegOperator_2", obj);
				break;
			case MSL::VM::POSITIVE_OP:
				InvokeObjectMethod("PosOperator_2", obj);
				break;
			case MSL::VM::SUM_OP:
				InvokeObjectMethod("SumOperator_2", obj);
				break;
			case MSL::VM::SUB_OP:
				InvokeObjectMethod("SubOperator_2", obj);
				break;
			case MSL::VM::MULT_OP:
				InvokeObjectMethod("MultOperator_2", obj);
				break;
			case MSL::VM::DIV_OP:
				InvokeObjectMethod("DivOperator_2", obj);
				break;
			case MSL::VM::MOD_OP:
				InvokeObjectMethod("ModOperator_2", obj);
				break;
			case MSL::VM::POWER_OP:
				InvokeObjectMethod("PowerOperator_2", obj);
				break;
			case MSL::VM::CMP_EQ:
				InvokeObjectMethod("IsEqual_2", obj);
				break;
			case MSL::VM::CMP_NEQ:
				InvokeObjectMethod("IsNotEqual_2", obj);
				break;
			case MSL::VM::CMP_L:
				InvokeObjectMethod("IsLess_2", obj);
				break;
			case MSL::VM::CMP_G:
				InvokeObjectMethod("IsGreater_2", obj);
				break;
			case MSL::VM::CMP_LE:
				InvokeObjectMethod("IsLessEqual_2", obj);
				break;
			case MSL::VM::CMP_GE:
				InvokeObjectMethod("IsGreaterEqual_2", obj);
				break;
			case MSL::VM::CMP_AND:
				InvokeObjectMethod("AndOperator_2", obj);
				break;
			case MSL::VM::CMP_OR:
				InvokeObjectMethod("OrOperator_2", obj);
				break;
			default:
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid opcode was passed to VM ALU: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		void VirtualMachine::PerformALUcallFloats(FloatObject* f1, const FloatObject::InnerType* f2, OPCODE op, Frame* frame)
		{
			FloatObject* result = reinterpret_cast<FloatObject*>(AllocFloat("0.0"));
			switch (op)
			{
			case OPCODE::SUM_OP:
				result->value = f1->value + *f2;
				objectStack.push_back(result);
				break;
			case OPCODE::SUB_OP:
				result->value = f1->value - *f2;
				objectStack.push_back(result);
				break;
			case OPCODE::MULT_OP:
				result->value = f1->value * *f2;
				objectStack.push_back(result);
				break;
			case OPCODE::DIV_OP:
				result->value = f1->value / *f2;
				objectStack.push_back(result);
				break;
			case OPCODE::POWER_OP:
				result->value = std::pow(f1->value, *f2);
				objectStack.push_back(result);
				break;
			case OPCODE::CMP_EQ:
				if (f1->value == *f2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_NEQ:
				if (f1->value != *f2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_L:
				if (f1->value < *f2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_G:
				if (f1->value > *f2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_LE:
				if (f1->value <= *f2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_GE:
				if (f1->value >= *f2)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			default:
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with two floats: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		void VirtualMachine::PerformALUcallClassTypes(ClassWrapper* class1, const ClassType* class2, OPCODE op, Frame* frame)
		{
			switch (op)
			{
			case OPCODE::CMP_EQ:
				if (class1->type->namespaceName == class2->namespaceName && class1->type->name == class2->name)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			case OPCODE::CMP_NEQ:
				if (class1->type->namespaceName != class2->namespaceName || class1->type->name != class2->name)
				{
					objectStack.push_back(AllocTrue());
				}
				else
				{
					objectStack.push_back(AllocFalse());
				}
				break;
			default:
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with two class types: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		UnknownObject* VirtualMachine::AllocUnknown(const std::string* value)
		{
			return new UnknownObject(value);
		}

		NullObject* VirtualMachine::AllocNull()
		{
			return &nullObject;
		}

		TrueObject* VirtualMachine::AllocTrue()
		{
			return &trueObject;
		}

		FalseObject* VirtualMachine::AllocFalse()
		{
			return &falseObject;
		}

		ArrayObject* VirtualMachine::AllocArray(size_t size)
		{
			ArrayObject* array = new ArrayObject(Type::BASE, size);
			for (size_t i = 0; i < size; i++)
				array->array[i].object = AllocNull();
			return array;
		}

		StringObject* VirtualMachine::AllocString(const std::string& value)
		{
			return new StringObject(value);
		}

		IntegerObject* VirtualMachine::AllocInteger(const std::string& value)
		{
			return new IntegerObject(value);
		}

		FloatObject* VirtualMachine::AllocFloat(const std::string& value)
		{
			return new FloatObject(std::stod(value));
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
				DisplayError("call stack was empty on VM launch, terminating");
				return;
			}
			const CallPath& path = callStack.top();
			const MethodType* entryPoint = GetMethodOrNull(*path.GetNamespace(), *path.GetClass(), *path.GetMethod());
			if (entryPoint == nullptr)
			{
				errors |= ERROR::INVALID_CALL_ARGUMENT | ERROR::TERMINATE_ON_LAUNCH;
				DisplayError("entry-point method, provided in call stack was not found");
				return;
			}

			objectStack.push_back(AllocNull()); // reference to class
			for (size_t i = 0; i < entryPoint->parameters.size(); i++)
			{
				objectStack.push_back(AllocNull());
			}
			auto startTimePoint = std::chrono::system_clock::now();

			StartNewStackFrame();

			auto endTimePoint = std::chrono::system_clock::now();
			auto elapsedTime = endTimePoint - startTimePoint;

			if (errors == 0)
			{
				if (objectStack.size() > 1)
				{
					errors |= ERROR::OBJECTSTACK_CORRUPTION;
					DisplayError("object stack is not in its initial position after execution");
					return;
				}
				if (!callStack.empty())
				{
					errors |= ERROR::CALLSTACK_CORRUPTION;
					DisplayError("call stack was not empty after VM execution");
					return;
				}
				if (config.execution.checkExitCode)
				{
					if (objectStack.empty())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						DisplayError("no return value from entry point function provided");
						return;
					}
					*config.streams.out << std::endl;
					if (objectStack.back()->type == Type::INTEGER)
					{
						*config.streams.out << "[VM]: execution finished with exit code " << reinterpret_cast<IntegerObject*>(objectStack.back())->value << std::endl;
					}
					else if (objectStack.back()->type == Type::NULLPTR)
					{
						*config.streams.out << "[VM]: execution finished with exit code 0" << std::endl;
					}
					else
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("return value from entry point function was neither integer nor null");
					}
				}
			}
			*config.streams.out << "[VM]: total code execution time: ";
			*config.streams.out << std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count() << "ms" << std::endl;
		}

		uint32_t VirtualMachine::GetErrors() const
		{
			return errors;
		}
	}
}
#undef PRINTPREVFRAME
#undef PRINTFRAME_2
#undef PRINTFRAME