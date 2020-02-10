#pragma once

#include "function.h"
#include "attribute.h"
#include "stringExtensions.h"
#include <unordered_map>

namespace MSL
{
	namespace compiler
	{
		/*
		Class class stores all its attributes and methods declared in source code
		*/
		class Class
		{
			/*
			support struct which is used in MemberTable as value to indicate attributes and methods
			*/
			struct TableIndex
			{
				/*
				if 'isFunction' = true, MethodArray should be used, AttributeArray either
				*/
				bool isFunction;
				/*
				index in AttributeArray or MemberArray (see TableIndex::isFunction)
				*/
				size_t index;
			};
			using MemberTable = std::unordered_map<std::string, TableIndex>;
			using AttributeArray = std::vector<Attribute>;
			using MethodArray = std::vector<Method>;

			/*
			string hash-table storing method / attribute name as key and TableIndex structure as value
			*/
			MemberTable table;
			/*
			array of all attributes (see attribute discription)
			*/
			AttributeArray attributes;
			/*
			array of all methods (see function discription)
			*/
			MethodArray methods;

		public:
			/*
			bit-masks of class modifiers
			*/
			enum Modifiers
			{
				_STATIC = 1,
				_ABSTRACT = 2,
				_PRIVATE = 4,
				_HAS_STATIC_CONSTRUCTOR = 8,
				_SYSTEM = 128
			};

			/*
			unique name of class. Namespace must not contain more than one class with the same name
			*/
			std::string name;
			/*
			modifiers of class. Each modifier can be checked for existance by a special method Class::is[...]()
			*/
			uint8_t modifiers;

			/*
			checks if the class is static
			*/
			bool IsStatic() const;
			/*
			checks if the class is abstract
			*/
			bool IsAbstract() const;
			/*
			checks if the class is internal
			*/
			bool IsPrivate() const;

			/*
			creates class with name provided
			*/
			Class(std::string name);
			/*
			class cannot be copied, consider using move instead
			*/
			Class(const Class& other) = delete;
			/*
			moves class contents
			*/
			Class(Class&& other) = default;
			/*
			moves attribute to MethodArray and adds its name to hash-table
			if method name already existed in table, it will be overwriten in hash-table, but not deleted from ClassArray
			method name is changed to support overloading, but attributes with the same name till must not be inserted
			*/
			void InsertMethod(const std::string& name, Method&& function);
			/*
			moves method to AttributeArray and adds its name to hash-table
			if attribute name already existed in table, it will be overwriten in hash-table, but not deleted from ClassArray
			attribute must not have the same name as any method in the class
			*/
			void InsertAttribute(const std::string& name, Attribute&& attribute);
			/*
			returns constant reference to all attributes in class
			*/
			const AttributeArray& GetAttributes() const;
			/*
			returns constant reference to all methods in class
			*/
			const MethodArray& GetMethods() const;
			/*
			checks if class already contains member with given name
			*/
			bool ContainsMember(const std::string& memberName) const;
		};
	}
}