#pragma once

#include <vector>
#include <memory>

#include "classType.h"
#include "methodType.h"
#include "bigInteger.h"

namespace MSL
{
	namespace VM
	{
		enum class Type : uint8_t
		{
			CLASS,
			INTEGER,
			FLOAT,
			STRING,
			FUNCTION,
		};

		struct BaseObject
		{
			Type type;

			BaseObject(Type type);
			virtual ~BaseObject() = default;
		};

		struct Function : BaseObject
		{
			size_t offset;
			const MethodType& type;

			Function(const MethodType& type);
		};

		struct Class : BaseObject
		{
			using MemberArray = std::vector<std::unique_ptr<BaseObject>>;
			const ClassType& type;
			MemberArray members;

			Class(const ClassType& type);
		};

		struct Integer : BaseObject
		{
			momo::BigInteger value;

			Integer(const std::string& value);
		};

		struct Float : BaseObject
		{
			double value;
			
			Float(const std::string& value);
		};

		struct String : BaseObject
		{
			std::string value;

			String(const std::string& value);
		};
	}
}
