#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "class.h"

namespace MSL
{
	namespace compiler
	{
		/*
		Namespace class stores all classes and interfaces declared in source files
		*/
		class Namespace
		{
			using ClassTable = std::unordered_map<std::string, size_t>;
			using ClassArray = std::vector<Class>;
			/*
			string hash-table storing class name as key and index in ClassArray as value
			*/
			ClassTable table;
			/*
			array of all classes (see class discription)
			*/
			ClassArray classes;
			/*
			unique name of namespace. Assembly must not contain more than one namespace with the same name
			*/
			std::string name;
		public:
			/*
			creates an empty namespace with 'name' = "unnamed"
			*/
			Namespace();
			/*
			creates namespace with name provided
			*/
			Namespace(std::string name);
			/*
			namespace cannot be copied, consider using move instead
			*/
			Namespace(const Namespace& other) = delete;
			/*
			moves namespace contents
			*/
			Namespace(Namespace&& other);
			/*
			returns namespace name as string copy
			*/
			std::string getName() const;
			/*
			returns human-read representation of namespace as string
			*/
			std::string toString() const;
			/*
			returns constant reference to all classes in namespace
			*/
			const ClassArray& getMembers() const;
			/*
			checks if namespace already contains class with given name
			*/
			bool ContainsClass(const std::string& className) const;
			/*
			moves class to ClassArray and adds its name to hash-table
			if class name already existed in table, it will be overwriten in hash-table, but not deleted from ClassArray
			*/
			void InsertClass(const std::string& name, Class&& _class);
		};
	}
}