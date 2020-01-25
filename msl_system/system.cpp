#include "system.h"

using ERROR = MSL::VM::VirtualMachine::ERROR;

void ReflectionGetType(PARAMS)
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

void ReflectionGetNamespace(PARAMS)
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

void ReflectionIsNamespaceExists(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* name = GetUnderlyingObject(stack.back());
	stack.pop_back();
	if (!AssertType(vm, name, Type::STRING)) return;

	std::string& ns = static_cast<StringObject*>(name)->value;

	if (vm->GetAssembly().namespaces.find(ns) ==
		vm->GetAssembly().namespaces.end())
	{
		stack.push_back(vm->AllocFalse());
	}
	else
	{
		stack.push_back(vm->AllocTrue());
	}
}

void ReflectionContainsMember(PARAMS)
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
		(result->type == Type::CLASS) && static_cast<ClassWrapper*>(result)->type->isInternal())
	{
		stack.push_back(vm->AllocFalse());
		return;
	}
	stack.push_back(vm->AllocTrue());
}

void ReflectionContainsMethod(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* argCount = GetUnderlyingObject(stack.back());
	stack.pop_back();
	BaseObject* methodObject = GetUnderlyingObject(stack.back());
	stack.pop_back();
	BaseObject* classArgument = GetUnderlyingObject(stack.back());
	stack.pop_back();

	if (!AssertType(vm, methodObject, Type::STRING)) return;
	if (!AssertType(vm, argCount, Type::INTEGER)) return;

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
		stack.push_back(vm->AllocFalse());
		return;
	}

	std::string method = methodName + '_' + // extra argument for `this`
		(args + int(classObject != nullptr)).to_string();
	auto methodIt = classType->methods.find(method);
	if (classType->methods.find(method) == classType->methods.end())
	{
		stack.push_back(vm->AllocFalse());
		return;
	}
	if (!methodIt->second.isPublic() || !methodIt->second.isStatic() && !methodIt->second.isConstructor() && classObject == nullptr)
	{
		stack.push_back(vm->AllocFalse());
		return;
	}
	stack.push_back(vm->AllocTrue());
}

void ReflectionGetMember(PARAMS)
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

void ReflectionCreateInstance(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* top = stack.back();
	stack.pop_back();
	stack.push_back(vm->AllocString(VM_COMMAND_CREATE_INSTANCE));
	stack.push_back(top);
	ReflectionInvoke(vm);
}

void ReflectionInvoke(PARAMS)
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
		array = &vm->AllocArray(1)->array;
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
	if (methodIt->second.isStatic())
		vm->InvokeStaticMethod(methodIt->first, classType);
	else
		vm->InvokeObjectMethod(methodIt->first, classObject);
}

void MathSqrt(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::sqrt(object->value);
}

void MathAbs(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::abs(object->value);
}

void MathSin(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::sin(object->value);
}

void MathCos(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::cos(object->value);
}

void MathTan(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::tan(object->value);
}

void MathExp(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::exp(object->value);
}

void MathAtan(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::atan(object->value);
}

void MathAcos(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::acos(object->value);
}

void MathAsin(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::asin(object->value);
}

void MathLog10(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::log10(object->value);
}

void MathLog2(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::log2(object->value);
}

void MathLog(PARAMS)
{
	auto* top = vm->GetObjectStack().back();
	if (!AssertType(vm, top, MSL::VM::Type::FLOAT)) return;
	auto* object = static_cast<MSL::VM::FloatObject*>(top);
	object->value = std::log(object->value);
}

void GCCollect(PARAMS)
{
	vm->GetGC().Collect(vm->GetAssembly(), vm->GetCallStack(), vm->GetObjectStack());
	vm->GetObjectStack().push_back(vm->AllocNull());
}

void GCDisable(PARAMS)
{
	if (vm->GetConfig().execution.safeMode)
	{
		vm->InvokeError(ERROR::INVALID_METHOD_CALL, "GC.Disable() function is disabled is MSL VM safe mode", "Disable");
		return;
	}
	vm->GetConfig().GC.allowCollect = false;
	vm->GetObjectStack().push_back(vm->AllocNull());
}

void GCEnable(PARAMS)
{
	if (vm->GetConfig().execution.safeMode)
	{
		vm->InvokeError(ERROR::INVALID_METHOD_CALL, "GC.Enable() function is disabled is MSL VM safe mode", "Enable");
		return;
	}
	vm->GetConfig().GC.allowCollect = true;
	vm->GetObjectStack().push_back(vm->AllocNull());
}

void GCReleaseMemory(PARAMS)
{
	vm->GetGC().ReleaseFreeMemory();
	vm->GetObjectStack().push_back(vm->AllocNull());
}

