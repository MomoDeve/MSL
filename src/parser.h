#pragma once

#include "lexer.h"
#include "function.h"
#include "class.h"
#include "stringExtensions.h"
#include "namespace.h"
#include "expressions.h"
#include "assembly.h"

namespace MSL
{
	namespace compiler
	{
		/*
		Parser class is used to transform array of tokens which can be got by Lexer into assembly
		Note that if the parsing was not successful, assembly is not valid and must not be used
		Assembly can be passed to CodeGenerator class to generate bytecode or observed by iterating over namespaces
		*/
		class Parser
		{
			using ModifierList = std::vector<Token::Type>;
			template<typename T> using unique_ptr = std::unique_ptr<T>;
			/*
			size of any constant in source code (integer / float / string) must not be more than constantMaxSize
			*/
			static const size_t constantMaxSize = std::numeric_limits<uint16_t>::max();
			/*
			assembly object which is generated after successful parsing (can be get by Parser::GetAssembly() method)
			*/
			Assembly assembly;
			/*
			pointer to lexer object from which parser peeks tokens
			*/
			Lexer* lexer;
			/*
			stream to which debug / warning / error information is outputted	
			*/
			std::ostream* stream;
			/*
			success variable indicates if the parsing was successful or not
			*/
			bool success;
			/*
			hasEntryPoint variable indicates if the entry-point function was found or not
			*/
			bool hasEntryPoint;
			/*
			bit-field which indicates which messages are outputted into stream provided (see Parser::stream)
			*/
			uint8_t mode;
			/*
			name of entry-point which will be looked in assembly
			*/
			std::string entryPointName = "Main";
			/*
			maxErrorCount indicates how many error messages will be outputted if parser fails to parse tokens
			(this variable is used to reduce stacktrace output or displaying errors in parsing next tokens in expression)
			*/
			const size_t maxErrorCount = 8;
			/*
			currentErrorCount indicates how many error messages have already been outputted to stream
			*/
			size_t currentErrorCount = 0;

            std::stringstream CompileMethodBody(Method& method, AstNodeList* body);

            Method CompileMethod(MethodDeclNode* method);
            
            Attribute CompileAttribute(AttributeDeclNode* attr);

            Class CompileClass(ClassDeclNode* cl);

            Namespace CompileNamespace(NamespaceDeclNode* ns);
			
            void CompileAST(BaseAstNode* AST);
			/*
			generate attribute of class
			also perform validation check of attribute modifiers
			returns true on success, false either
			*/
			bool ProcessAttribute(Namespace& _namespace, Class& classObject, const ModifierList& modifiers);
			/*
			generate method of class / interface
			also perform validation check of method modifiers and parse its body and parameters
			returns true on success, false either
			*/
			bool ProcessMethod(Namespace& _namespace, Class& classObject, const ModifierList& modifiers);
			/*
			writes message contents to the stream provided to Parser
			*/
			void Notify(std::string message);
			/*
			writes error message to the stream provided to Parser if this option is turned on
			displays message no more than Parser::maxErrorCount times
			after calling this function Parser::success = false
			*/
			void Error(std::string message);
			/*
			writes debug message to the stream provided to Parser if this option is turned on
			*/
			void Debug(std::string message);
			/*
			writes warning message to the stream provided to Parser if this option is turned on
			*/
			void Warning(std::string message);
			/*
			writes highlighted line from source code where the error occured
			*/
			void ShowErrorLine();
		public:
			/*
			bit-masks for which say which information parser will print to provided stream
			*/
			struct MODE
			{
				enum mode : uint8_t
				{
					NO_OUTPUT = 0,
					ERROR_ONLY = 1,
					WARNING_ONLY = ERROR_ONLY << 1,
					DEBUG_ONLY = WARNING_ONLY << 1,
					NO_STACKTRACE = (DEBUG_ONLY << 1),
					NO_DEBUG = ERROR_ONLY | WARNING_ONLY,
					FULL_TRACE = DEBUG_ONLY | NO_DEBUG,
				};
			};
			/*
			creates new parser object
			*/
			Parser(Lexer* lexer, std::ostream* errorStream, MODE::mode mode);
			/*
			parses all lexer contents and creates assembly of MSL program
			*/
			bool Parse();
			/*
			moves assembly object out of parser. Parser must not be used after calling this method
			*/
			Assembly&& PullAssembly();

			/*
			return constant reference to assembly object
			*/
			const Assembly& GetAssembly() const;
			/*
			checks if entry-point was defined in assembly
			*/
			bool HasEntryPoint() const;
			/*
			checks if parsing was successful and assembly can be got or pulled
			*/
			bool ParsingSuccess() const;
		};
	}
}