#include "virtualMachine.h"
#include <Windows.h>
#include <cstdlib>
#include <iomanip>

#undef ERROR
#undef CONST
#undef THIS
#undef TRUE
#undef FALSE
#undef IN
#undef OUT
#undef min
#undef max
#undef LoadLibrary
#undef FreeLibrary
#undef GetMessage

namespace MSL
{
	namespace VM
	{
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

		BaseObject* VirtualMachine::ResolveReference(BaseObject* object, const Frame::LocalsTable& locals, const MethodType* _method, const BaseObject* _class, const NamespaceType* _namespace, bool checkError = true)
		{
			if (!AssertType(object, Type::UNKNOWN)) return object;
			// safe as type is UNKNOWN
			const std::string& objectName = *GetObjectName(object);

			// search for local variable in method
			auto localsIt = locals.find(objectName);
			if (localsIt != locals.end()) return localsIt->second.object;

			// search for attribute in class object
			const ClassType* actualClass = nullptr;
			if (!_method->isStatic())
			{
				const ClassObject* thisObject = static_cast<const ClassObject*>(_class);
				actualClass = thisObject->type;
				auto classIt = thisObject->attributes.find(objectName);
				if (classIt != thisObject->attributes.end())
				{
					return classIt->second;
				}
			}
			else
			{
				actualClass = static_cast<const ClassWrapper*>(_class)->type;
			}
			// search for static attribute in class
			auto classIt = actualClass->staticInstance->attributes.find(objectName);
			if (classIt != actualClass->staticInstance->attributes.end())
			{
				return classIt->second;
			}

			// search for class in namespace / friend namespaces
			BaseObject* classWrap = SearchForClass(objectName, _namespace);
			if (classWrap != nullptr) return classWrap;

			// search for namespace in assembly
			const auto ns = GetNamespaceOrNull(objectName);
			if (ns != nullptr) return AllocNamespaceWrapper(ns);

			InvokeError(ERROR::MEMBER_NOT_FOUND, "object with name `" + objectName + "` was not found", objectName);
			std::string className = _class->ToString() + (_method->isStatic() ? "[static]" : "[this]");
			return nullptr;
		}

		BaseObject* VirtualMachine::GetMemberObject(BaseObject* object, const std::string& memberName)
		{
			BaseObject* memberObject = nullptr;
			switch (object->type)
			{
			case Type::NAMESPACE:
			{
				NamespaceWrapper* ns = static_cast<NamespaceWrapper*>(object);
				auto it = ns->type->classes.find(memberName);
				if (it != ns->type->classes.end())
				{
					memberObject = it->second.wrapper;
				}
			}
			break;
			case Type::CLASS_OBJECT:
			{
				ClassObject* obj = static_cast<ClassObject*>(object);
				auto& attributes = obj->attributes;
				auto objectAttr = attributes.find(memberName);
				if (objectAttr != attributes.end())
				{
					memberObject = objectAttr->second;
				}
				else
				{
					auto staticAttr = obj->type->staticInstance->attributes.find(memberName);
					if (staticAttr != obj->type->staticInstance->attributes.end())
					{
						memberObject = staticAttr->second;
					}
				}
			}
			break;
			case Type::CLASS:
			{
				ClassWrapper* cl = static_cast<ClassWrapper*>(object);
				memberObject = GetMemberObject(cl->type->staticInstance, memberName);
			}
			break;
			}
			return memberObject;
		}

		ClassWrapper* VirtualMachine::GetClassPrimitive(BaseObject* object)
		{
			const ClassType* cl = nullptr;
			switch (object->type)
			{
			case Type::INTEGER:
				cl = GetClassOrNull("System", "Integer");
				break;
			case Type::FLOAT:
				cl = GetClassOrNull("System", "Math");
				break;
			case Type::STRING:
				cl = GetClassOrNull("System", "String");
				break;
			case Type::TRUE:
				cl = GetClassOrNull("System", "True");
				break;
			case Type::FALSE:
				cl = GetClassOrNull("System", "False");
				break;
			case Type::NULLPTR:
				cl = GetClassOrNull("System", "Null");
				break;
			default:
				InvokeError(ERROR::INVALID_TYPE, "Cannot get primitive class of object with type: " + ToString(object->type), object->ToString());
				return nullptr;
			}
			return cl->wrapper;
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
							InvokeError(ERROR::AMBIGUOUS_TYPE, "find two or more matching classes while resolving object type: " + objectName, objectName);
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
				return static_cast<LocalObject*>(object)->ref.object;
			case MSL::VM::Type::ATTRIBUTE:
				return static_cast<AttributeObject*>(object)->object;
			default:
				return nullptr; // hits only if error occured
			}
		}

		const std::string* VirtualMachine::GetObjectName(const BaseObject* object) const
		{
			switch (object->type)
			{
			case Type::UNKNOWN:
				return static_cast<const UnknownObject*>(object)->ref;
			case Type::CLASS:
				return &static_cast<const ClassWrapper*>(object)->type->name;
			case Type::NAMESPACE:
				return &static_cast<const NamespaceWrapper*>(object)->type->name;
			case Type::ATTRIBUTE:
				return &static_cast<const AttributeObject*>(object)->type->name;
			case Type::LOCAL:
				return &static_cast<const LocalObject*>(object)->name;
			case Type::CLASS_OBJECT:
				return &static_cast<const ClassObject*>(object)->type->name;
			default:
				return nullptr;
			}
		}