void GCSetMinimalMemory(PARAMS)
{
	if (vm->GetConfig().execution.safeMode)
	{
		vm->InvokeError(
			ERROR::INVALID_METHOD_CALL,
			"GC.SetMinimalMemory() function is disabled is MSL VM safe mode",
			"SetMinimalMemory"
		);
		return;
	}
	BaseObject* value = vm->GetObjectStack().back();
	vm->GetObjectStack().pop_back(); // pop value
	if (!AssertType(vm, value, Type::INTEGER))
		return;
	IntegerObject::InnerType& memory = static_cast<IntegerObject*>(value)->value;
	if (memory < 0 || memory > std::numeric_limits<uint64_t>::max())
	{
		vm->InvokeError(
			ERROR::INVALID_ARGUMENT,
			"value parameter was invalid in GC.SetMinimalMemory(this, value) method: " + memory.to_string(),
			memory.to_string()
		);
		return;
	}
	uint64_t val = std::stoull(memory.to_string());
	vm->GetConfig().GC.minMemory = val;
	vm->GetObjectStack().push_back(vm->AllocNull());
}

void GCSetMaximalMemory(PARAMS)
{
	if (vm->GetConfig().execution.safeMode)
	{
		vm->InvokeError(
			ERROR::INVALID_METHOD_CALL,
			"GC.SetMaximalMemory() function is disabled is MSL VM safe mode",
			"SetMaximalMemory"
		);
		return;
	}
	BaseObject* value = vm->GetObjectStack().back();
	vm->GetObjectStack().pop_back(); // pop value
	if (!AssertType(vm, value, Type::INTEGER))
		return;
	IntegerObject::InnerType& memory = static_cast<IntegerObject*>(value)->value;
	if (memory < 0 || memory > std::numeric_limits<uint64_t>::max())
	{
		vm->InvokeError(
			ERROR::INVALID_ARGUMENT,
			"value parameter was invalid in GC.SetMaximalMemory(this, value) method: " + memory.to_string(),
			memory.to_string()
		);
		return;
	}
	uint64_t val = std::stoull(memory.to_string());
	vm->GetConfig().GC.maxMemory = val;
	vm->GetObjectStack().push_back(vm->AllocNull());
}

void GCSetLogPermissions(PARAMS)
{
	BaseObject* value = vm->GetObjectStack().back();
	vm->GetObjectStack().pop_back(); // pop value
	if (value->type != Type::TRUE && value->type != Type::FALSE)
	{
		vm->InvokeError(
			ERROR::INVALID_TYPE,
			"GC.SetLogPermission(this, value) accepts only Boolean as parameter",
			value->ToString()
		);
		return;
	}

	if (value->type == Type::TRUE)
		vm->GetGC().SetLogStream(vm->GetConfig().streams.error);
	else
		vm->GetGC().SetLogStream(vm->GetConfig().GC.log);

	vm->GetObjectStack().push_back(vm->AllocNull());
}

void ConsolePrint(PARAMS)
{
	BaseObject* object = GetUnderlyingObject(vm->GetObjectStack().back());

	if (vm->GetConfig().streams.out == nullptr) return;
	std::ostream& out = *vm->GetConfig().streams.out;

	switch (object->type)
	{
	case Type::NAMESPACE:
	{
		out << "namespace " << *GetObjectName(object);
		break;
	}
	case Type::CLASS:
	{
		ClassWrapper* c = static_cast<ClassWrapper*>(object);
		out << "class " << c->type->namespaceName + '.' + c->type->name;
		break;
	}
	case Type::ATTRIBUTE:
	{
		AttributeObject* attr = static_cast<AttributeObject*>(object);
		vm->GetObjectStack().push_back(attr->object);
		ConsolePrint(vm);
		return; // no PrintLine check, because it will happen inside recursion call
	}
	case Type::CLASS_OBJECT:
	{
		ClassObject* classObject = static_cast<ClassObject*>(object);
		out << classObject->type->namespaceName + '.' + classObject->type->name << " instance";
		break;
	}
	default:
	{
		out << object->ToString();
		break;
	}
	}
}

void ConsolePrintLine(PARAMS)
{
	ConsolePrint(vm);
	if (vm->GetConfig().streams.out != nullptr) *vm->GetConfig().streams.out << std::endl;
}

void ConsoleRead(PARAMS)
{
	StringObject::InnerType str;
	if (vm->GetConfig().streams.in != nullptr)
		*vm->GetConfig().streams.in >> str;

	vm->GetObjectStack().push_back(vm->AllocString(str));
}

void ConsoleReadInt(PARAMS)
{
	std::string str = "0";
	if (vm->GetConfig().streams.in != nullptr)
		*vm->GetConfig().streams.in >> str;

	vm->GetObjectStack().push_back(vm->AllocInteger(str));
}

void ConsoleReadFloat(PARAMS)
{
	std::string str = "0.0";
	if (vm->GetConfig().streams.in != nullptr)
		*vm->GetConfig().streams.in >> str;

	vm->GetObjectStack().push_back(vm->AllocFloat(str));
}

