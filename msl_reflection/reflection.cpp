#include "reflection.h"

using ERROR = MSL::VM::VirtualMachine::ERROR;

void GetType(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* object = GetUnderlyingObject(stack.back());
	switch (object->type)
	{
	case Type::CLASS:
	case Type::NAMESPACE:
		stack.back() = object;
		break;
	case Type::CLASS_OBJECT:
		stack.back() = static_cast<ClassObject*>(object)->type->wrapper;
		break;
	default:
		stack.back() = vm->GetClassPrimitive(object);
		break;
	}
}

void GetNamespace(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* name = GetUnderlyingObject(stack.back());
	stack.pop_back();
	if (!AssertType(vm, name, Type::STRING)) return;

	auto& nsName = static_cast<StringObject*>(name)->value;

	auto ns = vm->GetAssembly().namespaces.find(nsName);
	if (ns == vm->GetAssembly().namespaces.end())
	{
		vm->InvokeError(
			ERROR::MEMBER_NOT_FOUND,
			"current assembly does not contains namespace with name: " + nsName, 
			nsName
		);
		return;
	}
	stack.push_back(ns->second.wrapper);
}

void IsNamespaceExists(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* name = GetUnderlyingObject(stack.back());
	stack.pop_back();
	if (!AssertType(vm, name, Type::STRING)) return;

	std::string& ns = static_cast<StringObject*>(name)->value;

	if (vm->GetAssembly().namespaces.find(ns) ==
		vm->GetAssembly().namespaces.end())
	{
		stack.push_back(AllocFalse(vm->GetGC()));
	}
	else
	{
		stack.push_back(AllocTrue(vm->GetGC()));
	}
}

void ContainsMember(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* memberObject = GetUnderlyingObject(stack.back());
	stack.pop_back();
	BaseObject* callerObject = GetUnderlyingObject(stack.back());
	stack.pop_back();
	if (!AssertType(vm, memberObject, Type::STRING)) return;

	std::string& member = static_cast<StringObject*>(memberObject)->value;
	BaseObject* result = vm->GetMemberObject(callerObject, member);
	if (result == nullptr ||  // also check if object is private
	   (result->type == Type::ATTRIBUTE && !static_cast<AttributeObject*>(result)->type->isPublic()) ||
	   (result->type == Type::CLASS)    && static_cast<ClassWrapper*>(result)->type->isInternal())
	{
		stack.push_back(AllocFalse(vm->GetGC()));
		return;
	}
	stack.push_back(AllocTrue(vm->GetGC()));
}

void ContainsMethod(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* argCount = GetUnderlyingObject(stack.back());
	stack.pop_back();
	BaseObject* methodObject = GetUnderlyingObject(stack.back());
	stack.pop_back();
	BaseObject* classArgument = GetUnderlyingObject(stack.back());
	stack.pop_back();

	if (!AssertType(vm, methodObject, Type::STRING )) return;
	if (!AssertType(vm, argCount,     Type::INTEGER)) return;

	auto& methodName = static_cast<StringObject*>(methodObject)->value;
	auto& args = static_cast<IntegerObject*>(argCount)->value;

	const ClassType* classType = nullptr;
	ClassObject* classObject = nullptr;
	if (classArgument->type == Type::CLASS)
	{
		classType = static_cast<ClassWrapper*>(classArgument)->type;
	}
	else if (classArgument->type == Type::CLASS_OBJECT)
	{
		classObject = static_cast<ClassObject*>(classArgument);
		classType = classObject->type;
	}
	else
	{
		stack.push_back(AllocFalse(vm->GetGC()));
		return;
	}

	std::string method = methodName + '_' + // extra argument for `this`
		(args + int(classObject != nullptr)).to_string();
	auto methodIt = classType->methods.find(method);
	if (classType->methods.find(method) == classType->methods.end())
	{
		stack.push_back(AllocFalse(vm->GetGC()));
		return;
	}
	if (!methodIt->second.isPublic() || !methodIt->second.isStatic() && !methodIt->second.isConstructor() && classObject == nullptr)
	{
		stack.push_back(AllocFalse(vm->GetGC()));
		return;
	}
	stack.push_back(AllocTrue(vm->GetGC()));
}

