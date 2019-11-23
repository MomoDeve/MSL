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

namespace MSL
{
	#ifdef MSL_VM_DEBUG
	#define VM_TAG(tag) *config.streams.out << std::endl << "[" #tag "]: "
	#define VM_MESSAGE(msg) *config.streams.out << msg
	#define VM_DEBUG(msg) VM_TAG(DEBUG) <<  msg
	#else
	#define VM_DEBUG(msg)
	#define VM_TAG(tag)
	#define VM_MESSAGE(msg)
	#endif
	#define PRINTFRAME_2(_class, _method) DisplayInfo("current frame: " + GetFullClassType(_class) + '.' + GetFullMethodType(_method))
	#define PRINTFRAME PRINTFRAME_2(frame->_class, frame->_method)
	#define PRINTPREVFRAME CallPath back = std::move(callStack.back()); \
						   callStack.pop_back(); \
						   if (!callStack.empty()) \
						        DisplayInfo("called from frame: " +  \
								*callStack.back().GetNamespace() + \
								'.' + *callStack.back().GetClass() + \
								'.' + *callStack.back().GetMethod()); \
						   callStack.push_back(std::move(back))

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

			errors |= ERROR::OBJECT_NOT_FOUND;
			DisplayError("object with name: `" + objectName + "` was not found");
			std::string className = *GetObjectName(_class) + (_method->isStatic() ? "[static]" : "[this]");
			DisplayInfo("current frame: " + _namespace->name + '.' + className + '.' + GetFullMethodType(_method));
			return nullptr;
		}

		BaseObject* VirtualMachine::GetMemberObject(BaseObject* object, const std::string& memberName, bool printHelp)
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
				else if(!ns->type->classes.empty() && printHelp)
				{
					DisplayInfo("available classes:");
					for (const auto& c : ns->type->classes)
					{
						DisplayInfo('\t' + GetFullClassType(&c.second));
					}
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
					else if(printHelp && (!obj->type->staticAttributes.empty() || !obj->type->objectAttributes.empty()))
					{
						std::stringstream out;
						for (const auto& attr : obj->type->staticAttributes)
						{
							if(attr.second.isPublic()) out << "\tststaic" << attr.second.name << '\n';
						}
						for (const auto& attr : obj->type->objectAttributes)
						{
							if (attr.second.isPublic()) out << '\t' << attr.second.name << '\n';
						}
						std::string buff = out.str();
						buff.pop_back();
						if (buff.size() > 0)
						{
							DisplayInfo("available attributes:");
							DisplayInfo(std::move(buff));
						}
					}
				}
			}
			break;
			case Type::CLASS:
			{
				ClassWrapper* cl = static_cast<ClassWrapper*>(object);
				memberObject = GetMemberObject(cl->type->staticInstance, memberName, printHelp);
			}
			break;
			}
			return memberObject;
		}

		ClassWrapper* VirtualMachine::GetPrimitiveClass(BaseObject* object)
		{
			auto& ns = assembly.namespaces["System"];
			ClassType* cl = nullptr;
			switch (object->type)
			{
			case Type::INTEGER:
				cl = &ns.classes.find("Integer")->second;
				break;
			case Type::FLOAT:
				cl = &ns.classes.find("Float")->second;
				break;
			case Type::STRING:
				cl = &ns.classes.find("String")->second;
				break;
			case Type::TRUE:
				cl = &ns.classes.find("True")->second;
				break;
			case Type::FALSE:
				cl = &ns.classes.find("False")->second;
				break;
			case Type::NULLPTR:
				cl = &ns.classes.find("Null")->second;
				break;
			default:
				errors |= ERROR::INVALID_STACKOBJECT;
				DisplayError("Cannot get primitive class of object with type: " + ToString(object->type));
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
							errors |= ERROR::INVALID_CALL_ARGUMENT;
							DisplayError("find two or more matching classes while resolving object type: " + objectName);
							DisplayInfo("first match was: " + GetFullClassType(classWrap->type));
							DisplayInfo("also found: " + GetFullClassType(otherClass));
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
				errors |= ERROR::STACKOVERFLOW;
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

			// in case method was not found
			if (frame->_method == nullptr)
			{
				// probably class constructor was called (class name => method name)
				if (frame->_namespace != nullptr)
				{
					std::string methodName = *callStack.back().GetMethod();
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
							callStack.pop_back();
							callStack.push_back(std::move(newFrame));
							StartNewStackFrame();
							return;
						}
						else if (classType->isAbstract()) // if class is abstract, constructor is not found too
						{
							errors |= ERROR::ABSTRACT_MEMBER_CALL;
							DisplayError("cannot create instance of abstract class: " + GetFullClassType(classType));
						}
						else if (classType->isStatic())
						{
							errors |= ERROR::MEMBER_NOT_FOUND;
							DisplayError("cannot create instance of static class: " + GetFullClassType(classType));
						}
						else // probably constructor is just missing
						{
							errors |= ERROR::INVALID_METHOD_CALL;
							DisplayError("could not call class " + GetFullClassType(classType) + " constructor: " + methodName);
							DisplayInfo("available constructors of this class:");
							for (int i = 0; i < 17; i++) // find all possible class constructors (with less than 17 parameters, at least)
							{
								std::string methodName = classType->name + '_' + std::to_string(i);
								const auto method = GetMethodOrNull(classType, methodName);
								if (method != nullptr)
								{
									DisplayInfo('\t' + GetFullClassType(classType) + '.' + GetFullMethodType(method));
								}
							}
						}
					}
				}
				errors |= ERROR::MEMBER_NOT_FOUND;
				DisplayError("method passed to frame was not found: " + *callStack.back().GetNamespace() + '.' + *callStack.back().GetClass() + '.' + *callStack.back().GetMethod());
				if (frame->_class != nullptr)
				{
					DisplayInfo("available class methods: ");
					for (const auto& method : frame->_class->methods)
					{
						if(method.second.isPublic())
							DisplayInfo("\t" + ((method.second.isStatic() ? "static " : " ") + frame->_class->name + '.' + GetFullMethodType(&method.second)));
					}
					DisplayInfo("\n");
				}

				// displaying caller frame for debug
				errors |= ERROR::MEMBER_NOT_FOUND;
				PRINTPREVFRAME;
				return;
			}
			// if member if private, and it is called from another class, call is invalid
			if (!frame->_method->isPublic() && callStack.size() > 1)
			{
				CallPath& prev = callStack[callStack.size() - 2];
				if (*prev.GetNamespace() != frame->_namespace->name || *prev.GetClass() != frame->_class->name)
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
				callStack.push_back(std::move(staticConstructorFrame));
				objectStack.push_back(frame->_class->wrapper);
				StartNewStackFrame();
				if (errors != 0)
				{
					// error already occured
					DisplayError("static constructor caused a fatal error: " + GetFullClassType(frame->_class) + '.' + GetMethodActualName(constructor) + "()");
					return;
				}
				objectStack.pop_back();
			}

			// if class is system class, call must be delegated to SystemCall function
			if (frame->_class->isSystem())
			{
				PerformSystemCall(frame->_class, frame->_method, frame);
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
					VM_DEBUG(ToString(op) << ' ' << *objectName);
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
					object = ResolveReference(object, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (object == nullptr) return; // error is handled in ResolveReference method
					object = GetUnderlyingObject(object);

					objectStack.pop_back();
					BaseObject* index = objectStack.back();
					if(AssertType(index, Type::UNKNOWN)) index = ResolveReference(index, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (index == nullptr) return; // error is handled in ResolveReference method
					index = GetUnderlyingObject(index);

					VM_TAG(DEBUG);
					VM_MESSAGE(ToString(op) << ' ' << object->ToString());
					VM_MESSAGE('[' << index->ToString() << ']');

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
						ClassWrapper* wrap = GetPrimitiveClass(object);
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
						DisplayError("object with invalid type was passed to GET_INDEX call: " + ToString(object->type));
						PRINTFRAME;
						return;
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
						std::string className = GetMethodActualName(*GetObjectName(objectStack.back()));
						const NamespaceType* ns = static_cast<NamespaceWrapper*>(caller)->type;
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
						newFrame.SetNamespace(GetObjectName(caller));
						newFrame.SetClass(&classIt->second.name);
						newFrame.SetMethod(GetObjectName(objectStack.back()));
						callStack.push_back(std::move(newFrame));
					}
					break;
					case Type::INTEGER:
					case Type::FLOAT:
					case Type::STRING:
					case Type::TRUE:
					case Type::FALSE:
					case Type::NULLPTR:
					{
						objectStack[objectStack.size() - paramSize - 2] = caller;
						const ClassType* cl = GetPrimitiveClass(caller)->type;
						newFrame.SetNamespace(&cl->namespaceName);
						newFrame.SetClass(&cl->name);
						newFrame.SetMethod(GetObjectName(objectStack.back()));
						callStack.push_back(std::move(newFrame));
					}
					break;
					default:
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("caller of method was neither class object nor class type");
						DisplayInfo("called method name: " + *GetObjectName(objectStack.back()));
						DisplayInfo("caller was: " + caller->ToString());
						return;
					}
					break;
					}
					objectStack.pop_back(); // remove unknown object (function name)
				
					VM_TAG(DEBUG);
					VM_MESSAGE(ToString(op) << ' ');
					VM_MESSAGE(*callStack.back().GetNamespace() << '.');
					VM_MESSAGE(*callStack.back().GetClass() << '.');
					VM_MESSAGE(*callStack.back().GetMethod());
					
					StartNewStackFrame();
					break;
				}
				case (OPCODE::GET_MEMBER):
				{
					if (objectStack.size() < 2)
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						DisplayError("not enough objects in stack to get member");
						if (!objectStack.empty()) DisplayInfo("last one is: " + objectStack.back()->ToString());
						PRINTFRAME;
						return;
					}
					BaseObject* member = objectStack.back();
					objectStack.pop_back();
					BaseObject* calledObject = objectStack.back();
					objectStack.pop_back();
					if (AssertType(calledObject, Type::UNKNOWN)) calledObject = ResolveReference(calledObject, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (calledObject == nullptr) return; // check performed in ResolveReference method
					const std::string* memberName = GetObjectName(member);
					if (memberName == nullptr)
					{
						errors |= ERROR::INVALID_CALL_ARGUMENT;
						DisplayError("invalid member was called: " + member->ToString());
						PRINTFRAME;
						return;
					}

					BaseObject* memberObject = GetMemberObject(calledObject, *memberName);
					if (memberObject == nullptr)
					{
						errors |= ERROR::MEMBER_NOT_FOUND;
						DisplayError("member was not found: " + *GetObjectName(calledObject) + '.' + *GetObjectName(member));
						PRINTFRAME;
						return;
					}
					if (AssertType(memberObject, Type::ATTRIBUTE))
					{
						const AttributeType* type = static_cast<AttributeObject*>(memberObject)->type;
						if (!AssertType(calledObject, Type::CLASS_OBJECT) &&
							!AssertType(calledObject, Type::CLASS, "trying to get attribute from object which is neither class, nor class object", frame))
							return;

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
								errors |= ERROR::PRIVATE_MEMBER_ACCESS;
								DisplayError("trying to access class private member: " + GetFullClassType(classType) + '.' + type->name);
								PRINTFRAME;
								return;
							}
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
						if (AssertType(obj, Type::UNKNOWN)) obj = ResolveReference(obj, frame->locals, frame->_method, frame->classObject, frame->_namespace);
						if (obj == nullptr) return; // ResolveReference handles errors
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
					if (AssertType(object, Type::UNKNOWN)) object = ResolveReference(object, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					if (object == nullptr) return;
					object = GetUnderlyingObject(object);

					if (AssertType(object, Type::CLASS_OBJECT))
					{
						InvokeObjectMethod("ToBoolean_1", static_cast<ClassObject*>(object));
						if (errors != 0)
						{
							// error already occured
							DisplayError("could not convert class object into boolean: " + GetFullClassType(static_cast<ClassObject*>(object)->type));
							PRINTFRAME;
							return;
						}
						object = objectStack.back();
						objectStack.pop_back();
					}
					if (AssertType(object, Type::TRUE))
						frame->offset = frame->_method->labels[label];
					else if (object->type != Type::FALSE && object->type != Type::NULLPTR)
					{
						errors |= ERROR::INVALID_METHOD_CALL;
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
					if (AssertType(object, Type::UNKNOWN))
					{
						object = ResolveReference(object, frame->locals, frame->_method, frame->classObject, frame->_namespace);
					}
					if (object == nullptr) return;
					object = GetUnderlyingObject(object);

					if (AssertType(object, Type::CLASS_OBJECT))
					{
						InvokeObjectMethod("ToBoolean_1", static_cast<ClassObject*>(object));
						if (errors != 0)
						{
							// error already occured
							DisplayError("could not convert class object into boolean: " + GetFullClassType(static_cast<ClassObject*>(object)->type));
							PRINTFRAME;
							return;
						}
						object = objectStack.back();
						objectStack.pop_back();
					}
					if (AssertType(object, Type::FALSE) || AssertType(object, Type::NULLPTR))
						frame->offset = frame->_method->labels[label];
					else if (!AssertType(object, Type::TRUE, "object cannot be implicitly converted to boolean", frame)) return;
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
					callStack.pop_back();
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
					else if (AssertType(objectStack.back(), Type::UNKNOWN))
					{
						objectStack.back() = ResolveReference(objectStack.back(), frame->locals, frame->_method, frame->classObject, frame->_namespace);
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
			callStack.pop_back();
			if (objectStack.empty())
			{
				errors |= ERROR::OBJECTSTACK_EMPTY;
				DisplayError("object stack was empty but expected to have SystemCall arguments");
				DisplayInfo("execution interrupted in method: " + GetFullClassType(_class) + '.' + GetFullMethodType(_method));
				return;
			}
			else if (_class->name == "Reflection")
			{
				if (_method->name == "GetType_1")
				{
					BaseObject* object = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back(); // deleting reflection class reference
					switch (object->type)
					{
					case Type::NAMESPACE:
						objectStack.push_back(object);
						break;
					case Type::CLASS_OBJECT:
						objectStack.push_back(static_cast<ClassObject*>(object)->type->wrapper);
						break;
					case Type::CLASS:
						objectStack.push_back(object);
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
				else if (_method->name == "GetNamespace_1")
				{
					BaseObject* name = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back();
					if (!AssertType(name, Type::STRING, "namespace name must be a string variable", frame))
						return;

					std::string& ns = static_cast<StringObject*>(name)->value;

					auto namespaceIt = assembly.namespaces.find(ns);
					if (namespaceIt == assembly.namespaces.end())
					{
						errors |= ERROR::OBJECT_NOT_FOUND;
						DisplayError("current assembly does not contains namespace with name: " + ns);
						return;
					}
					objectStack.push_back(namespaceIt->second.wrapper);
				}
				else if (_method->name == "IsNamespaceExists_1")
				{
					BaseObject* name = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back();
					if (!AssertType(name, Type::STRING, "namespace name must be a string variable", frame))
						return;

					std::string& ns = static_cast<StringObject*>(name)->value;

					if (assembly.namespaces.find(ns) == assembly.namespaces.end())
					{
						objectStack.push_back(AllocFalse());
					}
					else
					{
						objectStack.push_back(AllocTrue());
					}
				}
				else if (_method->name == "ContainsMember_2")
				{
					BaseObject* memberObject = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					BaseObject* callerObject = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back();
					if (!AssertType(memberObject, Type::STRING, "member object name must be a string variable", frame))
						return;

					std::string& member = static_cast<StringObject*>(memberObject)->value;
					BaseObject* result = GetMemberObject(callerObject, member, false);
					if (result == nullptr ||  // also check if object is private
					   (AssertType(result, Type::ATTRIBUTE) && !static_cast<AttributeObject*>(result)->type->isPublic()) ||
						AssertType(result, Type::CLASS) && static_cast<ClassWrapper*>(result)->type->isInternal())
					{
						objectStack.push_back(AllocFalse());
						return;
					}
					objectStack.push_back(AllocTrue());
				}
				else if (_method->name == "ContainsMethod_3")
				{
					BaseObject* argCount = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					BaseObject* methodObject = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					BaseObject* classArgument = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back(); // pop reflection reference

					if (!AssertType(methodObject, Type::STRING, "method name must be a String object", frame))
						return;

					if (!AssertType(argCount, Type::INTEGER, "argCount parameter must be an Integer object", frame))
						return;

					StringObject::InnerType& methodName = static_cast<StringObject*>(methodObject)->value;
					IntegerObject::InnerType& args = static_cast<IntegerObject*>(argCount)->value;

					const ClassType* classType = nullptr;
					ClassObject* classObject = nullptr;
					if (AssertType(classArgument, Type::CLASS))
					{
						classType = static_cast<ClassWrapper*>(classArgument)->type;
					}
					else if (AssertType(classArgument, Type::CLASS_OBJECT))
					{
						classObject = static_cast<ClassObject*>(classArgument);
						classType = classObject->type;
					}
					else
					{
						objectStack.push_back(AllocFalse());
						return;
					}

					std::string method = methodName + '_' + (args + int(classObject != nullptr)).to_string();
					auto methodIt = classType->methods.find(method);
					if (classType->methods.find(method) == classType->methods.end())
					{
						objectStack.push_back(AllocFalse());
						return;
					}
					if (!methodIt->second.isPublic() || !methodIt->second.isStatic() && !methodIt->second.isConstructor() && classObject == nullptr)
					{
						objectStack.push_back(AllocFalse());
						return;
					}
					
					objectStack.push_back(AllocTrue());
				}
				else if (_method->name == "GetMember_2")
				{
					BaseObject* childObj = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					BaseObject* parentObj = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back();
					if (!AssertType(childObj, Type::STRING, "child object must be a string variable", frame))
						return;
					
					std::string& member = static_cast<StringObject*>(childObj)->value;
					BaseObject* result = GetMemberObject(parentObj, member);
					if (result == nullptr)
					{
						errors |= ERROR::MEMBER_NOT_FOUND;
						DisplayError("Member with name " + member + " was not found in " + parentObj->ToString());
						return;
					}
					objectStack.push_back(result);
				}
				else if (_method->name == "CreateInstance_2")
				{
					BaseObject* top = objectStack.back();
					objectStack.pop_back();
					objectStack.push_back(AllocString("__VM_CREATE_INSTANCE__"));
					objectStack.push_back(top);
					callStack.emplace_back();
					const MethodType* method = &_class->methods.find("Invoke_3")->second;
					PerformSystemCall(_class, method, frame);
				}
				else if (_method->name == "Invoke_3")
				{
					BaseObject* object = GetUnderlyingObject(objectStack.back()); // probably array object
					objectStack.pop_back();
					BaseObject* methodObject = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					BaseObject* classArgument = GetUnderlyingObject(objectStack.back());
					objectStack.pop_back();
					objectStack.pop_back(); // pop reflection reference

					if (!AssertType(methodObject, Type::STRING, "method name must be a String object", frame))
						return;

					ArrayObject::InnerType* array = nullptr;
					ClassObject* arrayClass = static_cast<ClassObject*>(object); // no type check before next line
					if (!AssertType(object, Type::CLASS_OBJECT) ||
						arrayClass->attributes.find("array") == arrayClass->attributes.end() ||
						!AssertType(arrayClass->attributes["array"]->object, Type::BASE))
					{
						array = &AllocArray(1)->array;
						if (array->empty()) return;
						(*array)[0] = { object, false };
					}
					else
					{
						array = &static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
					}
					if (!AssertType(classArgument, Type::CLASS_OBJECT) && !AssertType(classArgument, Type::CLASS, "class type expected as a parameter", frame))
						return;

					StringObject::InnerType& methodName = static_cast<StringObject*>(methodObject)->value;

					const ClassType* classType = nullptr;
					ClassObject* classObject = nullptr;
					if (AssertType(classArgument, Type::CLASS))
					{
						classType = static_cast<ClassWrapper*>(classArgument)->type;
					}
					else
					{
						classObject = static_cast<ClassObject*>(classArgument);
						classType = classObject->type;
					}

					if (methodName == "__VM_CREATE_INSTANCE__") // added by VM
					{
						methodName = classType->name;
					}
					std::string method = methodName + '_' + std::to_string(array->size() + (classObject != nullptr));
					auto methodIt = classType->methods.find(method);
					if (methodIt == classType->methods.end())
					{
						errors |= ERROR::MEMBER_NOT_FOUND;
						DisplayError("class provided does not have method: " + method + ", class was: " + GetFullClassType(classType));
						PRINTFRAME_2(_class, _method);
						return;
					}

					if (!methodIt->second.isStatic() && !methodIt->second.isConstructor() && classObject == nullptr)
					{
						errors |= ERROR::INVALID_METHOD_CALL;
						DisplayError("tried to call non-static method using class type as argument");
						PRINTFRAME_2(_class, _method);
						return;
					}

					CallPath newFrame;
					newFrame.SetNamespace(&classType->namespaceName);
					newFrame.SetClass(&classType->name);
					newFrame.SetMethod(&method);
					callStack.push_back(std::move(newFrame));

					if (methodIt->second.isStatic())
					{
						objectStack.push_back(classType->wrapper);
					}
					else
					{
						objectStack.push_back(classObject);
					}
					for (const auto& param : *array)
					{
						objectStack.push_back(param.object);
					}
					StartNewStackFrame();
				}
			} 
			else if (_class->name == "Array")
			{
				if (_method->name == "Array_0" || _method->name == "Array_1")
				{
					size_t arraySize = 0;
					BaseObject* size = objectStack.back();
					objectStack.pop_back(); // pop size. If not exists, class reference is popped anyway
					if (AssertType(size, Type::INTEGER))
					{
						objectStack.pop_back(); // delete class reference
						IntegerObject::InnerType& value = static_cast<IntegerObject*>(size)->value;
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
					ClassObject* arrayClass = static_cast<ClassObject*>(objectStack.back());
					ArrayObject* arrayObject = static_cast<ArrayObject*>(arrayClass->attributes["array"]->object);
					objectStack.pop_back(); // pop array object

					if (!AssertType(index, Type::INTEGER, "invalid argument was passed as array index", frame)) return;
					
					IntegerObject::InnerType& value = static_cast<IntegerObject*>(index)->value;
					if (value >= 0 && value < (unsigned long long)arrayObject->array.size())
						idx = std::stol(value.to_string());
					else
					{
						errors |= ERROR::INVALID_CALL_ARGUMENT;
						DisplayError("cannot access Array member with index = " + value.to_string());
						PRINTFRAME_2(_class, _method);
					}

					std::string name = "System.Array.array[" + std::to_string(idx) + ']';
					objectStack.push_back(AllocLocal(name, arrayObject->array[idx]));
				}
				else if (_method->name == "Size_1")
				{
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = static_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
					objectStack.push_back(AllocInteger(std::to_string(objects.size())));
				}
				else if (_method->name == "Empty_1")
				{
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = static_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
					if (objects.empty())
					{
						objectStack.push_back(AllocTrue());
					}
					else
					{
						objectStack.push_back(AllocFalse());
					}
				}
				else if (_method->name == "ToString_1")
				{
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = static_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;

					objectStack.push_back(AllocString("["));
					for (int i = 0; i < int(objects.size()); i++)
					{
						bool isString = AssertType(objects[i].object, Type::STRING);
						if (isString) static_cast<StringObject*>(objectStack.back())->value += '"';

						objectStack.push_back(objects[i].object);
						PerformALUCall(OPCODE::SUM_OP, 2, frame);

						if (isString) static_cast<StringObject*>(objectStack.back())->value += '"';
						
						if (i != int(objects.size()) - 1)
						{
							static_cast<StringObject*>(objectStack.back())->value += ", ";
						}
					}
					static_cast<StringObject*>(objectStack.back())->value += ']';
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
					ClassObject* arrayClass = static_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
					objectStack.push_back(AllocInteger(std::to_string(objects.size())));
				}
				else if (_method->name == "Pop_1")
				{
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = static_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
					if (!objects.empty())
					{
						BaseObject* object = objects.back().object;
						objects.pop_back();
						objectStack.push_back(object);
					}
					else
					{
						objectStack.push_back(AllocNull());
					}
				}
				else if (_method->name == "Append_2")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back(); // pop object
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array
					ClassObject* arrayClass = static_cast<ClassObject*>(array);
					ArrayObject::InnerType& objects = static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;

					objects.push_back({ object, false });
					objectStack.push_back(array);
				}
				else if (_method->name == "Next_2")
				{
					BaseObject* iter = objectStack.back(); 
					objectStack.pop_back(); // pop iter
					BaseObject* array = objectStack.back();
					objectStack.pop_back(); // pop array

					if (!AssertType(iter, Type::INTEGER, "Invalid iterator was passed to Array.Next(this, iter) method", frame)) return;
					
					IntegerObject::InnerType& iterValue = static_cast<IntegerObject*>(iter)->value;
					objectStack.push_back(AllocInteger((iterValue + 1).to_string()));
				}
				else
				{
					errors |= ERROR::INVALID_METHOD_CALL;
					DisplayError("Array class does not contains method: " + GetFullMethodType(_method));
					PRINTFRAME_2(_class, _method);
				}
			}
			else if (_class->name == "Dll")
			{
				if (_method->name.substr(0, 5) == "Call_") // Call with any parameters (see declaration in AddSystemNamespace)
				{
					#ifndef MSL_C_INTERFACE
					errors |= ERROR::INVALID_METHOD_CALL;
					DisplayError("Dll.Call method is not defined in MSL VM");
					return;
					#else
					int argCount = std::stoi(_method->name.substr(5, _method->name.size())) + 1; // class as extra parameter
					if (argCount > (int)objectStack.size())
					{
						errors |= ERROR::OBJECTSTACK_EMPTY;
						DisplayError("not enough arguments to call Dll.Call() method");
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

					using MSLFunction = void(*)(ObjectStack*, AssemblyType*, uint32_t*, Configuration*, GarbageCollector*);
					auto func = (MSLFunction)dllLoader.GetFunctionPointer(moduleName, functionName);
					if (func == NULL) 
					{
						errors |= ERROR::INVALID_METHOD_SIGNATURE;
						DisplayError("method " + functionName + " was not found in module: " + moduleName); 
						return; 
					}
					// DLL call
					func(&objectStack, &assembly, &errors, &config, &GC);
					#endif
				}
				else if (_method->name == "LoadLibrary_1")
				{
					#ifndef MSL_C_INTERFACE
					errors |= ERROR::INVALID_METHOD_CALL;
					DisplayError("Dll.LoadLibrary method is not defined in MSL VM");
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
					#ifndef MSL_C_INTERFACE
					errors |= ERROR::INVALID_METHOD_CALL;
					DisplayError("Dll.LoadLibrary method is not defined in MSL VM");
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

					if (!AssertType(obj, Type::INTEGER, "Integer class recieved wrong type", frame)) return;
	
					objectStack.push_back(AllocString(obj->ToString()));
				}
				else if (_method->name == "Integer_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocInteger("0"));
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
			else if (_class->name == "Float")
			{
				if (_method->name == "Float_0")
				{
					objectStack.pop_back();
					objectStack.push_back(AllocFloat("0.0"));
				}
				else if (_method->name == "ToString_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);

					objectStack.push_back(AllocString(obj->ToString()));
				}
				else if (_method->name == "Float_1")
				{
					BaseObject* object = objectStack.back();
					objectStack.pop_back();
					objectStack.pop_back(); // class reference
					objectStack.push_back(object);
				}
				else if (_method->name == "ToInt_0")
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
				else if (_method->name == "ToInt_0" || _method->name == "ToBool_0" || _method->name == "ToFloat_0")
				{
					BaseObject* obj = objectStack.back();
					objectStack.pop_back();
					obj = GetUnderlyingObject(obj);

					StringObject::InnerType& str = static_cast<StringObject*>(obj)->value;
					if (_method->name == "ToInt_0")
						objectStack.push_back(AllocInteger(str));
					else if (_method->name == "ToBool_0")
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
					objectStack.push_back(AllocInteger(std::to_string(str.size())));
				}
				else if (_method->name == "Begin_0")
				{
					objectStack.pop_back(); // string object
					objectStack.push_back(AllocInteger("0"));
				}
				else if (_method->name == "End_0")
				{
					BaseObject* str = objectStack.back();
					objectStack.pop_back(); // pop string
					if (!AssertType(str, Type::STRING, "String method must recieve string as an argument", frame)) return;
					StringObject* stringValue = static_cast<StringObject*>(str);
					objectStack.push_back(AllocInteger(std::to_string(stringValue->value.size())));
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
					objectStack.push_back(AllocInteger((iterValue + 1).to_string()));
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
						errors |= ERROR::INVALID_CALL_ARGUMENT;
						DisplayError("cannot access String element with index = " + indexValue.to_string());
						PRINTFRAME_2(_class, _method);
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

					if (!AssertType(obj, Type::TRUE, "True class recieved wrong type", frame)) return;

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

					if (!AssertType(obj, Type::FALSE, "False class recieved wrong type", frame)) return;

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

					if (!AssertType(obj, Type::NULLPTR, "Null class recieved wrong type", frame)) return;

					objectStack.push_back(AllocString(obj->ToString()));
				}
			}
			else if (_class->name == "GC")
			{
				if (_method->name == "Collect_0")
				{
					objectStack.pop_back(); // pop GC reference
					CollectGarbage(true);
					objectStack.push_back(AllocNull());
				}
				else if (_method->name == "Disable_0")
				{
					if (config.execution.safeMode)
					{
						DisplayError("GC.Disable() function is disabled is MSL VM safe mode");
						return;
					}
					objectStack.pop_back(); // pop GC reference
					config.GC.allowCollect = false;
					objectStack.push_back(AllocNull());
				}
				else if (_method->name == "Enable_0")
				{
					objectStack.pop_back(); // pop GC reference
					config.GC.allowCollect = true;
					objectStack.push_back(AllocNull());
				}
				else if (_method->name == "ReleaseMemory_0")
				{
					objectStack.pop_back(); // pop GC reference
					GC.ReleaseFreeMemory();
					objectStack.push_back(AllocNull());
				}
				else if (_method->name == "SetMinimalMemory_1")
				{
					if (config.execution.safeMode)
					{
						DisplayError("GC.SetMinimalMemory() function is disabled is MSL VM safe mode");
						return;
					}
					BaseObject* value = objectStack.back();
					objectStack.pop_back(); // pop value
					objectStack.pop_back(); // pop GC reference
					if (!AssertType(value, Type::INTEGER, "GC.SetMinimalMemory(this, value) accepts only integer as parameter", frame)) return;
					IntegerObject::InnerType& memory = static_cast<IntegerObject*>(value)->value;
					if (memory < 0 || memory > std::numeric_limits<uint64_t>::max())
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("value parameter was invalid in GC.SetMinimalMemory(this, value) method: " + memory.to_string());
						return;
					}
					uint64_t val = std::stoull(memory.to_string());
					config.GC.minMemory = val;
					objectStack.push_back(AllocNull());
				}
				else if (_method->name == "SetMaximalMemory_1")
				{
					if (config.execution.safeMode)
					{
						DisplayError("GC.SetMaximalMemory() function is disabled is MSL VM safe mode");
						return;
					}
					BaseObject* value = objectStack.back();
					objectStack.pop_back(); // pop value
					objectStack.pop_back(); // pop GC reference
					if (!AssertType(value, Type::INTEGER, "GC.SetMaximalMemory(this, value) accepts only integer as parameter", frame)) return;
					IntegerObject::InnerType& memory = static_cast<IntegerObject*>(value)->value;
					if (memory < 0 || memory > std::numeric_limits<uint64_t>::max())
					{
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("value parameter was invalid in GC.SetMaximalMemory(this, value) method: " + memory.to_string());
						return;
					}
					uint64_t val = std::stoull(memory.to_string());
					config.GC.maxMemory = val;
					objectStack.push_back(AllocNull());
				}
				else if (_method->name == "SetLogPermission_1")
				{
					BaseObject* value = objectStack.back();
					objectStack.pop_back(); // pop value
					objectStack.pop_back(); // pop GC reference
					if (!AssertType(value, Type::TRUE) &&
						!AssertType(value, Type::FALSE, "GC.SetLogPermission(this, value) accepts only Boolean as parameter", frame)) return;
					
					if (AssertType(value, Type::TRUE))
						GC.SetLogStream(config.streams.error);
					else
						GC.SetLogStream(config.GC.log);

					objectStack.push_back(AllocNull());
				}
			}
			else
			{
				errors |= ERROR::INVALID_METHOD_CALL;
				DisplayError("Invalid method was passed to PerformSystemCall() method: " + GetFullClassType(_class) + '.' + GetFullMethodType(_method));
			}
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
				if (local->ref.isConst && local->ref.object->type != Type::NULLPTR && (op == OPCODE::ASSIGN_OP || ALUinIncrMode))
				{
					errors |= ERROR::CONST_MEMBER_MODIFICATION;
					DisplayError("trying to modify const local variable: " + local->ToString() + " = " + (value ? value->ToString() : "?"));
					PRINTFRAME;
					return;
				}
				objectReference = &local->ref.object;
				break;
			}
			case Type::ATTRIBUTE:
			{
				AttributeObject* attr = static_cast<AttributeObject*>(object);
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
			case Type::TRUE:
			case Type::FALSE:
				objectReference = &object;
				if (op == OPCODE::ASSIGN_OP)
				{
					DisplayError("primitive types are not assignable, but ALU recieved primitive");
					if(value != nullptr)
						DisplayInfo("objects were: <" + ToString(value->type) + "> = " + ToString(value->type));
					PRINTFRAME;
					return;
				}
				break;
			default:
				errors |= ERROR::INVALID_STACKOBJECT;
				DisplayError("trying to perform operation with invalid object: " + object->ToString());
				PRINTFRAME;
				return;
			}

			if (parameters == 1)
				VM_DEBUG(ToString(op) << " <" << object->ToString() << "> ");
			else
				VM_DEBUG('<' << object->ToString() << "> " << ToString(op) << " <" << originalValue->ToString() << "> [assign = " << STRBOOL(ALUinIncrMode) << ']');

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
							AssertType(objectStack.back(), Type::INTEGER, "cannot convert class object to integer", frame)) return;
						
						value = objectStack.back();
						objectStack.pop_back();
						
					}
					if (AssertType(value, Type::INTEGER))
					{
						integerValue = &static_cast<IntegerObject*>(value)->value;
					}
					else if (!AssertType(value, Type::FLOAT, "cannot convert object passed to ALU to integer", frame)) return;
					else
					{
						FloatObject::InnerType* floatValue = &static_cast<FloatObject*>(value)->value;
						FloatObject* intConverted = static_cast<FloatObject*>(AllocFloat(std::to_string(integer->value.to_double())));
						PerformALUcallFloats(intConverted, floatValue, op, frame);
						break;
					}
				}
				PerformALUcallIntegers(integer, integerValue, op, frame);
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
							!AssertType(objectStack.back(), Type::STRING, "cannot convert class object to string", frame)) return;

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
							errors |= ERROR::INVALID_METHOD_CALL;
							DisplayError("cannot convert class object to float: " + GetFullClassType(valueClassObject->type) + ".ToFloat() method not found");
							PRINTFRAME;
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
						errors |= ERROR::INVALID_STACKOBJECT;
						DisplayError("cannot convert object passed to ALU to string: " + value->ToString());
						PRINTFRAME;
						return;
					}
				}
				PerformALUcallClassTypes(classWrap, classType, op, frame);
			}
			break;
			case Type::FALSE:
			case Type::TRUE:
			{
				if (ALUinIncrMode)
				{
					errors |= ERROR::INVALID_OPCODE;
					DisplayError("ALU increment mode cannot be used with bool primitives");
					PRINTFRAME;
					return;
				}
				bool b1 = (*objectReference)->type == Type::TRUE ? true : false;
				bool b2 = parameters == 2 ? (AssertType(value, Type::TRUE) ? true : false) : false; // false by default
				PerformALUcallBooleans(b1, b2, op, frame);
			}
			break;
			default:
				errors |= ERROR::INVALID_STACKOBJECT;
				DisplayError("unexpected object type found in ALU call: " + (*objectReference)->ToString());
				PRINTFRAME;
				return;
			}

			if (ALUinIncrMode)
			{
				PerformALUCall(OPCODE::ASSIGN_OP, 2, frame);
				ALUinIncrMode = false; // resets after call
			}
			VM_DEBUG("stack top: " << objectStack.back()->ToString());
		}

		void VirtualMachine::InitializeStaticMembers()
		{
			for (auto assemblyIt = assembly.namespaces.begin(); assemblyIt != assembly.namespaces.end(); assemblyIt++)
			{
				NamespaceType& ns = assemblyIt->second;
				ns.wrapper = GC.nsWrapAlloc->Alloc(&ns);
				for (auto namespaceIt = ns.classes.begin(); namespaceIt != ns.classes.end(); namespaceIt++)
				{
					ClassType& c = namespaceIt->second;
					c.wrapper = GC.classWrapAlloc->Alloc(&c);
					c.staticInstance = GC.classObjAlloc->Alloc(&c);
					c.staticInstance->attributes.reserve(c.staticAttributes.size());
					for (const auto& attr : c.staticAttributes)
					{
						AttributeObject* staticAttr = GC.attributeAlloc->Alloc(&attr.second);
						staticAttr->object = AllocNull();
						c.staticInstance->attributes[attr.second.name] = staticAttr;
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

			#define CONSTRUCTOR_0(_name) CONSTRUCTOR(_name, CONCAT(_name, __LINE__), 0); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 0)

			#define CONSTRUCTOR_1(_name, param1) CONSTRUCTOR(_name, CONCAT(_name, __LINE__), 1); \
			CONCAT(_name, __LINE__).parameters.push_back(#param1); \
			INSERT_METHOD(_name, CONCAT(_name, __LINE__), 1)

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
					DisplayInfo("[VM WARNING]: user-defined System class was replaced by VM: `" + name + '`');}\
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

			#define ATTRIBUTE(attr) AttributeType CONCAT(attr, __LINE__); CONCAT(attr, __LINE__).name = #attr; CURRENT_CLASS->objectAttributes.insert({ #attr, std::move(CONCAT(attr, __LINE__)) })

			#define PRIMITIVE_CLASS(_name) BEGIN_CLASS(_name); \
			STATIC_METHOD_0(_name); \
			STATIC_METHOD_0(ToString)
			/////////////////////////////////////////////////////////////////////////////////////////////////////

			BEGIN_NAMESPACE(System);

				BEGIN_CLASS(Reflection);
					STATIC_METHOD_1(GetType, object); // gets object type
					STATIC_METHOD_1(GetNamespace, name); // returns namespace with provided name
					STATIC_METHOD_1(IsNamespaceExists, name); // returns true if namespace exists, false either
					STATIC_METHOD_2(CreateInstance, type, args); // calls class constructor with args as parameters
					STATIC_METHOD_3(Invoke, object, method, args); // calls class method with args as parameters
					STATIC_METHOD_3(ContainsMethod, object, method, argCount); // returns true if object has method with argCount arguments
					STATIC_METHOD_2(GetMember, parent, child); // perform access to parent's child
					STATIC_METHOD_2(ContainsMember, object, member); // returns true if object has attribute member
				END_CLASS(Reflection);

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


				BEGIN_CLASS(String);
					STATIC_METHOD_0(String); // creates an empty string
					STATIC_METHOD_1(String, value); // creates copy of string from another
					STATIC_METHOD_0(ToInt); // converts string object to integer
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

				BEGIN_CLASS(Array);
					Array.modifiers &= ~ClassType::Modifiers::STATIC;
					CONSTRUCTOR_0(Array); // creates an empty array
					CONSTRUCTOR_1(Array, size); // creates array with the desired size filled with null
					METHOD_2(Append, this, object);  // adds object to the end of the array
					METHOD_2(GetByIndex, this, index); // return object stored in array by index
					METHOD_2(GetByIter, this, iter); // return object stored in array by iterator
					METHOD_2(Next, this, iter); // increments iterator of the array
					METHOD_1(Pop, this); // deletes last objects in the array and reduces its size by 1
					METHOD_1(Empty, this); // checks if the array is empty (size = 0)
					METHOD_1(Size, this); // returns array size
					METHOD_1(ToString, this); // converts array to string (all stored objects are implicitly converted to strings by default rules)
					METHOD_1(Begin, this); // return iterator to the begin of the array
					METHOD_1(End, this); // return iterator to the end of the array
					ATTRIBUTE(array); // private member which implements array storage
				END_CLASS(Array);

				BEGIN_CLASS(GC);
					STATIC_METHOD_0(Collect);
					STATIC_METHOD_0(Disable);
					STATIC_METHOD_0(Enable);
					STATIC_METHOD_0(ReleaseMemory);
					STATIC_METHOD_1(SetMinimalMemory, value);
					STATIC_METHOD_1(SetMaximalMemory, value);
					STATIC_METHOD_1(SetLogPermission, value);
				END_CLASS(GC);

			END_NAMESPACE(System);
		}

		bool VirtualMachine::LoadDll(const std::string& libName)
		{
			#ifdef MSL_C_INTERFACE
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
					DisplayError("VM hit alloc limit, allocated memory: " + utils::formatBytes(totalMemory));
					DisplayInfo("max memory of VM was set to: " + utils::formatBytes(config.GC.maxMemory));
				}
				errors |= ERROR::OUT_OF_MEMORY;
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
				errors |= ERROR::INVALID_HASH_VALUE;
				DisplayError("hash value of dependency object was invalid: " + std::to_string(hashValue));
				return false;
			}
		}

		bool VirtualMachine::AssertType(const BaseObject* object, Type type, const std::string& message, const Frame* frame)
		{
			if (AssertType(object, type)) return true;
			
			errors |= ERROR::INVALID_STACKOBJECT;
			DisplayError(message);
			DisplayInfo("expected object with type " + ToString(type) + ", found: " + ToString(object->type));
			DisplayInfo("object value was: " + object->ToString());
			if (frame != nullptr) PRINTFRAME;
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
				errors |= ERROR::MEMBER_NOT_FOUND;
				DisplayError("method name provided to InvokeObjectMethod() function not found");
				DisplayInfo("with " + GetFullClassType(object->type) + " object and method name: " + methodName);
				return;
			}
			if (method->isAbstract() || method->isStatic())
			{
				errors |= ERROR::INVALID_METHOD_SIGNATURE;
				DisplayError("trying to access abstract or static method in InvokeObjectMethod() function");
				DisplayInfo("with " + GetFullClassType(object->type) + " object and method name: " + methodName);
				return;
			}
			if (!method->isPublic())
			{
				errors |= ERROR::PRIVATE_MEMBER_ACCESS;
				DisplayError("trying to access private method in InvokeObjectMethod() function");
				DisplayInfo("with " + GetFullClassType(object->type) + " object and method name: " + methodName);
				return;
			}
			CallPath newFrame;
			newFrame.SetMethod(&methodName);
			newFrame.SetClass(&object->type->name);
			newFrame.SetNamespace(&object->type->namespaceName);
			callStack.push_back(std::move(newFrame));
			StartNewStackFrame();
		}

		void VirtualMachine::DisplayError(std::string message) const
		{
			if (config.streams.error == nullptr) return;
			DisplayInfo("\n[VM ERROR]: " + message);
			PrintObjectStack();
		}

		void VirtualMachine::DisplayInfo(std::string message) const
		{
			if (config.streams.error == nullptr) return;
			*config.streams.error << message << std::endl;
		}

		void VirtualMachine::PrintObjectStack() const
		{
			if (config.streams.error == nullptr) return;
			std::string line = "-----------------------------------------------------------\n";
			DisplayInfo("---------------------------STACK---------------------------");
			size_t count = 0;
			for (auto it = objectStack.rbegin(); it != objectStack.rend(); it++, count++)
			{
				*config.streams.error << std::left << std::setw(line.size() / 2);
				*config.streams.error << "[" + std::to_string(count) + "] " + (*it != nullptr ? (*it)->ToString() : "<error type>");
				*config.streams.error << std::right << std::setw(line.size() / 2 - 1) << (*it != nullptr ? (*it)->GetExtraInfo() : "") << std::endl;
			}
			DisplayInfo(line);
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

		void VirtualMachine::PerformALUcallIntegers(IntegerObject* int1, const IntegerObject::InnerType* int2, OPCODE op, Frame* frame)
		{
			IntegerObject* result = static_cast<IntegerObject*>(AllocInteger("0"));
			switch (op)
			{
			case OPCODE::NEGATION_OP:
				if (result->value == 0)
					objectStack.push_back(AllocTrue());
				else
					objectStack.push_back(AllocFalse());
				break;
			case OPCODE::NEGATIVE_OP:
				result->value = int1->value * -1;
				objectStack.push_back(result);
				break;
			case OPCODE::POSITIVE_OP:
				result->value = int1->value;
				objectStack.push_back(result);
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
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with two strings: " + ToString(op));
				PRINTFRAME;
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
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with string and integer: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		void VirtualMachine::PerformALUCallClassObject(ClassObject* obj, OPCODE op, Frame* frame)
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
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid opcode was passed to VM ALU: " + ToString(op));
				PRINTFRAME;
				break;
			}
		}

		void VirtualMachine::PerformALUcallFloats(FloatObject* f1, const FloatObject::InnerType* f2, OPCODE op, Frame* frame)
		{
			FloatObject* result = static_cast<FloatObject*>(AllocFloat("0.0"));
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
				objectStack.push_back(class1);
				errors |= ERROR::INVALID_OPCODE;
				DisplayError("invalid operation with two class types: " + ToString(op));
				PRINTFRAME;
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
			ClassObject* object = GC.classObjAlloc->Alloc(_class);
			object->attributes.reserve(_class->objectAttributes.size());
			for (const auto& attr : _class->objectAttributes)
			{
				AttributeObject* objectAttr = GC.attributeAlloc->Alloc(&attr.second);
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
			return GC.localObjAlloc->Alloc(local, localName);
		}

		Frame* VirtualMachine::AllocFrame()
		{
			return GC.frameAlloc->Alloc();
		}

		UnknownObject* VirtualMachine::AllocUnknown(const std::string* value)
		{
			return GC.unknownObjAlloc->Alloc(value);
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
			if ((uint32_t)size * sizeof(NullObject) > config.GC.maxMemory)
			{
				errors |= ERROR::OUT_OF_MEMORY;
				DisplayError("cannot allocate array with too big size = " + std::to_string(size) + " (" + MSL::utils::formatBytes(size) + ')');
				DisplayInfo("GC memory limit was set to " + MSL::utils::formatBytes(config.GC.maxMemory));
				return GC.arrayAlloc->Alloc(0);
			}

			ArrayObject* array = GC.arrayAlloc->Alloc(size);
			for (size_t i = 0; i < size; i++)
				array->array[i].object = AllocNull();
			return array;
		}

		StringObject* VirtualMachine::AllocString(const std::string& value)
		{
			return GC.stringAlloc->Alloc(value);
		}

		IntegerObject* VirtualMachine::AllocInteger(const std::string& value)
		{
			return GC.integerAlloc->Alloc(value);
		}

		FloatObject* VirtualMachine::AllocFloat(const std::string& value)
		{
			return GC.floatAlloc->Alloc(std::stod(value));
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
			GC.SetInitCapacity(config.GC.initCapacity);
			AddSystemNamespace();
			InitializeStaticMembers();

			if (config.execution.useUnicode)
			{
				#ifdef _WINDOWS_
				SetConsoleOutputCP(CP_UTF8);
				#else
				if(config.streams.error != nullptr)
					DisplayInfo("[VM WARNING]: useUnicode parameter was enabled, but VM supports it only on Windows system");
				#endif
			}
			if (errors != 0) return;

			if (callStack.empty())
			{
				errors |= ERROR::CALLSTACK_EMPTY | ERROR::TERMINATE_ON_LAUNCH;
				DisplayError("call stack was empty on VM launch, terminating");
				DisplayInfo("check if entry point (static Main function) is defined in MSL file");
				return;
			}

			const CallPath& path = callStack.back();
			if (path.GetNamespace() == nullptr || path.GetClass() == nullptr || path.GetMethod() == nullptr)
			{
				errors |= ERROR::TERMINATE_ON_LAUNCH;
				DisplayError("entry-point was not provided to the VM");
				return;
			}
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
			try
			{
				StartNewStackFrame();
			}
			catch (std::bad_alloc&)
			{
				errors |= ERROR::OUT_OF_MEMORY;
				DisplayError("VM hit memory limit and fatal error occured. Execution cancelled");
				if (config.streams.error != nullptr)
					*config.streams.error << "GC log information:";
				GC.SetLogStream(config.streams.error);
				GC.PrintLog();
				GC.ReleaseMemory();
			}
			catch (std::exception& e)
			{
				DisplayError("an exception occured during VM execution: ");
				if (config.streams.error != nullptr) *config.streams.error << e.what() << std::endl;
				GC.ReleaseMemory();
			}

			auto endTimePoint = std::chrono::system_clock::now();
			auto elapsedTime = endTimePoint - startTimePoint;

			if (errors == 0)
			{
				CollectGarbage(true);

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
							errors |= ERROR::INVALID_STACKOBJECT;
							DisplayError("return value from entry point function was neither integer nor null");
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

		uint32_t VirtualMachine::GetErrors() const
		{
			return errors;
		}
		std::vector<std::string> VirtualMachine::GetErrorStrings(uint32_t errors) const
		{
			std::vector<std::string> errorList;
			if (errors & ERROR::INVALID_CALL_ARGUMENT)
				errorList.push_back(STRING(ERROR::INVALID_CALL_ARGUMENT));
			if (errors & ERROR::INVALID_OPCODE)
				errorList.push_back(STRING(ERROR::INVALID_OPCODE));
			if (errors & ERROR::INVALID_STACKFRAME_OFFSET)
				errorList.push_back(STRING(ERROR::INVALID_STACKFRAME_OFFSET));
			if (errors & ERROR::OBJECTSTACK_EMPTY)
				errorList.push_back(STRING(ERROR::OBJECTSTACK_EMPTY));
			if (errors & ERROR::TERMINATE_ON_LAUNCH)
				errorList.push_back(STRING(ERROR::TERMINATE_ON_LAUNCH));
			if (errors & ERROR::OBJECT_NOT_FOUND)
				errorList.push_back(STRING(ERROR::OBJECT_NOT_FOUND));
			if (errors & ERROR::INVALID_METHOD_SIGNATURE)
				errorList.push_back(STRING(ERROR::INVALID_METHOD_SIGNATURE));
			if (errors & ERROR::MEMBER_NOT_FOUND)
				errorList.push_back(STRING(ERROR::MEMBER_NOT_FOUND));
			if (errors & ERROR::INVALID_STACKOBJECT)
				errorList.push_back(STRING(ERROR::INVALID_STACKOBJECT));
			if (errors & ERROR::STACKOVERFLOW)
				errorList.push_back(STRING(ERROR::STACKOVERFLOW));
			if (errors & ERROR::PRIVATE_MEMBER_ACCESS)
				errorList.push_back(STRING(ERROR::PRIVATE_MEMBER_ACCESS));
			if (errors & ERROR::CALLSTACK_CORRUPTION)
				errorList.push_back(STRING(ERROR::CALLSTACK_CORRUPTION));
			if (errors & ERROR::OBJECTSTACK_CORRUPTION)
				errorList.push_back(STRING(ERROR::OBJECTSTACK_CORRUPTION));
			if (errors & ERROR::CONST_MEMBER_MODIFICATION)
				errorList.push_back(STRING(ERROR::CONST_MEMBER_MODIFICATION));
			if (errors & ERROR::ABSTRACT_MEMBER_CALL)
				errorList.push_back(STRING(ERROR::ABSTRACT_MEMBER_CALL));
			if (errors & ERROR::INVALID_METHOD_CALL)
				errorList.push_back(STRING(ERROR::INVALID_METHOD_CALL));
			if (errors & ERROR::INVALID_HASH_VALUE)
				errorList.push_back(STRING(ERROR::INVALID_HASH_VALUE));
			if (errors & ERROR::OUT_OF_MEMORY)
				errorList.push_back(STRING(ERROR::OUT_OF_MEMORY));
			if (errors & ERROR::DLL_NOT_FOUND)
				errorList.push_back(STRING(ERROR::DLL_NOT_FOUND));

			return errorList;
		}
	}
}
#undef PRINTPREVFRAME
#undef PRINTFRAME_2
#undef PRINTFRAME