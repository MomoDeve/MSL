#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "baseExpression.h"

namespace MSL
{
	namespace compiler
	{
		/*
		Function class stores its body and signature information for call
		*/
		class Function
		{
			using LocalScopeTable = std::unordered_map<std::string, size_t>;
			using VariableArray = std::vector<std::string>;
			using ParameterArray = std::vector<std::string>;
			using DependencyTable = std::unordered_map<std::string, size_t>;

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
			/*
			bit-masks of function modifiers
			*/
			enum Modifiers
			{
				_ABSTRACT = 1,
				_STATIC = 2,
				_PUBLIC = 4,
				_CONSTRUCTOR = 8,
				_ENTRY_POINT = 128,
			};

			/*
			array of all functon parameter names
			*/
			ParameterArray params;
			/*
			pointer to the array of expressions as function body
			*/
			std::unique_ptr<ExpressionList> body;
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
			bool isAbstract() const;
			/*
			checks if the function is static
			*/
			bool isStatic() const;
			/*
			checks if the function is public
			*/
			bool isPublic() const;
			/*
			checks if the function is assembly entry-point
			*/
			bool isEntryPoint() const;
			/*
			checks if the function is class constructor
			*/
			bool isConstructor() const;
			/*
			checks if the function has body
			*/
			bool hasBody() const;

			/*
			returns human-read representation of function as string
			*/
			std::string ToString() const;
			/*
			returns constant reference to all variables in function
			*/
			const VariableArray& getVariables() const;

			/*
			creates function with name provided
			*/
			Function(std::string name);
			/*
			function cannot be copied, consider using move instead
			*/
			Function(const Function& function) = delete;
			/*
			moves function contents
			*/
			Function(Function&& function);

			/*
			moves local variable name to VariableArray and adds its name to hash-table
			if variable name already existed in table, it will be overwriten in hash-table, but not deleted from VariableArray
			*/
			void InsertLocal(const std::string& localName);
			/*
			moves outer variable name to VariableArray and adds its name to hash-table
			if variable name already existed in table, it will be overwriten in hash-table, but not deleted from VariableArray
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
			generates bytecode for function body
			this method can be safely called if function does not have body
			*/
			void GenerateBytecode(CodeGenerator& generator) const;

			/*
			generates unique name for function to prevent name collisions in hash-table
			if class already contains overloaded function with the same name
			*/
			static std::string GenerateUniqueName(const std::string& name, size_t paramSize);
		};

		/*
		generates bytecode for each of expressionList expressions
		*/
		void GenerateExpressionListBytecode(const ExpressionList& list, CodeGenerator& code, const Function& function);
	}
}