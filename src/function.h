#pragma once

#include "expressions.h"
#include <unordered_map>
#include <sstream>

namespace MSL
{
	namespace compiler
	{
		/*
		Function class stores its body and signature information for call
		*/
		class Method
		{
			using LocalScopeTable = std::unordered_map<std::string, size_t>;
			using VariableArray = std::vector<std::string>;
			using ParameterArray = std::vector<std::string>;
			using DependencyTable = std::unordered_map<std::string, size_t>;
            using BytecodeArray = std::stringstream;
            using ErrorList = std::vector<std::string>;
			/*
			string hash-table storing local variable name as key and index in VariableArray as value
			*/
			LocalScopeTable locals;
			/*
			string hash-table storing all outer variable name as key and index in VariableArray as value
			*/
			DependencyTable dependencies;
			/*
			array of all variable names int function body
			*/
			VariableArray variables;
		public:
            ErrorList errors;

			/*
			bit-masks of function modifiers
			*/
			enum Modifiers
			{
				_ABSTRACT = 1,
				_STATIC = 2,
				_PUBLIC = 4,
				_CONSTRUCTOR = 8,
				_STATIC_CONSTRUCTOR = 16,
				_ENTRY_POINT = 128,
			};

			/*
			array of all functon parameter names
			*/
			ParameterArray params;
			/*
			method body as bytecode array
			*/
			BytecodeArray bytecode;
			/*
			unique name of function. Class must not contain more than one function with the same name and parameter count
			*/
			std::string name;
			/*
			temporary variable which is used by Function::GenerateBytecode() method to set jump-labels in loops of if-clauses
			*/
			mutable uint16_t labelInnerId;
			/*
			modifiers of function. Each modifier can be checked for existance by a special method Class::is[...]()
			*/
			uint8_t modifiers;

			/*
			checks if the function is abstract
			*/
			bool IsAbstract() const;
			/*
			checks if the function is static
			*/
			bool IsStatic() const;
			/*
			checks if the function is public
			*/
			bool isPublic() const;
			/*
			checks if the function is assembly entry-point
			*/
			bool IsEntryPoint() const;
			/*
			checks if the function is class constructor
			*/
			bool isConstructor() const;

			/*
			returns human-read representation of function as string
			*/
			std::string ToString() const;
			/*
			returns constant reference to all variables in function
			*/
			const VariableArray& GetVariables() const;

			/*
			creates function with name provided
			*/
			Method(std::string name);
			/*
			function cannot be copied, consider using move instead
			*/
			Method(const Method& function) = delete;
			/*
			moves function contents
			*/
			Method(Method&& function) = default;

			/*
			moves local variable name to VariableArray and adds its name to hash-table
			if variable name already existed in table, it will be overwriten in hash-table, but not deleted from VariableArray
			*/
			void InsertLocal(const std::string& localName);
			/*
			removes local entry from local table. local still stands in dependency array.
			calling this method without checking for local with ContainsLocal() method is prohibited
			*/
			void RemoveLocal(const std::string& localName);
			/*
			moves outer variable name to VariableArray and adds its name to hash-table
			if variable name already existed in table, no changes are applied
			*/
			void InsertDependency(const std::string& dependencyName);

			/*
			checks if class already contains local variable with given name
			*/
			bool ContainsLocal(const std::string& localName) const;
			/*
			checks if class already contains outer variable with given name
			*/
			bool ContainsDependency(const std::string& dependenctyName) const;
			/*
			returns index in VariableArray by searching variable name in LocalScopeTable or in DependencyTable
			GetHash() must only be used if the variable already exists in VariableArray
			*/
			size_t GetHash(const std::string& variableName) const;
            /*
            returns method body as bytecode array
            */
            const BytecodeArray& GetBytecode() const;


            void PushError(const std::string& error);
			/*
			generates unique name for function to prevent name collisions in hash-table
			if class already contains overloaded function with the same name
			*/
			static std::string GenerateUniqueName(const std::string& name, size_t paramSize);
		};
	}
}