#include "array.h"

using VM = MSL::VM::VirtualMachine;

static ClassObject* InitArray(PARAMS, size_t size)
{
	const ClassType* arrayClass = &vm->GetAssembly().namespaces["System"].classes["Array"];
	ClassObject* arr = AllocClassObject(vm->GetGC(), arrayClass);
	arr->attributes["array"]->object = AllocArray(vm->GetGC(), size);
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
	if (value < MinSize && value > MaxSize)
	{
		vm->InvokeError(
			VM::ERROR::INVALID_ARGUMENT, 
			"cannot create Array instance with size: " + value.to_string(), 
			value.to_string()
		);
		return;
	};
	vm->GetObjectStack().push_back(InitArray(vm, std::stoul(value.to_string())));
}

void GetByIndex(PARAMS)
{
	auto& stack = vm->GetObjectStack();

	BaseObject* index = stack.back();
	stack.pop_back(); // pop index

	auto& array = GetArrayReference(stack.back());
	stack.pop_back(); // pop array

	if (!AssertType(vm, index, Type::INTEGER)) return;

	auto& value = static_cast<IntegerObject*>(index)->value;
	if (value < MinSize && value > MaxSize)
	{
		vm->InvokeError(
			VM::ERROR::INVALID_ARGUMENT,
			"cannot get element of Array with index: " + value.to_string(),
			value.to_string()
		);
		return;
	};
	size_t idx = std::stoul(value.to_string());
	stack.push_back(AllocLocal(vm->GetGC(), "__array", array[idx]));
}

void Size(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	auto& array = GetArrayReference(stack.back());
	stack.pop_back();
	stack.push_back(AllocInteger(vm->GetGC(), array.size()));
}

void ToString(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	auto& array = GetArrayReference(stack.back());
	stack.pop_back();
	
	auto output = AllocString(vm->GetGC(), "[");
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

void Pop(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	auto& array = GetArrayReference(stack.back());
	stack.pop_back();

	if (array.empty())
	{
		vm->InvokeError(
			VM::ERROR::INVALID_METHOD_CALL,
			"Array.Pop() was called on empty array",
			"Pop"
		);
		return;
	}
	stack.push_back(array.back().object);
	array.pop_back();
}

void Append(PARAMS)
{
	auto& stack = vm->GetObjectStack();
	BaseObject* object = stack.back();
	stack.pop_back(); // pop object
	auto& array = GetArrayReference(stack.back());
	array.push_back({ object, false });
}