void ConsoleReadLine(PARAMS)
{
	StringObject::InnerType str;
	if (vm->GetConfig().streams.in != nullptr)
		std::getline(*vm->GetConfig().streams.in, str);

	vm->GetObjectStack().push_back(vm->AllocString(str));
}

void ConsoleReadBool(PARAMS)
{
	std::string str;
	if (vm->GetConfig().streams.in != nullptr)
		*vm->GetConfig().streams.in >> str;

	if (str == "1" || str == "True" || str == "true")
	{
		vm->GetObjectStack().push_back(vm->AllocTrue());
	}
	else
	{
		vm->GetObjectStack().push_back(vm->AllocFalse());
	}
}

static ClassObject* InitArray(PARAMS, size_t size)
{
	const ClassType* arrayClass = &vm->GetAssembly().namespaces["System"].classes["Array"];
	ClassObject* arr = vm->AllocClassObject(arrayClass);
	arr->attributes["array"]->object = vm->AllocArray(size);
	return arr;
}

static ArrayObject::InnerType& GetArrayReference(BaseObject* array)
{
	array = GetUnderlyingObject(array);
	return static_cast<ArrayObject*>(array)->array;
}

static momo::BigInteger MaxSize = (unsigned long long)std::numeric_limits<size_t>::max();
static momo::BigInteger MinSize = 0;

void ArrayConstructor(PARAMS)
{
	BaseObject* size = vm->GetObjectStack().back();
	vm->GetObjectStack().pop_back();
	if (!AssertType(vm, size, Type::INTEGER)) return;

	auto& value = static_cast<IntegerObject*>(size)->value;
	if (value < MinSize || value > MaxSize)
	{
		vm->InvokeError(
			ERROR::INVALID_ARGUMENT,
			"cannot create Array instance with size: " + value.to_string(),
			value.to_string()
		);
		return;
	};
	vm->GetObjectStack().push_back(InitArray(vm, std::stoul(value.to_string())));
}

void ArrayGetByIndex(PARAMS)
{
	auto& stack = vm->GetObjectStack();

	BaseObject* index = stack.back();
	stack.pop_back(); // pop index

	auto& array = GetArrayReference(stack.back());
	stack.pop_back(); // pop array

	if (!AssertType(vm, index, Type::INTEGER)) return;

	auto& value = static_cast<IntegerObject*>(index)->value;
	if (value < MinSize || value > MaxSize)
	{
		vm->InvokeError(
			ERROR::INVALID_ARGUMENT,
			"cannot get element of Array with index: " + value.to_string(),
			value.to_string()
		);
		return;
	};
	size_t idx = (size_t)std::stoull(value.to_string());
	stack.push_back(vm->AllocLocal("__array", array[idx]));
}

void ArraySize(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	auto& array = GetArrayReference(stack.back());
	stack.pop_back();
	stack.push_back(vm->AllocInteger(array.size()));
}

void ArrayToString(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	auto& array = GetArrayReference(stack.back());
	stack.pop_back();

	auto output = vm->AllocString("[");
	stack.push_back(output);
	for (int i = 0; i < int(array.size()); i++)
	{
		if (i != 0) output->value += ", ";
		bool isString = array[i].object->type == Type::STRING;
		if (isString) output->value += '"';

		stack.push_back(array[i].object);
		vm->PerformALUCall(OPCODE::SUM_OP, 2, vm->GetCallStack().back().GetFrame());
		output = static_cast<StringObject*>(stack.back());

		if (isString) output->value += '"';
	}
	output->value += ']';
}

void ArrayPop(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	auto& array = GetArrayReference(stack.back());
	stack.pop_back();

	if (array.empty())
	{
		vm->InvokeError(
			ERROR::INVALID_METHOD_CALL,
			"Array.Pop() was called on empty array",
			"Pop"
		);
		return;
	}
	stack.push_back(array.back().object);
	array.pop_back();
}

void ArrayAppend(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* object = stack.back();
	stack.pop_back(); // pop object
	auto& array = GetArrayReference(stack.back());
	array.push_back({ object, false });
}

void ArraySort(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	auto& array = GetArrayReference(stack.back());
	try
	{
		std::sort(array.begin(), array.end(), [&stack, &vm](MSL::VM::Local& o1, MSL::VM::Local& o2)
			{
				stack.push_back(o1.object);
				stack.push_back(o2.object);
				vm->PerformALUCall(OPCODE::CMP_L, 2, vm->GetCallStack().back().GetFrame());
				if (vm->GetErrors() != 0)
					throw std::exception("invalid compare");
				auto output = stack.back();
				stack.pop_back();
				if (output->type == Type::TRUE)
					return true;
				if (output->type == Type::FALSE)
					return false;
				AssertType(vm, output, Type::TRUE);
				throw std::exception("invalid compare");
			});
	}
	catch (std::exception&)
	{
		return;
	}
}