		void VirtualMachine::StartNewStackFrame()
		{
			// recursion limit check
			if (callStack.size() > config.execution.recursionLimit)
			{
				errors |= ERROR::STACKOVERFLOW | ERROR::FATAL_ERROR;
				callStack.pop_back();
				return;
			}
			// getting frame arguments
			CallPath& top = callStack.back();
			top.SetFrame(AllocFrame());
			Frame* frame = top.GetFrame();
			frame->_namespace = GetNamespaceOrNull(*callStack.back().GetNamespace());
			frame->_class = GetClassOrNull(frame->_namespace, *callStack.back().GetClass());
			frame->_method = GetMethodOrNull(frame->_class, *callStack.back().GetMethod());

			#define RET_CS_POP callStack.pop_back(); return

			// in case method was not found
			if (frame->_method == nullptr)
			{
				// probably class constructor was called (class name => method name)
				ClassWrapper* wrapper = nullptr;
				if (frame->_namespace != nullptr)
				{
					std::string methodName = *callStack.back().GetMethod();
					std::string className = GetMethodActualName(methodName);
					wrapper = SearchForClass(className, frame->_namespace);
					if (wrapper != nullptr)
					{
						const ClassType* classType = wrapper->type;
						if (classType->methods.find(methodName) != classType->methods.end()) // constructor was found, delegating call to new frame
						{
							CallPath newFrame;
							newFrame.SetNamespace(&classType->namespaceName);
							newFrame.SetClass(&className);
							newFrame.SetMethod(&methodName);
							callStack.pop_back();
							callStack.push_back(std::move(newFrame));
							StartNewStackFrame();
							return;
						}
						else if (classType->isAbstract()) // if class is abstract, constructor is not found too
						{
							InvokeError(ERROR::ABSTRACT_MEMBER_ACCESS, "cannot create instance of abstract class: " + GetFullClassType(classType), classType->name);

						}
						else if (classType->isStatic())
						{
							InvokeError(ERROR::MEMBER_NOT_FOUND, "cannot create instance of static class: " + GetFullClassType(classType), classType->name);

						}
						else // probably constructor is just missing
						{
							InvokeError(ERROR::METHOD_NOT_FOUND, "could not call class " + GetFullClassType(classType) + " constructor: " + methodName, classType->name);
						}
					}
				}
				if (frame->_class != nullptr)
				{
					if (errors == 0)
					{
						InvokeError(
							ERROR::METHOD_NOT_FOUND,
							"method " + GetFullClassType(frame->_class) + '.' + *callStack.back().GetMethod() + " was not found",
							GetMethodActualName(*callStack.back().GetMethod())
						);
					}
				}
				RET_CS_POP;
			}
			// if member is private, and it is called from another class, call is invalid
			if (!frame->_method->isPublic() && callStack.size() > 1)
			{
				CallPath& prev = callStack[callStack.size() - 2];
				if (*prev.GetNamespace() != frame->_namespace->name || *prev.GetClass() != frame->_class->name)
				{
					InvokeError(
						ERROR::PRIVATE_MEMBER_ACCESS, 
						"trying to call private method: " + GetFullClassType(frame->_class) + '.' + GetFullMethodType(frame->_method), 
						GetMethodActualName(frame->_method->name)
					);
 					RET_CS_POP;
				}
			}
			// if method is abstract, it cannot be called
			if (frame->_method->isAbstract())
			{
				InvokeError(
					ERROR::ABSTRACT_MEMBER_ACCESS, 
					"trying to call abstract method: " + GetFullClassType(frame->_class) + '.' + GetFullMethodType(frame->_method), 
					GetMethodActualName(frame->_method->name)
				);
 				RET_CS_POP;
			}
			if (frame->_method->isStaticConstructor())
			{
				if (frame->_class->staticConstructorCalled)
				{
					InvokeError(
						ERROR::INVALID_METHOD_CALL,
						"static constructor of class cannot be called twice: " + GetFullClassType(frame->_class) + '.' + GetFullMethodType(frame->_method),
						GetMethodActualName(frame->_method->name)
					);
 					RET_CS_POP;
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
				callStack.push_back(std::move(staticConstructorFrame));
				objectStack.push_back(frame->_class->wrapper);
				StartNewStackFrame();
				if (errors != 0)
				{
					RET_CS_POP;
				}
				objectStack.pop_back();
			}

			// if class is system class, call must be delegated to SystemCall function
			if (frame->_class->isSystem())
			{
				callStack.pop_back();
				PerformSystemCall(frame->_class, frame->_method, frame);
				return;
			}

			// read all parameters of method and add them to frame as const locals
			for (auto it = frame->_method->parameters.rbegin(); it != frame->_method->parameters.rend(); it++)
			{
				if (objectStack.empty())
				{
					InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "object stack does not contain enough parameters for method call", GetMethodActualName(frame->_method->name));

					RET_CS_POP;
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
					InvokeError(ERROR::INVALID_BYTECODE | ERROR::FATAL_ERROR, "first parameter of non-static method must always be equal to `this`", GetMethodActualName(frame->_method->name));
 					RET_CS_POP;
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
					InvokeError(ERROR::INVALID_METHOD_CALL, "can not create instance of static class: " + GetFullClassType(frame->_class), frame->_class->name);
 					RET_CS_POP;
				}
				else
				{
					frame->classObject = AllocClassObject(frame->_class);
					frame->locals["this"].object = frame->classObject;
				}
			}

			while (frame->offset < frame->_method->body.size())
			{
				if (errors != 0)
				{
					// add stack trace
					exception.AddTraceEntry(
						GetFullClassType(frame->_class) + '.' + GetMethodActualName(frame->_method->name)
					);
					// return if no catch statements in this frame
					if (frame->exceptionStack.empty() || errors & ERROR::FATAL_ERROR)
					{
						RET_CS_POP;
					}
					// get jump label for handler
					errors = 0;
					auto catchStatement = frame->exceptionStack.back();
					objectStack.resize(catchStatement.stackSize);
					frame->exceptionStack.pop_back();
					frame->offset = frame->_method->labels[catchStatement.label];
				}
				OPCODE op = ReadOPCode(frame->_method->body, frame->offset);
				CollectGarbage();
				switch (op)
				{
				case (OPCODE::PUSH_OBJECT):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if (ValidateHashValue(hash, frame->_method->dependencies.size()))
					{
						const std::string* objectName = &frame->_method->dependencies[hash];
						objectStack.push_back(AllocUnknown(objectName));
					}
					break;
				}
				#define ALU_1(op) case op: PerformALUCall(op, 1, frame); break
				ALU_1(OPCODE::NEGATION_OP);
				ALU_1(OPCODE::NEGATIVE_OP);
				ALU_1(OPCODE::POSITIVE_OP);
				#define ALU_2(op) case op: PerformALUCall(op, 2, frame); break
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

				case (OPCODE::GET_INDEX):
				{
					if (objectStack.size() < 2)
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "not enough parameters in stack for get_index call", !objectStack.empty() ? objectStack.back()->ToString() : "");
 						return;
					}
					BaseObject* object = objectStack.back();
					object = ResolveReference(object, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (object == nullptr) return; // error is handled in ResolveReference method
					object = GetUnderlyingObject(object);

					objectStack.pop_back();
					BaseObject* index = objectStack.back();
					if(AssertType(index, Type::UNKNOWN)) index = ResolveReference(index, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (index == nullptr) return; // error is handled in ResolveReference method
					index = GetUnderlyingObject(index);

					objectStack.back() = object;
					objectStack.push_back(index);
					switch (object->type)
					{
					case Type::CLASS_OBJECT:
						InvokeObjectMethod("GetByIndex_2", static_cast<ClassObject*>(object));
						break;
					case Type::INTEGER:
					case Type::FLOAT:
					case Type::STRING:
					case Type::TRUE:
					case Type::FALSE:
					case Type::NULLPTR:
					{
						ClassWrapper* wrap = GetClassPrimitive(object);
						CallPath newFrame;
						auto method = std::make_unique<std::string>("GetByIndex_1");
						auto methodPtr = method.get();
						frame->localStorage.push_back(std::move(method));
						newFrame.SetNamespace(&wrap->type->namespaceName);
						newFrame.SetClass(&wrap->type->name);
						newFrame.SetMethod(methodPtr);
						callStack.push_back(std::move(newFrame));
						StartNewStackFrame();
					}
					break;
					default:
						InvokeError(
							ERROR::INVALID_TYPE, 
							"object with invalid type was passed to GetByIndex() call: " + ToString(object->type), 
							object->ToString()
						);
						return;
					}
					break;
				}
				case (OPCODE::CALL_FUNCTION):
				{
					uint8_t paramSize = ReadOPCode(frame->_method->body, frame->offset);
					if (objectStack.size() < paramSize + 2u) // function object + caller object
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "not enough parameters in stack for function call", !objectStack.empty() ? objectStack.back()->ToString() : "");
 						return;
					}
					BaseObject* obj = objectStack.back();
					if (!AssertType(obj, Type::UNKNOWN, "expected method name, found object: " + obj->ToString(), frame)) return;

					for (size_t i = 0; i < paramSize; i++)
					{
						size_t index = objectStack.size() - i - 2;
						BaseObject* obj = objectStack[index];
						if (AssertType(obj, Type::UNKNOWN))
						{
							objectStack[index] = ResolveReference(objectStack[index], frame->locals, frame->_method, frame->classObject, frame->_namespace);
							if (objectStack[index] == nullptr) return;
						}
					}
					CallPath newFrame;
					BaseObject* caller = objectStack[objectStack.size() - paramSize - 2];
					if (AssertType(caller, Type::UNKNOWN)) caller = ResolveReference(caller, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (caller == nullptr) return; // check performed in ResolveReference method

					caller = GetUnderlyingObject(caller);
					switch (caller->type)
					{
					case Type::CLASS_OBJECT:
					{
						ClassObject* object = static_cast<ClassObject*>(caller);
						// top of stack must be an unknown object (function name)
						UnknownObject* function = static_cast<UnknownObject*>(objectStack.back());
						std::string objectFunctionName = GetMethodActualName(*GetObjectName(objectStack.back())) + '_' + std::to_string(paramSize + 1);
						if (object->type->methods.find(objectFunctionName) != object->type->methods.end())
						{
							objectStack[objectStack.size() - paramSize - 2] = object;
							frame->localStorage.push_back(std::make_unique<std::string>(objectFunctionName));
							function->ref = frame->localStorage.back().get();
						}
						newFrame.SetNamespace(&object->type->namespaceName);
						newFrame.SetClass(&object->type->name);
						newFrame.SetMethod(GetObjectName(objectStack.back()));
						callStack.push_back(std::move(newFrame));
					}
					break;
					case Type::CLASS:
					{
						ClassWrapper* object = static_cast<ClassWrapper*>(caller);
						objectStack[objectStack.size() - paramSize - 2] = object;
						newFrame.SetNamespace(&object->type->namespaceName);
						newFrame.SetClass(&object->type->name);
						newFrame.SetMethod(GetObjectName(objectStack.back()));
						callStack.push_back(std::move(newFrame));
					}
					break;
					case Type::NAMESPACE:
					{
						UnknownObject* function = static_cast<UnknownObject*>(objectStack.back());
						// top of stack must be unknown object (function name)
						std::string className = GetMethodActualName(*GetObjectName(objectStack.back()));
						const NamespaceType* ns = static_cast<NamespaceWrapper*>(caller)->type;
						auto classIt = ns->classes.find(className);
						if (classIt == ns->classes.end())
						{
							InvokeError(ERROR::MEMBER_NOT_FOUND, "class `" + className + "` was not found in namespace: " + ns->name, className);
 							return;
						}
						else if (classIt->second.isInternal() && ns->name != frame->_namespace->name)
						{
							InvokeError(ERROR::PRIVATE_MEMBER_ACCESS, "trying to access namespace internal member: " + GetFullClassType(&classIt->second), className);
 							return;
						}
						objectStack[objectStack.size() - paramSize - 2] = classIt->second.wrapper;
						newFrame.SetNamespace(GetObjectName(caller));
						newFrame.SetClass(&classIt->second.name);
						newFrame.SetMethod(GetObjectName(objectStack.back()));
						callStack.push_back(std::move(newFrame));
						break;
					}
					case Type::INTEGER:
					case Type::FLOAT:
					case Type::STRING:
					case Type::TRUE:
					case Type::FALSE:
					case Type::NULLPTR:
					{
						objectStack[objectStack.size() - paramSize - 2] = caller;
						const ClassType* cl = GetClassPrimitive(caller)->type;
						newFrame.SetNamespace(&cl->namespaceName);
						newFrame.SetClass(&cl->name);
						newFrame.SetMethod(GetObjectName(objectStack.back()));
						callStack.push_back(std::move(newFrame));
						break;
					}
					default:
						InvokeError(ERROR::INVALID_TYPE, "caller of method was neither class object nor class type", caller->ToString());
						return;
						break;
					}
					objectStack.pop_back(); // remove unknown object (function name)
					
					StartNewStackFrame();
					break;
				}
				case (OPCODE::GET_MEMBER):
				{
					if (objectStack.size() < 2)
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "not enough objects in stack to get member", !objectStack.empty() ? objectStack.back()->ToString() : "");
						break;
					}
					BaseObject* member = objectStack.back();
					objectStack.pop_back();
					BaseObject* calledObject = objectStack.back();
					objectStack.pop_back();
					if (AssertType(calledObject, Type::UNKNOWN)) calledObject = ResolveReference(calledObject, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (calledObject == nullptr) break; // check performed in ResolveReference method
					const std::string* memberName = GetObjectName(member);
					if (!AssertType(member, Type::UNKNOWN) || memberName == nullptr)
					{
						InvokeError(ERROR::INVALID_TYPE, "invalid member was called: " + member->ToString(), member->ToString());
						break;
					}

					BaseObject* memberObject = GetMemberObject(calledObject, *memberName);
					if (memberObject == nullptr)
					{
						InvokeError(ERROR::MEMBER_NOT_FOUND, "member was not found: " + calledObject->ToString() + '.' + member->ToString(), member->ToString());
						break;
					}
					if (AssertType(memberObject, Type::ATTRIBUTE))
					{
						const AttributeType* type = static_cast<AttributeObject*>(memberObject)->type;
						if (!AssertType(calledObject, Type::CLASS_OBJECT) &&
							!AssertType(calledObject, Type::CLASS, "trying to get attribute from object which is neither class, nor class object", frame))
							break;

						if (!type->isPublic())
						{
							const ClassType* classType = nullptr;
							if (AssertType(calledObject, Type::CLASS))
							{
								classType = static_cast<ClassWrapper*>(calledObject)->type;
							}
							else
							{
								classType = static_cast<ClassObject*>(calledObject)->type;
							}
							if (classType != frame->_class)
							{
								InvokeError(ERROR::PRIVATE_MEMBER_ACCESS, "trying to access class private member: " + GetFullClassType(classType) + '.' + type->name, type->name);
								break;
							}
						}
					}
					objectStack.push_back(memberObject);
					break;
				}
				case (OPCODE::POP_TO_RETURN):
					if (objectStack.empty())
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "object stack is empty, but `return` instruction called", "");
					}
					else
					{
						BaseObject* obj = objectStack.back();
						if (AssertType(obj, Type::UNKNOWN)) obj = ResolveReference(obj, frame->locals, frame->_method, frame->classObject, frame->_namespace);
						if (obj == nullptr) break; // ResolveReference handles errors
						objectStack.back() = obj;
					}
					callStack.pop_back();
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
				case (OPCODE::PUSH_CATCH):
					frame->exceptionStack.push_back({
						objectStack.size(),
						ReadLabel(frame->_method->body, frame->offset)
					});
					break;
				case (OPCODE::POP_CATCH):
					if (frame->exceptionStack.empty())
					{
						InvokeError(ERROR::EXCEPTIONSTACK_EMPTY | ERROR::FATAL_ERROR, "exception handler was already popped", GetMethodActualName(frame->_method->name));
 						return;
					}
					frame->exceptionStack.pop_back();
					break;
				case (OPCODE::JUMP_IF_TRUE):
				{
					auto label = ReadLabel(frame->_method->body, frame->offset);
					if (objectStack.empty())
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "object stack is empty, but jump_if_true needs boolean", "jump_if_true");
 						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (AssertType(object, Type::UNKNOWN)) object = ResolveReference(object, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (object == nullptr) break;
					object = GetUnderlyingObject(object);

					if (AssertType(object, Type::CLASS_OBJECT))
					{
						InvokeObjectMethod("ToBoolean_1", static_cast<ClassObject*>(object));
						if (errors != 0) break;
						object = objectStack.back();
						objectStack.pop_back();
					}
					if (AssertType(object, Type::TRUE))
						frame->offset = frame->_method->labels[label];
					else if (object->type != Type::FALSE && object->type != Type::NULLPTR)
					{
						InvokeError(ERROR::INVALID_TYPE, "objects cannot be implicitly converted to Boolean", object->ToString());
					}
					break;
				}
				case (OPCODE::JUMP_IF_FALSE):
				{
					auto label = ReadLabel(frame->_method->body, frame->offset);
					if (objectStack.empty())
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "object stack is empty, but jump_if_false needs boolean", "jump_if_false");
 						return;
					}
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					if (AssertType(object, Type::UNKNOWN))
					{
						object = ResolveReference(object, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					}
					if (object == nullptr) break;
					object = GetUnderlyingObject(object);

					if (AssertType(object, Type::CLASS_OBJECT))
					{
						InvokeObjectMethod("ToBoolean_1", static_cast<ClassObject*>(object));
						if (errors != 0) break;
						object = objectStack.back();
						objectStack.pop_back();
					}
					if (AssertType(object, Type::FALSE) || AssertType(object, Type::NULLPTR))
						frame->offset = frame->_method->labels[label];
					else if (!AssertType(object, Type::TRUE, "object cannot be implicitly converted to boolean", frame)) 
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
					if (ValidateHashValue(hash, frame->_method->dependencies.size()))
					{
						if (!frame->integerCache.Has(hash))
							frame->integerCache.Add(hash, 
								IntegerObject::InnerType(frame->_method->dependencies[hash])
							);
						objectStack.push_back(AllocInteger(frame->integerCache[hash]));
					}
					break;
				}
				case (OPCODE::PUSH_FLOAT):
				{
					size_t hash = ReadHash(frame->_method->body, frame->offset);
					if (ValidateHashValue(hash, frame->_method->dependencies.size()))
					{
						if (!frame->floatCache.Has(hash))
							frame->floatCache.Add(hash,
								FloatObject::InnerType(std::stod(frame->_method->dependencies[hash]))
							);
						objectStack.push_back(AllocFloat(frame->floatCache[hash]));
					}
					break;
				}
				case (OPCODE::PUSH_THIS):
					objectStack.push_back(frame->classObject);
					break;
				case (OPCODE::PUSH_NULL):
					objectStack.push_back(AllocNull());
					break;
				case (OPCODE::SET_ALU_INCR):
					AluIncrMode = true;
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
					callStack.pop_back();
					return;
					break;
				case (OPCODE::JUMP):
					frame->offset = frame->_method->labels[ReadLabel(frame->_method->body, frame->offset)];
					break;
				case (OPCODE::POP_STACK_TOP):
					if (objectStack.empty())
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "pop_stack_top instruction called, but object stack was empty", "pop_stack_top");
 						return;
					}
					objectStack.pop_back();
					break;
				default:
					InvokeError(ERROR::INVALID_BYTECODE | ERROR::FATAL_ERROR, "opcode " + OpcodeToMethod(op) + " was found, but not expected", OpcodeToMethod(op));
					break;
				}
			}
			InvokeError(ERROR::INVALID_BYTECODE | ERROR::FATAL_ERROR, "execution of method went out of frame", std::to_string(frame->offset));
		}

		void VirtualMachine::PerformSystemCall(const ClassType* _class, const MethodType* _method, Frame* frame)
		{
			if (objectStack.empty())
			{
				InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "object stack was empty but expected to have SystemCall arguments", "");
				return;
			}
			else if (_class->name == "Dll")
			{
				if (_method->name.substr(0, 5) == "Call_") // Call with any parameters (see declaration in AddSystemNamespace)
				{
					#ifndef MSL_DLL_API
					InvokeError(ERROR::INVALID_METHOD_CALL, "Dll.Call method is not defined in MSL VM", "Call");
					return;
					#else
					int argCount = std::stoi(_method->name.substr(5, _method->name.size())) + 1; // class as extra parameter
					if (argCount > (int)objectStack.size())
					{
						InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "not enough arguments to call Dll.Call() method", std::to_string(argCount));
						return;
					}
					auto argBegin = objectStack.end() - argCount;
					BaseObject* module = GetUnderlyingObject(*(argBegin + 1));
					BaseObject* function = GetUnderlyingObject(*(argBegin + 2));
					objectStack.erase(argBegin, argBegin + 3);
					if (!AssertType(function, Type::STRING, "function argument must be a string object", frame)) return;
					if (!AssertType(function, Type::STRING, "module argument must be a string object", frame)) return;

					StringObject::InnerType& moduleName = static_cast<StringObject*>(module)->value;
					StringObject::InnerType& functionName = static_cast<StringObject*>(function)->value;

					using MSLFunction = void(*)(VirtualMachine*);
					auto DllFunction = (MSLFunction)dllLoader.GetFunctionPointer(moduleName, functionName);
					
					if (!dllLoader.HasLibrary(moduleName))
					{
						InvokeError(ERROR::DLL_NOT_FOUND, "module was not loaded before method call", moduleName);
						return;
					}
					if (DllFunction == NULL) 
					{
						InvokeError(ERROR::METHOD_NOT_FOUND, "method " + functionName + " was not found in module: " + moduleName, functionName); 
						return; 
					}
					// DLL call
					DllFunction(this);
					#endif
				}
				else if (_method->name == "LoadLibrary_1")
				{
					#ifndef MSL_DLL_API
					InvokeError(ERROR::INVALID_METHOD_CALL, "Dll.LoadLibrary method is not defined in MSL VM", "LoadLibrary");
					return;
					#else
					BaseObject* lib = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back();
					if (!AssertType(lib, Type::STRING, "dll library name must be a string", frame)) return;
					if (LoadDll(static_cast<StringObject*>(lib)->value))
						objectStack.push_back(AllocTrue());
					else
						objectStack.push_back(AllocFalse());
					#endif
				}
				else if (_method->name == "FreeLibrary_1")
				{
					#ifndef MSL_DLL_API
					InvokeError(ERROR::INVALID_METHOD_CALL, "Dll.FreeLibrary method is not defined in MSL VM", "FreeLibrary");
					return;
					#else
					BaseObject* lib = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back();
					if (!AssertType(lib, Type::STRING, "dll library name must be a string", frame)) return;
					dllLoader.FreeLibrary(static_cast<StringObject*>(lib)->value);
					objectStack.push_back(AllocNull());
					#endif
				}
			}
			else if (_class->name == "Integer")
			{
				if (_method->name == "ToString_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);
					objectStack.push_back(AllocString(obj->ToString()));
				}
				else if (_method->name == "Integer_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocInteger(0));
				}
				else if (_method->name == "Integer_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // class reference
					if (!AssertType(object, Type::INTEGER, "Integer object exprected as constructor argument", frame))
						return;
					objectStack.push_back(object);
				}
			}
			else if (_class->name == "Math")
			{
				if (_method->name == "Math_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocFloat(0.0));
				}
				else if (_method->name == "ToString_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);
					objectStack.push_back(AllocString(obj->ToString()));
				}
				else if (_method->name == "Math_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // class reference
					objectStack.push_back(object);
				}
				else if (_method->name == "ToInteger_0")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					FloatObject::InnerType value = static_cast<FloatObject*>(object)->value;
					objectStack.push_back(AllocInteger(std::to_string(value)));
				}
			}
			else if (_class->name == "String")
			{
				if (_method->name == "String_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocString(""));
				}
				else if (_method->name == "String_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // class reference

					objectStack.push_back(object);
				}
				else if (_method->name == "ToString_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);
					objectStack.push_back(AllocString(obj->ToString()));
				}
				else if (_method->name == "ToInteger_0" || _method->name == "ToBoolean_0" || _method->name == "ToFloat_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);

					StringObject::InnerType& str = static_cast<StringObject*>(obj)->value;
					if (_method->name == "ToInteger_0")
						objectStack.push_back(AllocInteger(str));
					else if (_method->name == "ToBoolean_0")
						if (str == "True" || str == "true" || str == "1")
							objectStack.push_back(AllocTrue());
						else
							objectStack.push_back(AllocFalse());
					else if (_method->name == "ToFloat_0")
						objectStack.push_back(AllocFloat(str));
					else
						objectStack.push_back(AllocNull());
				}
				else if (_method->name == "Empty_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);

					StringObject::InnerType& str = static_cast<StringObject*>(obj)->value;
					if (str.empty())
						objectStack.push_back(AllocTrue());
					else
						objectStack.push_back(AllocFalse());
				}
				else if (_method->name == "Size_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);

					StringObject::InnerType& str = static_cast<StringObject*>(obj)->value;
					objectStack.push_back(AllocInteger(str.size()));
				}
				else if (_method->name == "Begin_0")
				{
					objectStack.pop_back(); // string object
					objectStack.push_back(AllocInteger(0));
				}
				else if (_method->name == "End_0")
				{
					BaseObject* str = objectStack.back();
					objectStack.pop_back(); // pop string
					if (!AssertType(str, Type::STRING, "String method must recieve string as an argument", frame)) return;
					StringObject* stringValue = static_cast<StringObject*>(str);
					objectStack.push_back(AllocInteger(stringValue->value.size()));
				}
				else if (_method->name == "Next_1")
				{
					BaseObject* iter = objectStack.back();
					objectStack.pop_back(); // pop iter
					BaseObject* str = objectStack.back();
					objectStack.pop_back(); // pop string

					if (!AssertType(iter, Type::INTEGER, "Invalid iterator was passed to Array.Next(this, iter) method", frame)) return;
					if (!AssertType(str, Type::STRING, "String method must recieve string as an argument", frame)) return;

					IntegerObject::InnerType& iterValue = static_cast<IntegerObject*>(iter)->value;
					objectStack.push_back(AllocInteger(iterValue + 1));
				}
				else if(_method->name == "GetByIndex_1" || _method->name == "GetByIter_1")
				{
					BaseObject* idx = objectStack.back();
					objectStack.pop_back();
					BaseObject* str = objectStack.back();
					objectStack.pop_back();
					str = GetUnderlyingObject(str);
					idx = GetUnderlyingObject(idx);

					if (!AssertType(str, Type::STRING, "String class recieved wrong type", frame)) return;
					if (!AssertType(idx, Type::INTEGER, "index must be an integer", frame)) return;
					
					IntegerObject::InnerType& indexValue = static_cast<IntegerObject*>(idx)->value;
					StringObject::InnerType& stringValue = static_cast<StringObject*>(str)->value;
					size_t index = 0;

					if (indexValue >= 0 && indexValue < (unsigned long long)stringValue.size())
					{
						index = std::stol(indexValue.to_string());
						objectStack.push_back(AllocString(StringObject::InnerType(1, stringValue[index])));
					}
					else
					{
						InvokeError(ERROR::INVALID_ARGUMENT, "cannot access String element with index = " + indexValue.to_string(), indexValue.to_string());
						return;
					}
				}
			}
			else if (_class->name == "True")
			{
				if (_method->name == "True_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocTrue());
				}
				else if (_method->name == "True_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // class reference
					if (!AssertType(object, Type::TRUE, "true exprected as constructor argument", frame))
						return;
					objectStack.push_back(object);
				}
				else if (_method->name == "ToString_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);
					objectStack.push_back(AllocString(obj->ToString()));
				}
			}
			else if (_class->name == "False")
			{
				if (_method->name == "False_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocFalse());
				}
				else if (_method->name == "False_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // class reference
					if (!AssertType(object, Type::FALSE, "false exprected as constructor argument", frame))
						return;
					objectStack.push_back(object);
				}
				else if (_method->name == "ToString_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);
					objectStack.push_back(AllocString(obj->ToString()));
				}
			}
			else if (_class->name == "Null")
			{
				if (_method->name == "Null_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocNull());
				}
				else if (_method->name == "Null_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // class reference
					if (!AssertType(object, Type::NULLPTR, "null exprected as constructor argument", frame))
						return;
					objectStack.push_back(object);
				}
				else if (_method->name == "ToString_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);
					objectStack.push_back(AllocString(obj->ToString()));
				}
			}
			else if (_class->name == "Exception")
			{
				if (_method->name == "Instance_0")
				{
					objectStack.push_back(AllocString(exception.GetErrorType()));
					objectStack.push_back(AllocString(exception.GetMessage()));
					objectStack.push_back(AllocString(exception.GetArgument()));
					PerformSystemCall(_class, &_class->methods.at("Exception_3"), frame);
				}
				else if (_method->name == "Exception_3")
				{
					if (!AssertType(objectStack.back(), Type::STRING, "String object expected as type parameter", frame))
						return;
					auto argument = static_cast<StringObject*>(objectStack.back());
					objectStack.pop_back(); // pop argument
					
					if (!AssertType(objectStack.back(), Type::STRING, "String object expected as message parameter", frame))
						return;
					auto message = static_cast<StringObject*>(objectStack.back());
					objectStack.pop_back(); // pop message

					if (!AssertType(objectStack.back(), Type::STRING, "String object expected as argument parameter", frame))
						return;
					auto type = static_cast<StringObject*>(objectStack.back());
					objectStack.pop_back(); // pop type
					
					auto ExceptionClass = static_cast<ClassWrapper*>(objectStack.back())->type;
					objectStack.pop_back(); // pop System.Exception
					auto ExceptionObject = AllocClassObject(ExceptionClass);

					// type attribute
					InitializeAttribute(ExceptionObject, "type", type);

					// message attribute
					InitializeAttribute(ExceptionObject, "message", message);

					// argument attribute
					InitializeAttribute(ExceptionObject, "argument", argument);

					// stackTrace attribute
					auto arrayClass = GetClassOrNull("System", "Array");
					objectStack.push_back(arrayClass->wrapper);
					objectStack.push_back(AllocInteger(exception.GetTraceSize()));

					CallPath newFrame;
					newFrame.SetMethod(&GetMethodOrNull(arrayClass, "Array_1")->name);
					newFrame.SetClass(&arrayClass->name);
					newFrame.SetNamespace(&arrayClass->namespaceName);
					callStack.push_back(std::move(newFrame));
					StartNewStackFrame();

					auto arrayInstance = static_cast<ClassObject*>(objectStack.back());
					objectStack.pop_back();
					ArrayObject* gcArray = static_cast<ArrayObject*>(arrayInstance->attributes.at("array")->object);

					for (size_t i = 0; i < gcArray->array.size(); i++)
						gcArray->array[i] = { AllocString(exception.GetTraceEntry(i)), true };
					InitializeAttribute(ExceptionObject, "stackTrace", arrayInstance);

					objectStack.push_back(ExceptionObject);
				}
			}
			else
			{
				InvokeError(
					ERROR::INVALID_METHOD_CALL, 
					"Invalid method was passed to PerformSystemCall() method: " + GetFullClassType(_class) + '.' + GetFullMethodType(_method), 
					GetMethodActualName(_method->name)
				);
			}
		}

		void VirtualMachine::PerformALUCall(OPCODE op, size_t parameters, Frame* frame)
		{
			if (objectStack.size() < parameters)
			{
				InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "object stack was empty on VM ALU call", "");
 				return;
			}	

			BaseObject* object = nullptr;
			BaseObject* value = nullptr;
			BaseObject* originalValue = nullptr;
			if (parameters == 2)
			{
				value = objectStack.back();
				objectStack.pop_back();
				originalValue = value;
				if (AssertType(value, Type::UNKNOWN))
				{
					value = ResolveReference(value, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (value == nullptr) return;
				}
				value = GetUnderlyingObject(value);
			}
			object = objectStack.back();
			objectStack.pop_back();
			if (AssertType(object, Type::UNKNOWN))
			{
				auto localIt = frame->locals.find(*static_cast<UnknownObject*>(object)->ref);
				if (localIt != frame->locals.end())
				{
					object = AllocLocal(localIt->first, localIt->second);
				}
				else
				{
					object = ResolveReference(object, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (object == nullptr) return;
				}
			}
			BaseObject** objectReference = nullptr;
			switch (object->type)
			{
			case Type::LOCAL:
			{
				LocalObject* local = static_cast<LocalObject*>(object);
				if (local->ref.isConst && local->ref.object->type != Type::NULLPTR && 
					(op == OPCODE::ASSIGN_OP || AluIncrMode))
				{
					InvokeError(ERROR::CONST_MEMBER_MODIFICATION, "trying to modify const local variable: " + local->ToString() + " = " + (value ? value->ToString() : "null"), local->ToString());
 					return;
				}
				objectReference = &local->ref.object;
				break;
			}
			case Type::ATTRIBUTE:
			{
				AttributeObject* attr = static_cast<AttributeObject*>(object);
				if (attr->type->isConst() && attr->object->type != Type::NULLPTR && 
					(op == OPCODE::ASSIGN_OP || AluIncrMode))
				{
					InvokeError(ERROR::CONST_MEMBER_MODIFICATION, "trying to modify const class attribute: " + attr->type->name, attr->type->name);
 					return;
				}
				objectReference = &attr->object;
				break;
			}
			case Type::INTEGER:
			case Type::STRING:
			case Type::FLOAT:
			case Type::CLASS:
			case Type::TRUE:
			case Type::FALSE:
				objectReference = &object;
				if (op == OPCODE::ASSIGN_OP)
				{
					InvokeError(ERROR::INVALID_TYPE, "primitive types are not assignable: " + object->ToString(), object->ToString());					
					return;
				}
				break;
			default:
				InvokeError(ERROR::INVALID_TYPE, "trying to perform operation with invalid object: " + object->ToString(), object->ToString());
 				return;
			}

			if (op == OPCODE::ASSIGN_OP)
			{
				*objectReference = value;
				objectStack.push_back(object);
				return;
			}

			// call assign after operations in this method
			if (AluIncrMode)
			{
				objectStack.push_back(object);
			}

			switch ((*objectReference)->type)
			{
			case Type::CLASS_OBJECT:
			{
				objectStack.push_back(*objectReference);
				if (parameters == 2) objectStack.push_back(value);
				ClassObject* classObject = static_cast<ClassObject*>(*objectReference);
				PerformALUCallClassObject(classObject, op, frame);
			}
			break;
			case Type::INTEGER:
			{
				IntegerObject* integer = static_cast<IntegerObject*>(*objectReference);
				IntegerObject::InnerType* integerValue = nullptr;
				if (parameters == 2)
				{
					value = GetUnderlyingObject(value);
					if (AssertType(value, Type::CLASS_OBJECT))
					{
						ClassObject* valueClassObject = static_cast<ClassObject*>(value);
						objectStack.push_back(value);

						InvokeObjectMethod("ToInteger_1", valueClassObject);
						if (errors != 0 || 
							AssertType(objectStack.back(), Type::INTEGER, "cannot convert class object to Integer", frame)) return;
						
						value = objectStack.back();
						objectStack.pop_back();
					}
					if (AssertType(value, Type::INTEGER))
					{
						integerValue = &static_cast<IntegerObject*>(value)->value;
					}
					else if (!AssertType(value, Type::FLOAT, "cannot convert object to Integer: " + value->ToString(), frame))
					{
						return;
					}
					else
					{
						FloatObject::InnerType* floatValue = &static_cast<FloatObject*>(value)->value;
						FloatObject* intConverted = static_cast<FloatObject*>(AllocFloat(integer->value.to_double()));
						PerformALUcallFloats(intConverted, floatValue, op, frame);
						break;
					}
				}
				PerformALUCallIntegers(integer, integerValue, op, frame);
			}
			break;
			case Type::STRING:
			{
				StringObject::InnerType tmpString;
				StringObject* str = static_cast<StringObject*>(*objectReference);
				StringObject::InnerType* stringValue = nullptr;
				if (parameters == 2)
				{
					value = GetUnderlyingObject(value);
					if (AssertType(value, Type::CLASS_OBJECT))
					{
						ClassObject* valueClassObject = static_cast<ClassObject*>(value);
						objectStack.push_back(value);
						InvokeObjectMethod("ToString_1", valueClassObject);

						if (errors != 0 ||
							!AssertType(objectStack.back(), Type::STRING, "cannot convert class object to String", frame)) return;

						value = objectStack.back();
						objectStack.pop_back();
					}
					if (AssertType(value, Type::STRING))
					{
						stringValue = &static_cast<StringObject*>(value)->value;
					}
					else if (AssertType(value, Type::INTEGER))
					{
						IntegerObject::InnerType* integer = &static_cast<IntegerObject*>(value)->value;
						PerformALUcallStringInteger(str, integer, op, frame);
						break;
					}
					else if (AssertType(value, Type::FLOAT))
					{
						tmpString = value->ToString();
						stringValue = &tmpString;
					}
					else if (AssertType(value, Type::TRUE))
					{
						std::string val = "true";
						StringObject::InnerType* strTrue = &val;
						PerformALUcallStrings(str, strTrue, op, frame);
						break;
					}
					else if (AssertType(value, Type::FALSE))
					{
						std::string val = "false";
						StringObject::InnerType* strFalse = &val;
						PerformALUcallStrings(str, strFalse, op, frame);
						break;
					}
					else if (AssertType(value, Type::NULLPTR))
					{
						std::string val = "null";
						StringObject::InnerType* strNull = &val;
						PerformALUcallStrings(str, strNull, op, frame);
						break;
					}
					else
					{
						InvokeError(ERROR::METHOD_NOT_FOUND, "cannot convert object to String: " + value->ToString(), value->ToString());
 						return;
					}
				}
				PerformALUcallStrings(str, stringValue, op, frame);
			}
			break;
			case Type::FLOAT:
			{
				FloatObject* floatObject = static_cast<FloatObject*>(*objectReference);
				FloatObject::InnerType* floatValue = nullptr;
				FloatObject::InnerType tmpFloat;
				if (parameters == 2)
				{
					value = GetUnderlyingObject(value);
					if (AssertType(value, Type::CLASS_OBJECT))
					{
						ClassObject* valueClassObject = static_cast<ClassObject*>(value);
						objectStack.push_back(value);
						InvokeObjectMethod("ToFloat_1", valueClassObject);
						if (errors == 0 && AssertType(objectStack.back(), Type::FLOAT))
						{
							value = objectStack.back();
							objectStack.pop_back();
						}
						else
						{
							InvokeError(ERROR::INVALID_METHOD_CALL, "cannot convert class object to Float: " + GetFullClassType(valueClassObject->type) + ".ToFloat() method not found", valueClassObject->ToString());
 							return;
						}
					}
					if (AssertType(value, Type::FLOAT))
					{
						floatValue = &static_cast<FloatObject*>(value)->value;
					}
					else if (AssertType(value, Type::INTEGER))
					{
						IntegerObject* integer = static_cast<IntegerObject*>(value);
						tmpFloat = integer->value.to_double();
						floatValue = &tmpFloat;
					}
					else
					{
						InvokeError(ERROR::METHOD_NOT_FOUND, "cannot convert object to String: " + value->ToString(), value->ToString());
 						return;
					}
				}
				PerformALUcallFloats(floatObject, floatValue, op, frame);
			}
			break;
			case Type::CLASS:
			{
				ClassWrapper* classWrap = static_cast<ClassWrapper*>(*objectReference);
				const ClassType* classType = nullptr;
				if (parameters == 2)
				{
					if (AssertType(value, Type::ATTRIBUTE))
					{
						value = static_cast<AttributeObject*>(value)->object;
					}
					if (AssertType(value, Type::CLASS))
					{
						classType = static_cast<ClassWrapper*>(value)->type;
					}
					else
					{
						InvokeError(ERROR::METHOD_NOT_FOUND, "cannot convert object to String: " + value->ToString(), value->ToString());
 						return;
					}
				}
				PerformALUcallClassTypes(classWrap, classType, op, frame);
			}
			break;
			case Type::FALSE:
			case Type::TRUE:
			{
				if (AluIncrMode)
				{
					InvokeError(ERROR::INVALID_BYTECODE, "Boolean value cannot be incremented: " + (*objectReference)->ToString(), (*objectReference)->ToString());
 					return;
				}
				bool b1 = (*objectReference)->type == Type::TRUE ? true : false;
				bool b2 = parameters == 2 ? (AssertType(value, Type::TRUE) ? true : false) : false; // false by default
				PerformALUcallBooleans(b1, b2, op, frame);
			}
			break;
			default:
				InvokeError(ERROR::INVALID_TYPE, "unexpected object type found in ALU call: " + (*objectReference)->ToString(), (*objectReference)->ToString());
 				return;
			}

			if (AluIncrMode)
			{
				PerformALUCall(OPCODE::ASSIGN_OP, 2, frame);
				AluIncrMode = false; // resets after call
			}
		}

		void VirtualMachine::InitializeStaticMembers()
		{
			for (auto assemblyIt = assembly.namespaces.begin(); assemblyIt != assembly.namespaces.end(); assemblyIt++)
			{
				NamespaceType& ns = assemblyIt->second;
				ns.wrapper = GC.nsWrapAlloc.Alloc(&ns);
				for (auto namespaceIt = ns.classes.begin(); namespaceIt != ns.classes.end(); namespaceIt++)
				{
					ClassType& c = namespaceIt->second;
					c.wrapper = GC.classWrapAlloc.Alloc(&c);
					c.staticInstance = GC.classObjAlloc.Alloc(&c);
					c.staticInstance->attributes.reserve(c.staticAttributes.size());
					for (const auto& attr : c.staticAttributes)
					{
						AttributeObject* staticAttr = GC.attributeAlloc.Alloc(&attr.second);
						staticAttr->object = AllocNull();
						c.staticInstance->attributes[attr.first] = staticAttr;
					}
				}
			}
		}

		void VirtualMachine::AddSystemNamespace()
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////
			#define CONCAT_IMPL(x, y) x##y
			#define CONCAT(x, y) CONCAT_IMPL(x, y)
			#define BEGIN_NAMESPACE(_name) NamespaceType _name; \
			_name.name = #_name; \
			NamespaceType* CURRENT_NAMESPACE = &_name; \
			ClassType* CURRENT_CLASS = nullptr

			#define BEGIN_CLASS(_name) ClassType _name; \
			_name.name = #_name; \
			_name.namespaceName = CURRENT_NAMESPACE->name; \
			_name.modifiers |= ClassType::Modifiers::STATIC | ClassType::Modifiers::SYSTEM; \
			CURRENT_CLASS = &_name

			#define METHOD(_name, _cppobj, params) MethodType _cppobj; \
			_cppobj.name = #_name "_" #params; \
			_cppobj.modifiers |= MethodType::Modifiers::PUBLIC;

			#define INSERT_METHOD(_name, _cppobj, params) CURRENT_CLASS->methods.insert({ #_name "_" #params, std::move(_cppobj) })

			#define CONSTRUCTOR(_name, _cppobj, params) METHOD(_name, _cppobj, params); \
			_cppobj.modifiers |= MethodType::Modifiers::CONSTRUCTOR

			#define CONSTRUCTOR_0(_name) \
			CONSTRUCTOR(_name, CONCAT(_name, __LINE__), 0); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 0)

			#define CONSTRUCTOR_1(_name, param1) \
			CONSTRUCTOR(_name, CONCAT(_name, __LINE__), 1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 1)

			#define CONSTRUCTOR_2(_name, param1, param2) \
			CONSTRUCTOR(_name, CONCAT(_name, __LINE__), 2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 2)

			#define CONSTRUCTOR_3(_name, param1, param2, param3) \
			CONSTRUCTOR(_name, CONCAT(_name, __LINE__), 3); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param3); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 3)

			#define STATIC_METHOD(_name, _cppobj, params) METHOD(_name, _cppobj, params); \
			_cppobj.modifiers |= MethodType::Modifiers::STATIC

			#define END_CLASS(_class) CURRENT_NAMESPACE->classes.insert({ #_class, std::move(_class) }); CURRENT_CLASS = nullptr

			#define END_NAMESPACE(_name) auto assemblyIt = assembly.namespaces.find(#_name); \
			if(assemblyIt != assembly.namespaces.end()) {\
			for(auto it = _name.classes.begin(); it != _name.classes.end(); it++) \
			{	\
				auto name = it->second.name; \
			    if(assemblyIt->second.classes.find(name) != assemblyIt->second.classes.end()) {\
					assemblyIt->second.classes.erase(name);\
				   if(config.streams.error != nullptr) \
					*config.streams.error << "[VM WARNING]: user-defined System class was replaced by VM: `" << name << '`'; }\
				assemblyIt->second.classes.insert({std::move(name), std::move(it->second)}); } \
			} \
			else assembly.namespaces.insert({ #_name, std::move(_name) }); CURRENT_NAMESPACE = nullptr

			#define STATIC_METHOD_0(_name) \
			STATIC_METHOD(_name, CONCAT(_name, __LINE__), 0); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 0)

			#define STATIC_METHOD_1(_name, param1) \
			STATIC_METHOD(_name, CONCAT(_name, __LINE__), 1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 1)

			#define STATIC_METHOD_2(_name, param1, param2) \
			STATIC_METHOD(_name, CONCAT(_name, __LINE__), 2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 2)

			#define STATIC_METHOD_3(_name, param1, param2, param3) \
			STATIC_METHOD(_name, CONCAT(_name, __LINE__), 3); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param3); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 3)

			#define STATIC_METHOD_4(_name, param1, param2, param3, param4) \
			STATIC_METHOD(_name, CONCAT(_name, __LINE__), 4); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param3); \
			CONCAT(_name, __LINE__).parameters.push_back(#param4); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 4)

			#define STATIC_METHOD_5(_name, param1, param2, param3, param4, param5) \
			STATIC_METHOD(_name, CONCAT(_name, __LINE__), 5); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param3); \
			CONCAT(_name, __LINE__).parameters.push_back(#param4); \
			CONCAT(_name, __LINE__).parameters.push_back(#param5); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 5)

			#define STATIC_METHOD_6(_name, param1, param2, param3, param4, param5, param6) \
			STATIC_METHOD(_name, CONCAT(_name, __LINE__), 6); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param3); \
			CONCAT(_name, __LINE__).parameters.push_back(#param4); \
			CONCAT(_name, __LINE__).parameters.push_back(#param5); \
			CONCAT(_name, __LINE__).parameters.push_back(#param6); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 6)

			#define METHOD_0(_name) METHOD(_name, CONCAT(_name, __LINE__), 0); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 0)

			#define METHOD_1(_name, param1) METHOD(_name, CONCAT(_name, __LINE__), 1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 1)

			#define METHOD_2(_name, param1, param2) METHOD(_name, CONCAT(_name, __LINE__), 2); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param2); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 2)

			#define ATTRIBUTE_BEGIN_IMPL(attr) \
			AttributeType CONCAT(attr, __LINE__); \
			CONCAT(attr, __LINE__).name = #attr

			#define ATTRIBUTE_END_IMPL(attr) CURRENT_CLASS->objectAttributes.\
			insert({ #attr, std::move(CONCAT(attr, __LINE__)) })

			#define ATTRIBUTE(attr) ATTRIBUTE_BEGIN_IMPL(attr); ATTRIBUTE_END_IMPL(attr)

			#define PUBLIC_CONST_ATTRIBUTE(attr)\
			ATTRIBUTE_BEGIN_IMPL(attr);\
			CONCAT(attr, __LINE__).modifiers |= AttributeType::Modifiers::PUBLIC;\
			CONCAT(attr, __LINE__).modifiers |= AttributeType::Modifiers::CONST;\
			ATTRIBUTE_END_IMPL(attr);

			#define PRIMITIVE_CLASS(_name) BEGIN_CLASS(_name); \
			STATIC_METHOD_0(_name); \
			STATIC_METHOD_0(ToString)
			/////////////////////////////////////////////////////////////////////////////////////////////////////

			BEGIN_NAMESPACE(System);

				BEGIN_CLASS(Dll);
					STATIC_METHOD_2(Call, module, function);
					STATIC_METHOD_3(Call, module, function, arg1);
					STATIC_METHOD_4(Call, module, function, arg1, arg2);
					STATIC_METHOD_5(Call, module, function, arg1, arg2, arg3);
					STATIC_METHOD_6(Call, module, function, arg1, arg2, arg3, arg4);
					STATIC_METHOD_1(LoadLibrary, path);
					STATIC_METHOD_1(FreeLibrary, path);
				END_CLASS(Dll);

				PRIMITIVE_CLASS(Integer);
					STATIC_METHOD_1(Integer, value);
				END_CLASS(Integer);

				PRIMITIVE_CLASS(Float);
					STATIC_METHOD_1(Float, value);
					STATIC_METHOD_0(ToInt);
				END_CLASS(Float);

				PRIMITIVE_CLASS(True);
					STATIC_METHOD_1(True, value);
				END_CLASS(True);

				PRIMITIVE_CLASS(False);
					STATIC_METHOD_1(False, value);
				END_CLASS(False);

				PRIMITIVE_CLASS(Null);
					STATIC_METHOD_1(Null, value);
				END_CLASS(Null);

				BEGIN_CLASS(Exception);
					Exception.modifiers &= ~ClassType::Modifiers::STATIC;
					STATIC_METHOD_0(Instance);
					PUBLIC_CONST_ATTRIBUTE(message);
					PUBLIC_CONST_ATTRIBUTE(stackTrace);
					PUBLIC_CONST_ATTRIBUTE(type);
					PUBLIC_CONST_ATTRIBUTE(argument);
					CONSTRUCTOR_3(Exception, type, message, argument);
				END_CLASS(Exception);

				BEGIN_CLASS(String);
					STATIC_METHOD_0(String); // creates an empty string
					STATIC_METHOD_1(String, value); // creates copy of string from another
					STATIC_METHOD_0(ToInteger); // converts string object to integer
					STATIC_METHOD_0(ToBool); // converts string object to bool
					STATIC_METHOD_0(ToFloat); // converts string object to float
					STATIC_METHOD_0(ToString); // converts object to string
					STATIC_METHOD_0(Empty); // checks if the string is empty (size = 0)
					STATIC_METHOD_0(Size); // returns string size
					STATIC_METHOD_0(Begin); // return iterator to the begin of the string
					STATIC_METHOD_0(End); // returns iterator to the end of the string
					STATIC_METHOD_1(GetByIndex, index); // returns symbol of the string by index as one-length string
					STATIC_METHOD_1(GetByIter, iter); // returns symbol of the string by iterator as one-length string
					STATIC_METHOD_1(Next, iter); // increments iterator of the string
				END_CLASS(String);

			END_NAMESPACE(System);
		}

		bool VirtualMachine::LoadDll(const std::string& libName)
		{
			#ifdef MSL_DLL_API
			dllLoader.AddLibrary(libName.c_str());
			return dllLoader.GetLastError() == NO_ERROR;
			#else
			return false;
			#endif
		}

		void VirtualMachine::CollectGarbage(bool forceCollection)
		{
			uint64_t totalMemory = GC.GetTotalMemoryAlloc();
			if (totalMemory > config.GC.maxMemory)
			{
				if ((errors & ERROR::OUT_OF_MEMORY) == 0)
				{
					InvokeError(ERROR::OUT_OF_MEMORY | ERROR::FATAL_ERROR, "VM hit alloc limit, allocated memory: " + utils::formatBytes(totalMemory), std::to_string(totalMemory));
				}
				errors |= ERROR::OUT_OF_MEMORY | ERROR::FATAL_ERROR;
				return;
			}

			if (!config.GC.allowCollect && !forceCollection) return;

			uint64_t iterAlloc = GC.GetMemoryAllocSinceIter();

			if (forceCollection ||
			   iterAlloc > config.GC.minMemory && 
			   iterAlloc > GC.GetClearedMemorySinceIter())
			{
				GC.Collect(this->assembly, this->callStack, this->objectStack);
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
				InvokeError(ERROR::INVALID_BYTECODE | ERROR::FATAL_ERROR, "hash value of dependency object was invalid: " + std::to_string(hashValue), std::to_string(hashValue));
				return false;
			}
		}

		bool VirtualMachine::AssertType(const BaseObject* object, Type type, const std::string& message, const Frame* frame)
		{
			if (AssertType(object, type)) return true;
			InvokeError(ERROR::INVALID_TYPE, message, object->ToString());
			return false;
		}

		bool MSL::VM::VirtualMachine::AssertType(const BaseObject* object, Type type)
		{
			return object->type == type;
		}

		void VirtualMachine::InvokeObjectMethod(const std::string& methodName, const ClassObject* object)
		{
			const auto method = GetMethodOrNull(object->type, methodName);
			if (method == nullptr)
			{
				InvokeError(ERROR::MEMBER_NOT_FOUND, "method name provided to InvokeObjectMethod() function not found", methodName);
				return;
			}
			if (method->isAbstract() || method->isStatic())
			{
				InvokeError(ERROR::INVALID_METHOD_CALL, "trying to access abstract or static method in InvokeObjectMethod() function", methodName);
				return;
			}
			if (!method->isPublic())
			{
				InvokeError(ERROR::PRIVATE_MEMBER_ACCESS, "trying to access private method in InvokeObjectMethod() function", methodName);
				return;
			}
			CallPath newFrame;
			newFrame.SetMethod(&methodName);
			newFrame.SetClass(&object->type->name);
			newFrame.SetNamespace(&object->type->namespaceName);
			callStack.push_back(std::move(newFrame));
			StartNewStackFrame();
		}

		void VirtualMachine::InitializeAttribute(ClassObject* object, const std::string& attribute, BaseObject* value)
		{
			const AttributeType* attrType = &object->type->objectAttributes.at(attribute);
			AttributeObject* attrObject = GC.attributeAlloc.Alloc(attrType);
			attrObject->object = value;
			object->attributes[attribute] = attrObject;
		}

		void VirtualMachine::InvokeError(size_t error, const std::string& message, const std::string& arg)
		{
			errors |= error;
			exception.Init(message, arg, ErrorToString(error));
		}

		std::string VirtualMachine::OpcodeToMethod(OPCODE op) const
		{
			switch (op)
			{
			case MSL::VM::NEGATION_OP:
				return "NegationOperator";
			case MSL::VM::NEGATIVE_OP:
				return "NegOperator";
			case MSL::VM::POSITIVE_OP:
				return "PosOperator";
			case MSL::VM::SUM_OP:
				return "SumOperator";
			case MSL::VM::SUB_OP:
				return "SubOperator";
			case MSL::VM::MULT_OP:
				return "MultOperator";
			case MSL::VM::DIV_OP:
				return "DivOperator";
			case MSL::VM::MOD_OP:
				return "ModOperator";
			case MSL::VM::POWER_OP:
				return "PowerOperator";
			case MSL::VM::CMP_EQ:
				return "IsEqual";
			case MSL::VM::CMP_NEQ:
				return "IsNotEqual";
			case MSL::VM::CMP_L:
				return "IsLess";
			case MSL::VM::CMP_G:
				return "IsGreater";
			case MSL::VM::CMP_LE:
				return "IsLessEqual";
			case MSL::VM::CMP_GE:
				return "IsGreaterEqual";
			case MSL::VM::CMP_AND:
				return "AndOperator";
			case MSL::VM::CMP_OR:
				return "OrOperator";
			default:
				return "";
			}
		}

		std::string VirtualMachine::ErrorToString(size_t error) const
		{
			error &= (~(ERROR::FATAL_ERROR | ERROR::TERMINATE_ON_LAUNCH));
			switch (error)
			{
			case ERROR::CALLSTACK_EMPTY:
				return "CallStackEmpty";
			case ERROR::INVALID_ARGUMENT:
				return "InvalidArgument";
			case ERROR::INVALID_BYTECODE:
				return "InvalidBytecode";
			case ERROR::INVALID_OPERATION:
				return "InvalidOperation";
			case ERROR::OBJECTSTACK_EMPTY:
				return "ObjectStackEmpty";
			case ERROR::MEMBER_NOT_FOUND:
				return "MemberNotFound";
			case ERROR::STACKOVERFLOW:
				return "StackOverflow";
			case ERROR::PRIVATE_MEMBER_ACCESS:
				return "PrivateMemberAccess";
			case ERROR::CONST_MEMBER_MODIFICATION:
				return "ConstMemberModification";
			case ERROR::ABSTRACT_MEMBER_ACCESS:
				return "AbstractMemberAccess";
			case ERROR::INVALID_METHOD_CALL:
				return "InvalidMethodCall";
			case ERROR::OUT_OF_MEMORY:
				return "OutOfMemory";
			case ERROR::DLL_NOT_FOUND:
				return "DllNotFound";
			case ERROR::EXCEPTIONSTACK_EMPTY:
				return "ExceptionStackEmpty";
			case ERROR::INVALID_TYPE:
				return "InvalidType";
			case ERROR::AMBIGUOUS_TYPE:
				return "AmbiguousType";
			case ERROR::METHOD_NOT_FOUND:
				return "MethodNotFound";
			default:
				return "NoError";
			}
		}

		void VirtualMachine::PrintObjectStack() const
		{
			if (config.streams.error == nullptr) return;
			auto& out = *config.streams.error;
			std::string line = "-----------------------------------------------------------\n";
			out << "---------------------------STACK---------------------------\n";
			size_t count = 0;
			for (auto it = objectStack.rbegin(); it != objectStack.rend(); it++, count++)
			{
				out << std::left << std::setw(line.size() / 2);
				out << "[" + std::to_string(count) + "] " + (*it != nullptr ? (*it)->ToString() : "<error type>");
				out << std::right << std::setw(line.size() / 2 - 1) << (*it != nullptr ? (*it)->GetExtraInfo() : "") << std::endl;
			}
			out << line;
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
			if (i < 0) return "[err_sym " + methodName + ']';
			return std::string(methodName.begin(), methodName.begin() + i);
		}

		void VirtualMachine::PerformALUCallIntegers(IntegerObject* int1, const IntegerObject::InnerType* int2, OPCODE op, Frame* frame)
		{
			IntegerObject* result = AllocInteger(0);
			switch (op)
			{
			case OPCODE::NEGATION_OP:
				if (result->value == 0)
					objectStack.push_back(AllocTrue());
				else
					objectStack.push_back(AllocFalse());
				break;
			case OPCODE::NEGATIVE_OP:
				result->value = -(int1->value);
				objectStack.push_back(result);
				break;
			case OPCODE::POSITIVE_OP:
				// optimize +int1 == int1
				objectStack.push_back(int1);
				break;
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
				InvokeError(ERROR::INVALID_OPERATION, "invalid operation with two integers: " + OpcodeToMethod(op), OpcodeToMethod(op));
				break;
			}
		}

		void VirtualMachine::PerformALUcallStrings(StringObject* str1, const StringObject::InnerType* str2, OPCODE op, Frame* frame)
		{
			switch (op)
			{
			case OPCODE::SUM_OP:
			{
				StringObject* result = static_cast<StringObject*>(AllocString(""));
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
				InvokeError(ERROR::INVALID_OPERATION, "invalid operation with two strings: " + OpcodeToMethod(op), OpcodeToMethod(op));
				break;
			}
		}

		void VirtualMachine::PerformALUcallStringInteger(StringObject* str, const IntegerObject::InnerType* integer, OPCODE op, Frame* frame)
		{
			StringObject* result = static_cast<StringObject*>(AllocString(""));
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
				InvokeError(ERROR::INVALID_OPERATION, "invalid operation with String and Integer: " + OpcodeToMethod(op), OpcodeToMethod(op));
				break;
			}
		}

		void VirtualMachine::PerformALUCallClassObject(ClassObject* obj, OPCODE op, Frame* frame)
		{
			std::string methodName = OpcodeToMethod(op);
			if(methodName.empty())
			{
				InvokeError(ERROR::INVALID_OPERATION, "invalid operation with two class objects: " + OpcodeToMethod(op), OpcodeToMethod(op));
				
			}
			switch (op)
			{
			// one argument
			case OPCODE::NEGATION_OP:
			case OPCODE::NEGATIVE_OP:
			case OPCODE::POSITIVE_OP:
				InvokeObjectMethod(methodName + "_1", obj);
			default:
				InvokeObjectMethod(methodName + "_2", obj);
			}
		}

		void VirtualMachine::PerformALUcallBooleans(bool b1, bool b2, OPCODE op, Frame* frame)
		{
			switch (op)
			{
			case MSL::VM::NEGATION_OP:
				if (!b1)
					objectStack.push_back(AllocTrue());
				else
					objectStack.push_back(AllocFalse());
				break;
			case MSL::VM::CMP_EQ:
				if (b1 == b2)
					objectStack.push_back(AllocTrue());
				else
					objectStack.push_back(AllocFalse());
				break;
			case MSL::VM::CMP_NEQ:
				if (b1 != b2)
					objectStack.push_back(AllocTrue());
				else
					objectStack.push_back(AllocFalse());
				break;
			case MSL::VM::CMP_AND:
				if (b1 && b2)
					objectStack.push_back(AllocTrue());
				else
					objectStack.push_back(AllocFalse());
				break;
			case MSL::VM::CMP_OR:
				if (b1 || b2)
					objectStack.push_back(AllocTrue());
				else
					objectStack.push_back(AllocFalse());
				break;
			default:
				InvokeError(ERROR::INVALID_OPERATION, "invalid operation with two Booleans: " + OpcodeToMethod(op), OpcodeToMethod(op));
				
				break;
			}
		}

		void VirtualMachine::PerformALUcallFloats(FloatObject* f1, const FloatObject::InnerType* f2, OPCODE op, Frame* frame)
		{
			FloatObject* result = static_cast<FloatObject*>(AllocFloat(0.0));
			switch (op)
			{
			case OPCODE::NEGATIVE_OP:
				result->value = -1.0f * f1->value;
				objectStack.push_back(result);
				break;
			case OPCODE::POSITIVE_OP:
				result->value = f1->value;
				objectStack.push_back(result);
				break;
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
				InvokeError(ERROR::INVALID_OPERATION, "invalid operation with two Floats: " + OpcodeToMethod(op), OpcodeToMethod(op));
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
				objectStack.push_back(class1);
				InvokeError(ERROR::INVALID_OPERATION, "invalid operation with two class types: " + OpcodeToMethod(op), OpcodeToMethod(op));
 				break;
			}
		}

		ClassWrapper* VirtualMachine::AllocClassWrapper(const ClassType* _class)
		{
			return _class->wrapper;
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
				callStack.push_back(std::move(newFrame));
				objectStack.push_back(_class->wrapper);
				StartNewStackFrame();
				objectStack.pop_back();
			}
			ClassObject* object = GC.classObjAlloc.Alloc(_class);
			object->attributes.reserve(_class->objectAttributes.size());
			for (const auto& attr : _class->objectAttributes)
			{
				AttributeObject* objectAttr = GC.attributeAlloc.Alloc(&attr.second);
				objectAttr->object = AllocNull();
				object->attributes[attr.second.name] = objectAttr;
			}

			return object;
		}

		NamespaceWrapper* VirtualMachine::AllocNamespaceWrapper(const NamespaceType* _namespace)
		{
			return _namespace->wrapper;
		}

		LocalObject* VirtualMachine::AllocLocal(const std::string& localName, Local& local)
		{
			return GC.localObjAlloc.Alloc(local, localName);
		}

		Frame* VirtualMachine::AllocFrame()
		{
			return GC.frameAlloc.Alloc();
		}

		UnknownObject* VirtualMachine::AllocUnknown(const std::string* value)
		{
			return GC.unknownObjAlloc.Alloc(value);
		}

		NullObject* VirtualMachine::AllocNull()
		{
			return &GC.nullObject;
		}

		TrueObject* VirtualMachine::AllocTrue()
		{
			return &GC.trueObject;
		}

		FalseObject* VirtualMachine::AllocFalse()
		{
			return &GC.falseObject;
		}

		ArrayObject* VirtualMachine::AllocArray(size_t size)
		{
			if ((uint64_t)size * sizeof(NullObject) > config.GC.maxMemory)
			{
				InvokeError(
					ERROR::INVALID_ARGUMENT, 
					"cannot allocate array with too big size = " + std::to_string(size) + " (" + MSL::utils::formatBytes((uint64_t)size * sizeof(NullObject)) + ')',
					std::to_string(size)
				);
				return GC.arrayAlloc.Alloc(0);
			}

			ArrayObject* array = GC.arrayAlloc.Alloc(size);
			for (size_t i = 0; i < size; i++)
			{
				array->array[i].object = AllocNull();
				array->array[i].isElement = true;
			}
			return array;
		}

		StringObject* VirtualMachine::AllocString(const std::string& value)
		{
			return GC.stringAlloc.Alloc(value);
		}

		IntegerObject* VirtualMachine::AllocInteger(const std::string& value)
		{
			return GC.integerAlloc.Alloc(value);
		}

		IntegerObject* VirtualMachine::AllocInteger(int64_t value)
		{
			return GC.integerAlloc.Alloc(value);
		}

		IntegerObject* VirtualMachine::AllocInteger(const IntegerObject::InnerType& value)
		{
			return GC.integerAlloc.Alloc(value);
		}

		FloatObject* VirtualMachine::AllocFloat(const std::string& value)
		{
			return GC.floatAlloc.Alloc(std::stod(value));
		}

		FloatObject* VirtualMachine::AllocFloat(FloatObject::InnerType value)
		{
			return GC.floatAlloc.Alloc(value);
		}

		VirtualMachine::VirtualMachine(Configuration config)
			: config(std::move(config)), errors(0), AluIncrMode(false) { }

		bool VirtualMachine::AddBytecodeFile(std::istream* binaryFile)
		{
			if (!assembly.namespaces.empty() && !config.compilation.allowAssemblyMerge) return false;

			AssemblyEditor editor(binaryFile, config.streams.error);
			CallPath* callPath = nullptr;
			if (callStack.empty())
			{
				callStack.emplace_back();
				callPath = &callStack.back();
			}
			bool result = editor.MergeAssemblies(assembly, config.compilation.varifyBytecode, callPath);
			if(callPath != nullptr && callPath->GetMethod() == nullptr)
				callStack.pop_back();
			return result;
		}

		void VirtualMachine::Run()
		{
			GC.SetLogStream(config.GC.log);
			AddSystemNamespace();
			InitializeStaticMembers();

			if (config.execution.useUnicode)
			{
				#ifdef _WINDOWS_
				SetConsoleOutputCP(CP_UTF8);
				#else
				if(config.streams.error != nullptr)
					*config.streams.error << "[VM WARNING]: useUnicode parameter was enabled, but VM supports it only on Windows system";
				#endif
			}
			if (errors != 0) return;

			if (callStack.empty())
			{
				InvokeError(ERROR::CALLSTACK_EMPTY | ERROR::TERMINATE_ON_LAUNCH, "call stack was empty on VM launch, terminating", "");
				return;
			}

			const CallPath& path = callStack.back();
			if (path.GetNamespace() == nullptr || path.GetClass() == nullptr || path.GetMethod() == nullptr)
			{
				InvokeError(ERROR::INVALID_BYTECODE | ERROR::TERMINATE_ON_LAUNCH, "entry-point was not provided to the VM", "");
				return;
			}
			const MethodType* entryPoint = GetMethodOrNull(*path.GetNamespace(), *path.GetClass(), *path.GetMethod());
			if (entryPoint == nullptr)
			{
				InvokeError(ERROR::METHOD_NOT_FOUND | ERROR::TERMINATE_ON_LAUNCH, "entry-point method, provided in call stack was not found", "");
				return;
			}

			objectStack.push_back(AllocNull()); // reference to class
			for (size_t i = 0; i < entryPoint->parameters.size(); i++)
			{
				objectStack.push_back(AllocNull());
			}
			auto startTimePoint = std::chrono::system_clock::now();
			try
			{
				StartNewStackFrame();
			}
			catch (std::bad_alloc&)
			{
				InvokeError(ERROR::OUT_OF_MEMORY | ERROR::FATAL_ERROR, "VM hit memory limit and fatal error occured. Execution cancelled", "bad_alloc");
				if (config.streams.error != nullptr)
					*config.streams.error << "GC log information:";
				GC.SetLogStream(config.streams.error);
				GC.PrintLog();
				GC.ReleaseMemory();
			}
			catch (std::exception& e)
			{
				InvokeError(ERROR::FATAL_ERROR, "an exception occured during VM execution: ", e.what());
				if (config.streams.error != nullptr) *config.streams.error << e.what() << std::endl;
				GC.ReleaseMemory();
			}

			auto endTimePoint = std::chrono::system_clock::now();
			auto elapsedTime = endTimePoint - startTimePoint;
			
			if (errors != 0)
			{
				if (config.streams.error != nullptr)
				{
					std::ostream& err = *config.streams.error;
					if (errors & ERROR::FATAL_ERROR)
						err << "[VM]: fatal error during VM execution:";
					else
						err << "[VM]: unhandled exception during VM execution:";
					err << "\n    type: '" << exception.GetErrorType() + "'";
					err << "\n    message: '" << exception.GetMessage() << "'";
					err << "\n    argument: '" << exception.GetArgument() << "'";
					err << "\n    exception stack trace:";
					for (size_t i = 0; i < exception.GetTraceSize(); i++)
					{
						err << "\n        " << exception.GetTraceEntry(i);
					}
					err << std::endl;
				}
				else
				{
					CollectGarbage(true);

					if (objectStack.size() > 1)
					{
						InvokeError(ERROR::INVALID_BYTECODE | ERROR::FATAL_ERROR, "object stack is not in its initial position after execution", std::to_string(objectStack.size()));
						return;
					}
					if (!callStack.empty())
					{
						InvokeError(ERROR::INVALID_BYTECODE | ERROR::FATAL_ERROR, "call stack was not empty after VM execution", std::to_string(callStack.size()));
						return;
					}
					if (config.execution.checkExitCode)
					{
						if (objectStack.empty())
						{
							InvokeError(ERROR::OBJECTSTACK_EMPTY | ERROR::FATAL_ERROR, "no return value from entry point function provided", "");
							return;
						}
						if (config.streams.out != nullptr)
						{
							*config.streams.out << std::endl;
							if (AssertType(objectStack.back(), Type::INTEGER))
							{
								*config.streams.out << "[VM]: execution finished with exit code " << static_cast<IntegerObject*>(objectStack.back())->value << std::endl;
							}
							else if (AssertType(objectStack.back(), Type::NULLPTR))
							{
								*config.streams.out << "[VM]: execution finished with exit code 0" << std::endl;
							}
							else
							{
								InvokeError(ERROR::INVALID_TYPE, "return value from entry point function was neither integer nor null", objectStack.back()->ToString());
							}
						}
					}
				}
			}
			GC.ReleaseMemory();

			if (config.streams.out != nullptr)
			{
				*config.streams.out << "[VM]: total code execution time: ";
				*config.streams.out << std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count() << "ms" << std::endl;
			}
		}

		uint32_t& VirtualMachine::GetErrors()
		{
			return errors;
		}

		Configuration& VirtualMachine::GetConfig()
		{
			return config;
		}

		ExceptionTrace& VirtualMachine::GetException()
		{
			return exception;
		}

		std::vector<std::string> VirtualMachine::GetErrorStrings(uint32_t errors) const
		{
			std::vector<std::string> errorList;
			if (errors & ERROR::CALLSTACK_EMPTY) 
				errorList.push_back(STRING(ERROR::CALLSTACK_EMPTY));
			if (errors & ERROR::OBJECTSTACK_EMPTY)
				errorList.push_back(STRING(ERROR::OBJECTSTACK_EMPTY));
			if (errors & ERROR::EXCEPTIONSTACK_EMPTY) 
				errorList.push_back(STRING(ERROR::EXCEPTIONSTACK_EMPTY));
			if (errors & ERROR::STACKOVERFLOW) 
				errorList.push_back(STRING(ERROR::STACKOVERFLOW));
			if (errors & ERROR::OUT_OF_MEMORY) 
				errorList.push_back(STRING(ERROR::OUT_OF_MEMORY));
			if (errors & ERROR::INVALID_BYTECODE)
				errorList.push_back(STRING(ERROR::INVALID_BYTECODE));
			if (errors & ERROR::INVALID_OPERATION)
				errorList.push_back(STRING(ERROR::INVALID_OPERATION));
			if (errors & ERROR::INVALID_ARGUMENT)
				errorList.push_back(STRING(ERROR::INVALID_ARGUMENT));
			if (errors & ERROR::INVALID_TYPE)
				errorList.push_back(STRING(ERROR::INVALID_TYPE));
			if (errors & ERROR::INVALID_METHOD_CALL) 
				errorList.push_back(STRING(ERROR::INVALID_METHOD_CALL));
			if (errors & ERROR::DLL_NOT_FOUND) 
				errorList.push_back(STRING(ERROR::DLL_NOT_FOUND));
			if (errors & ERROR::MEMBER_NOT_FOUND) 
				errorList.push_back(STRING(ERROR::MEMBER_NOT_FOUND));
			if (errors & ERROR::METHOD_NOT_FOUND) 
				errorList.push_back(STRING(ERROR::METHOD_NOT_FOUND));
			if (errors & ERROR::PRIVATE_MEMBER_ACCESS) 
				errorList.push_back(STRING(ERROR::PRIVATE_MEMBER_ACCESS));
			if (errors & ERROR::ABSTRACT_MEMBER_ACCESS) 
				errorList.push_back(STRING(ERROR::ABSTRACT_MEMBER_ACCESS));
			if (errors & ERROR::CONST_MEMBER_MODIFICATION) 
				errorList.push_back(STRING(ERROR::CONST_MEMBER_MODIFICATION));
			if (errors & ERROR::AMBIGUOUS_TYPE) 
				errorList.push_back(STRING(ERROR::AMBIGUOUS_TYPE));
			if (errors & ERROR::TERMINATE_ON_LAUNCH)
				errorList.push_back(STRING(ERROR::TERMINATE_ON_LAUNCH));
			if (errors & ERROR::FATAL_ERROR) 
				errorList.push_back(STRING(ERROR::FATAL_ERROR));

			return errorList;
		}

		AssemblyType& VirtualMachine::GetAssembly()
		{
			return assembly;
		}

		VirtualMachine::CallStack& VirtualMachine::GetCallStack()
		{
			return callStack;
		}

		VirtualMachine::ObjectStack& VirtualMachine::GetObjectStack()
		{
			return objectStack;
		}

		GarbageCollector& VirtualMachine::GetGC()
		{
			return GC;
		}
	}
}
#undef RET_CS_POP
#undef PRINTPREVFRAME
#undef PRINTFRAME_2
#undef PRINTFRAME