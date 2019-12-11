#include "gc.h"

using ERROR = MSL::VM::VirtualMachine::ERROR;

void Collect(PARAMS)
{
	vm->GetGC().Collect(vm->GetAssembly(), vm->GetCallStack(), vm->GetObjectStack());
	vm->GetObjectStack().push_back(AllocNull(vm->GetGC()));
}

void Disable(PARAMS)
{
	if (vm->GetConfig().execution.safeMode)
	{
		vm->InvokeError(ERROR::INVALID_METHOD_CALL, "GC.Disable() function is disabled is MSL VM safe mode", "Disable");
		return;
	}
	vm->GetConfig().GC.allowCollect = false;
	vm->GetObjectStack().push_back(AllocNull(vm->GetGC()));
}

void Enable(PARAMS)
{
	if (vm->GetConfig().execution.safeMode)
	{
		vm->InvokeError(ERROR::INVALID_METHOD_CALL, "GC.Enable() function is disabled is MSL VM safe mode", "Enable");
		return;
	}
	vm->GetConfig().GC.allowCollect = true;
	vm->GetObjectStack().push_back(AllocNull(vm->GetGC()));
}

void ReleaseMemory(PARAMS)
{
	vm->GetGC().ReleaseFreeMemory();
	vm->GetObjectStack().push_back(AllocNull(vm->GetGC()));
}

void SetMinimalMemory(PARAMS)
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
	vm->GetObjectStack().push_back(AllocNull(vm->GetGC()));
}

void SetMaximalMemory(PARAMS)
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
	vm->GetObjectStack().push_back(AllocNull(vm->GetGC()));
}

void SetLogPermissions(PARAMS)
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

	vm->GetObjectStack().push_back(AllocNull(vm->GetGC()));
}