void GetMember(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* childObj = GetUnderlyingObject(stack.back());
	stack.pop_back();
	BaseObject* parentObj = GetUnderlyingObject(stack.back());
	stack.pop_back();
	if (!AssertType(vm, childObj, Type::STRING)) return;

	std::string& member = static_cast<StringObject*>(childObj)->value;
	BaseObject* result = vm->GetMemberObject(parentObj, member);
	if (result == nullptr)
	{
		vm->InvokeError(
			ERROR::MEMBER_NOT_FOUND, 
			"Member with name `" + member + "` was not found in " + parentObj->ToString(),
			member
		);
	}
	stack.push_back(result);
}

#define VM_COMMAND_CREATE_INSTANCE "__VM_CREATE_INSTANCE__"

void CreateInstance(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* top = stack.back();
	stack.pop_back();
	stack.push_back(AllocString(vm->GetGC(), VM_COMMAND_CREATE_INSTANCE));
	stack.push_back(top);
	Invoke(vm);
}

void Invoke(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* object = GetUnderlyingObject(stack.back()); // probably array object
	stack.pop_back();
	BaseObject* methodObject = GetUnderlyingObject(stack.back());
	stack.pop_back();
	BaseObject* classArgument = GetUnderlyingObject(stack.back());
	stack.pop_back();

	if (!AssertType(vm, methodObject, Type::STRING)) return;

	ArrayObject::InnerType* array = nullptr;
	ClassObject* arrayClass = static_cast<ClassObject*>(object); // no type check before next line
	if (object->type != Type::CLASS_OBJECT ||
		arrayClass->attributes.find("array") == arrayClass->attributes.end() ||
		arrayClass->attributes["array"]->object->type != Type::BASE)
	{
		array = &AllocArray(vm->GetGC(), 1)->array;
		if (array->empty()) return; // OutOfMemory (?)
		(*array)[0] = { object, false };
	}
	else
	{
		array = &static_cast<ArrayObject*>(arrayClass->attributes["array"]->object)->array;
	}
	if (classArgument->type != Type::CLASS &&
		!AssertType(vm, classArgument, Type::CLASS_OBJECT)) return;

	auto& methodName = static_cast<StringObject*>(methodObject)->value;

	const ClassType* classType = nullptr;
	ClassObject* classObject = nullptr;
	if (classArgument->type == Type::CLASS)
	{
		classType = static_cast<ClassWrapper*>(classArgument)->type;
	}
	else
	{
		classObject = static_cast<ClassObject*>(classArgument);
		classType = classObject->type;
	}

	if (methodName == VM_COMMAND_CREATE_INSTANCE) // can be added by CreateInstance
	{
		methodName = classType->name;
	}
	std::string method = methodName + '_' + // add argument for `this`
		std::to_string(array->size() + (classObject != nullptr));
	auto methodIt = classType->methods.find(method);
	if (methodIt == classType->methods.end())
	{
		vm->InvokeError(
			ERROR::MEMBER_NOT_FOUND, 
			"class provided does not have method `" + methodName + 
			"` with " + std::to_string(array->size()) + " arguments, class was: " + 
			classType->namespaceName + '.' + classType->name,
			methodName
		);
		return;
	}

	if (!methodIt->second.isStatic() && !methodIt->second.isConstructor() && classObject == nullptr)
	{
		vm->InvokeError(
			ERROR::INVALID_METHOD_CALL, 
			"tried to call non-static method using class type as argument", 
			methodName
		);
		return;
	}

	CallPath newFrame;
	newFrame.SetNamespace(&classType->namespaceName);
	newFrame.SetClass(&classType->name);
	newFrame.SetMethod(&method);
	vm->GetCallStack().push_back(std::move(newFrame));

	if (methodIt->second.isStatic())
	{
		stack.push_back(classType->wrapper);
	}
	else
	{
		stack.push_back(classObject);
	}
	for (const auto& param : *array)
	{
		stack.push_back(param.object);
	}
	vm->StartNewStackFrame();
}