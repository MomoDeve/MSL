#include "msl_types.h"

namespace MSL
{
	namespace VM
	{

		ClassWrapper* AllocClassWrapper(GarbageCollector& GC, const ClassType* _class)
		{
			return _class->wrapper;
		}

		ClassObject* AllocClassObject(GarbageCollector& GC, const ClassType* _class)
		{
			if (!_class->staticConstructorCalled && _class->hasStaticConstructor())
			{
				return nullptr; // static constructor must be called before
			}
			ClassObject* object = GC.classObjAlloc.Alloc(_class);
			object->attributes.reserve(_class->objectAttributes.size());
			for (const auto& attr : _class->objectAttributes)
			{
				AttributeObject* objectAttr = GC.attributeAlloc.Alloc(&attr.second);
				objectAttr->object = AllocNull(GC);
				object->attributes[attr.second.name] = objectAttr;
			}
			return object;
		}

		NamespaceWrapper* AllocNamespaceWrapper(GarbageCollector& GC, const NamespaceType* _namespace)
		{
			return _namespace->wrapper;
		}

		LocalObject* AllocLocal(GarbageCollector& GC, const std::string& localName, Local& local)
		{
			return GC.localObjAlloc.Alloc(local, localName);
		}

		UnknownObject* AllocUnknown(GarbageCollector& GC, const std::string* value)
		{
			return GC.unknownObjAlloc.Alloc(value);
		}

		NullObject* AllocNull(GarbageCollector& GC)
		{
			return &GC.nullObject;
		}

		TrueObject* AllocTrue(GarbageCollector& GC)
		{
			return &GC.trueObject;
		}

		FalseObject* AllocFalse(GarbageCollector& GC)
		{
			return &GC.falseObject;
		}

		ArrayObject* AllocArray(GarbageCollector& GC, size_t size)
		{
			ArrayObject* array = GC.arrayAlloc.Alloc(size);
			for (size_t i = 0; i < size; i++)
			{
				array->array[i].object = AllocNull(GC);
				array->array[i].isElement = true;
			}
			return array;
		}

		StringObject* AllocString(GarbageCollector& GC, const std::string& value)
		{
			return GC.stringAlloc.Alloc(value);
		}

		IntegerObject* AllocInteger(GarbageCollector& GC, const std::string& value)
		{
			return GC.integerAlloc.Alloc(value);
		}

		IntegerObject* AllocInteger(GarbageCollector& GC, size_t value)
		{
			return GC.integerAlloc.Alloc((unsigned long long)value);
		}

		IntegerObject* AllocInteger(GarbageCollector& GC, const IntegerObject::InnerType& value)
		{
			return GC.integerAlloc.Alloc(value);
		}

		FloatObject* AllocFloat(GarbageCollector& GC, const std::string& value)
		{
			try
			{
				return GC.floatAlloc.Alloc(std::stod(value));
			}
			catch (std::invalid_argument& e) // stod throws exception if value is not parsed
			{
				e;
				return GC.floatAlloc.Alloc(0.0);
			}
		}
	}
}

bool MSL::utils::AssertType(MSL::VM::VirtualMachine* vm, MSL::VM::BaseObject* object, MSL::VM::Type type)
{
	using ERROR = MSL::VM::VirtualMachine::ERROR;

	if (object->type == type) return true;

	vm->InvokeError(
		ERROR::INVALID_TYPE,
		"object with type " + MSL::VM::ToString(object->type) + " incopatible with expected type " + MSL::VM::ToString(type),
		object->ToString()
	);
	return false;
}

#undef TRUE
#undef FALSE

MSL::VM::BaseObject* MSL::utils::GetUnderlyingObject(MSL::VM::BaseObject* object)
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
		return static_cast<MSL::VM::LocalObject*>(object)->ref.object;
	case MSL::VM::Type::ATTRIBUTE:
		return static_cast<MSL::VM::AttributeObject*>(object)->object;
	default:
		return nullptr; // hits only if error occured
	}
}

const std::string* MSL::utils::GetObjectName(const MSL::VM::BaseObject* object)
{
	switch (object->type)
	{
	case MSL::VM::Type::UNKNOWN:
		return static_cast<const MSL::VM::UnknownObject*>(object)->ref;
	case MSL::VM::Type::CLASS:
		return &static_cast<const MSL::VM::ClassWrapper*>(object)->type->name;
	case MSL::VM::Type::NAMESPACE:
		return &static_cast<const MSL::VM::NamespaceWrapper*>(object)->type->name;
	case MSL::VM::Type::ATTRIBUTE:
		return &static_cast<const MSL::VM::AttributeObject*>(object)->type->name;
	case MSL::VM::Type::LOCAL:
		return &static_cast<const MSL::VM::LocalObject*>(object)->name;
	case MSL::VM::Type::CLASS_OBJECT:
		return &static_cast<const MSL::VM::ClassObject*>(object)->type->name;
	default:
		return nullptr;
	}
}