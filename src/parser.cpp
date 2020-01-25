#include "parser.h"
using namespace MSL::utils;

namespace MSL
{
	namespace compiler
	{
		#define THROW(message) Error(message); return false;
		#define TO_BASE(derived_ptr) unique_ptr<BaseExpression>(derived_ptr.release())

		Parser::Parser(Lexer* lexer, std::ostream* errorStream, Parser::MODE::mode mode)
			: lexer(lexer), stream(errorStream), mode(mode), hasEntryPoint(false), success(true) { }

		bool Parser::Parse()
		{
			while (!lexer->End())
			{
				Token::Type tokenType = lexer->Peek().type;
				if (tokenType == Token::Type::ERROR)
				{
					Error("unresolved symbol, aborting");
					success = false;
					return success;
				}
				else if (tokenType & Token::Type::VALUE_TYPE)
				{
					if (lexer->Peek().value.size() > constantMaxSize)
					{
						Error("constants cannot be more than " + std::to_string(constantMaxSize) + " characters long");
					}
				}
				lexer->Next();
			}
			lexer->ToBegin();
			GenerateAssembly();
			lexer->ToBegin();
			return success;
		}

		Assembly&& Parser::PullAssembly()
		{
			return std::move(assembly);
		}

		bool Parser::GenerateAssembly()
		{
			while (!lexer->End())
			{
				if (lexer->Peek().type == Token::Type::NAMESPACE)
				{
					lexer->Next(); // skipping `namespace` -> [namespace name]
					if (lexer->Peek().type == Token::Type::OBJECT)
					{
						std::string namespaceName = lexer->Peek().value;
						lexer->Next(); // skipping [namespace name] -> `{`
						if (lexer->Peek().type != Token::Type::BRACE_BRACKET_O)
						{
							THROW("namespace must be opened with `{`: " + namespaceName);
						}
						lexer->Next(); // skipping `{`

						Namespace* namespacePtr = nullptr;
						Namespace namespaceInstance(namespaceName);
						if (assembly.ContainsNamespace(namespaceName))
						{
							namespacePtr = &assembly.GetNamespaceByName(namespaceName);
						}
						else
						{
							namespacePtr = &namespaceInstance;
						}
						Namespace& _namespace = *namespacePtr;

						if (!GenerateMembers(_namespace))
						{
							THROW("error while generating namespace: " + namespaceName);
						}
						if (_namespace.getMembers().empty())
						{
							lexer->Prev(); // to prev line to show actual line of empty namespace
							Warning(namespaceName + ": namespace declared empty");
							lexer->Next();
						}
						assembly.InsertNamespace(namespaceName, std::move(_namespace));
					}
					else // namespace name type is not Class
					{
						THROW("incorrect namespace name: " + lexer->Peek().value);
					}
				}
				else
				{
					THROW("namespace not found, aborting"); // namespace expected
				}
			}
			return true; // namespaces are parsed successfully
		}

		bool Parser::GenerateMembers(Namespace& _namespace)
		{
			while (lexer->Peek().type != Token::Type::BRACE_BRACKET_C)
			{
				if (lexer->End()) // expected `{`
				{
					THROW("namespace must be closed with `{`: " + _namespace.getName());
				}

				ModifierList modifiers;
				if (!GetModifiers(modifiers, { Token::Type::CLASS, Token::Type::USING }))
				{
					THROW("incorrect namespace member declaration"); // unknown modifier or no `class`
				}
				if (lexer->Peek().type == Token::Type::CLASS)
				{
					Class classObject("unknown");
					if (ProcessClass(_namespace, classObject, modifiers))
					{
						std::string className = classObject.name;
						if (_namespace.ContainsClass(className))
						{
							THROW("multiple class definition: class " + className);
						}
						Debug("namespace: " + _namespace.getName() + ", class added: " + className);
						_namespace.InsertClass(className, std::move(classObject));
					}
					else
					{
						THROW("cannot process class in namespace: " + _namespace.getName());
					}
				}
				else if (lexer->Peek().type == Token::Type::USING)
				{
					if (modifiers.size() > 0)
					{
						THROW("using expression cannot have modifiers");
					}
					if (!AddUsingExpression(_namespace))
					{
						THROW("processing of using expression failed in namespace: " + _namespace.getName());
					}
				}
				else
				{
					THROW("invalid unit declaration");
				}
			}
			lexer->Next(); // skipping `}` of namespace decl
			return true;
		}

		bool Parser::AddUsingExpression(Namespace& _namespace)
		{
			lexer->Next(); // skipping `using` -> `namespace`
			if (lexer->Peek().type == Token::Type::NAMESPACE)
			{
				lexer->Next(); // skipping `namespace` -> [namespace name]
				if (lexer->Peek().type == Token::Type::OBJECT)
				{
					if (_namespace.friendNamespaces.find(lexer->Peek().value) != _namespace.friendNamespaces.end())
					{
						Warning("namespace " + lexer->Peek().value + " is already a friend namespace to " + _namespace.getName());
					}
					_namespace.friendNamespaces.insert(lexer->Peek().value);
					lexer->Next(); // skipping [namespace name] -> `;`
				}
				else
				{
					THROW("`using namespace` does not define namespace name");
				}
			}
			else
			{
				THROW("`namespace` token must be used after `using` token");
			}
			if (lexer->Peek().type != Token::Type::SEMICOLON)
			{
				THROW("`;` expected after using expression");
			}
			lexer->Next(); // skipping `;`
			return true;
		}

		bool Parser::ProcessClass(Namespace& _namespace, Class& classObject, const ModifierList& modifiers)
		{
			if (lexer->Peek().type != Token::Type::CLASS)
			{
				THROW("class expected, found: " + lexer->Peek().value);
			}
			lexer->Next(); // skipping `class` -> [class name]
			const Token className = lexer->Peek();
			if (className.type != Token::Type::OBJECT)
			{
				THROW("incorrect class name: " + className.value);
			}

			if (find(modifiers, Token::Type::PRIVATE))
			{
				THROW("class cannot be `private`: " + className.value);
			}
			if (find(modifiers, Token::Type::CONST))
			{
				THROW("class cannot be `const`: " + className.value);
			}
			if (find(modifiers, Token::Type::PUBLIC) && find(modifiers, Token::Type::INTERNAL))
			{
				THROW("classes cannot be both `internal` and `public`: " + className.value);
			}
			if (!find(modifiers, Token::Type::PUBLIC) && !find(modifiers, Token::Type::INTERNAL))
			{
				Warning(className.value + ": no class access modifier provided, using `public` by default");
			}

			lexer->Next(); // skipping [class name] -> `{`
			if (lexer->Peek().type != Token::Type::BRACE_BRACKET_O)
			{
				THROW("open bracket not found in declaration of class: " + className.value);
			}
			else
			{
				bool isInternal = find(modifiers, Token::Type::INTERNAL);
				bool isStatic = find(modifiers, Token::Type::STATIC);
				bool isAbstract = find(modifiers, Token::Type::ABSTRACT);

				classObject.name = className.value;

				if (isAbstract)
				{
					classObject.modifiers |= Class::Modifiers::_ABSTRACT;
				}
				if (isStatic)
				{
					classObject.modifiers |= Class::Modifiers::_STATIC;
				}
				if (isInternal)
				{
					classObject.modifiers |= Class::Modifiers::_INTERNAL;
				}

				lexer->Next(); // skipping `{` -> [class member]
				while (lexer->Peek().type != Token::Type::BRACE_BRACKET_C)
				{
					if (lexer->End()) // `}` expected
					{
						THROW("closing bracket not found in class " + className.value);
					}

					ModifierList memberModifiers;
					if (!GetModifiers(memberModifiers, { Token::Type::VARIABLE, Token::Type::FUNCTION }))
					{
						THROW("cannot process member in class " + className.value);
					}
					if (lexer->Peek().type == Token::Type::VARIABLE)
					{
						if (!ProcessAttribute(_namespace, classObject, memberModifiers))
						{
							THROW("cannot process variable in class " + className.value);
						}
					}
					else if (lexer->Peek().type == Token::Type::FUNCTION)
					{
						if (!ProcessMethod(_namespace, classObject, memberModifiers))
						{
							THROW("cannot process method in class " + className.value);
						}
					}
				}
				lexer->Next(); // skipping `}` -> [next unit]
			}
			if (classObject.GetAttributes().empty() && classObject.GetMethods().empty())
			{
				Warning(className.value + ": class declared empty");
			}
			std::string constructorName = classObject.name + "_0";
			if (!classObject.ContainsMember(constructorName) && !classObject.IsStatic() && !classObject.IsAbstract())
			{
				Function constructor(constructorName);
				constructor.modifiers = Function::Modifiers::_PUBLIC | Function::Modifiers::_CONSTRUCTOR;
				constructor.body = std::make_unique<ExpressionList>();
				constructor.body->push_back(std::make_unique<ReturnExpression>());
				constructor.name = classObject.name;
				classObject.InsertMethod(constructorName, std::move(constructor));
			}

			return true; // class is parsed successfully
		}

		bool Parser::ProcessAttribute(Namespace& _namespace, Class& classObject, const ModifierList& modifiers)
		{
			if (lexer->Peek().type != Token::Type::VARIABLE)
			{
				THROW("member declaration expected, found: " + lexer->Peek().value);
			}
			lexer->Next(); // skipping `var` -> [variable name]
			if (lexer->Peek().type != Token::Type::OBJECT)
			{
				THROW("invalid variable name: " + lexer->Peek().value);
			}

			const std::string& memberName = lexer->Peek().value;
			if (classObject.ContainsMember(memberName))
			{
				THROW("this member has already been declared: " + memberName);
			}

			bool isPublic = find(modifiers, Token::Type::PUBLIC);
			bool isConst = find(modifiers, Token::Type::CONST);
			bool isStatic = find(modifiers, Token::Type::STATIC);
			if (find(modifiers, Token::Type::PUBLIC) && find(modifiers, Token::Type::PRIVATE))
			{
				THROW("conflicting modifiers: `public` and `private` in variable defenition");
			}
			if (find(modifiers, Token::Type::INTERNAL))
			{
				THROW("variable cannot be `internal`: " + memberName);
			}
			if (find(modifiers, Token::Type::ABSTRACT))
			{
				THROW("variable cannot be `abstract`: " + memberName);
			}
			if (!isPublic && !find(modifiers, Token::Type::PRIVATE))
			{
				Warning(classObject.name + ": no member access modifier provided: " + memberName + ", using `private` by default");
			}

			lexer->Next(); // skipping [variable name] -> `;`
			Attribute member("unknown");

			if (lexer->Peek().type == Token::Type::SEMICOLON)
			{
				member.name = memberName;

				if (isConst)
				{
					member.modifiers |= Attribute::Modifiers::_CONST;
				}
				if (isStatic)
				{
					member.modifiers |= Attribute::Modifiers::_STATIC;
				}
				if (isPublic)
				{
					member.modifiers |= Attribute::Modifiers::_PUBLIC;
				}

				lexer->Next();
			}
			else // `;` expected
			{
				THROW("invalid member declaration: " + memberName);
			}

			Debug(classObject.name + ": variable added: " + memberName);
			classObject.InsertAttribute(memberName, std::move(member));

			return true;
		}

		bool Parser::ProcessMethod(Namespace& _namespace, Class& classObject, const ModifierList& modifiers)
		{
			if (lexer->Peek().type != Token::Type::FUNCTION)
			{
				THROW("method declaration expected, found: " + lexer->Peek().value);
			}
			lexer->Next(); // skipping `function` -> [function name]
			if (lexer->Peek().type != Token::Type::OBJECT)
			{
				THROW("invalid member name: " + lexer->Peek().value);
			}

			const std::string& memberName = lexer->Peek().value;

			bool isPublic = find(modifiers, Token::Type::PUBLIC);
			bool isAbstract = find(modifiers, Token::Type::ABSTRACT);
			bool isStatic = find(modifiers, Token::Type::STATIC);

			if (find(modifiers, Token::Type::PUBLIC) && find(modifiers, Token::Type::PRIVATE))
			{
				THROW("conflicting modifiers: `public` and `private` in method defenition: " + memberName);
			}
			if (find(modifiers, Token::Type::INTERNAL))
			{
				THROW("method cannot be `internal`: " + memberName);
			}
			if (find(modifiers, Token::Type::CONST))
			{
				THROW("method cannot be `const`: " + memberName);
			}
			if (!isPublic && !find(modifiers, Token::Type::PRIVATE))
			{
				Warning(classObject.name + ": no member access modifier provided: " + memberName + ", using `private` by default");
			}

			Function functionObject("unknown");

			if (memberName == entryPointName && !classObject.IsAbstract())
			{
				if (!isStatic && !classObject.IsStatic())
				{
					Warning(classObject.name + ": only static function can be entry-point: " + memberName);
				}
				else if (hasEntryPoint)
				{
					THROW("multiple entry-point function declarations: " + memberName);
				}
			}
			if (memberName == classObject.name && classObject.IsAbstract())
			{
				THROW("abstract class cannot contain constructor");
			}

			if (isAbstract && !classObject.IsAbstract())
			{
				THROW("non-abstract class cannot contain abstract function: " + memberName);
			}
			if (!isAbstract && classObject.IsAbstract())
			{
				Warning(classObject.name + ": non-abstract function in abstract class is automatically `abstract`: " + memberName);
				isAbstract = true;
			}
			if (!isStatic && classObject.IsStatic())
			{
				Warning(classObject.name + ": non-static function in static class is automatically `static`: " + memberName);
				isStatic = true;
			}

			lexer->Next();
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_O)
			{
				THROW("function must be declared with parameters: " + memberName);
			}
			if (!isStatic && memberName != classObject.name)
			{
				functionObject.params.push_back("this");
			}
			functionObject.name = memberName;
			if (!ParseFunctionParamsDecl(functionObject.params))
			{
				THROW("error while parsing function parameter list: " + functionObject.name);
			}

			if (isAbstract)
			{
				functionObject.modifiers |= Function::Modifiers::_ABSTRACT;
			}
			if (isStatic)
			{
				functionObject.modifiers |= Function::Modifiers::_STATIC;
			}
			if (isPublic)
			{
				functionObject.modifiers |= Function::Modifiers::_PUBLIC;
			}
			if (memberName == classObject.name)
			{
				if (!isStatic)
				{
					functionObject.modifiers |= Function::Modifiers::_CONSTRUCTOR;
				}
				else if(functionObject.params.size() == 0)
				{
					classObject.modifiers |= Class::Modifiers::_STATIC_CONSTRUCTOR;
					functionObject.modifiers |= Function::Modifiers::_STATIC_CONSTRUCTOR;
				}
				else
				{
					THROW("static constructor must not have parameters");
				}
			}

			lexer->Next(); // skipping `)` -> `{` 
			if (lexer->Peek().type == Token::Type::BRACE_BRACKET_O)
			{
				auto body = std::make_unique<ExpressionList>(ParseExpressionBlock(functionObject));
				if (body->empty() || dynamic_cast<ReturnExpression*>(body->back().get()) == nullptr) // if no return, return void
				{
					body->push_back(std::make_unique<ReturnExpression>());
				}
				functionObject.body = std::move(body);
			}
			else if (!isAbstract)
			{
				THROW("non-abstract function must have body: " + memberName);
			}
			else if (lexer->Peek().type != Token::Type::SEMICOLON)
			{
				lexer->Prev();
				THROW("incorrect function declaration: " + memberName);
			}
			else
			{
				lexer->Next();
			}

			Debug(classObject.name + ": method added: " + memberName);
			std::string functionInnerName = Function::GenerateUniqueName(memberName, functionObject.params.size()); // unique name for overloading
			if (memberName == classObject.name && functionObject.modifiers & Function::Modifiers::_STATIC)
			{
				functionInnerName += "_static";
			}

			if (isStatic && !isAbstract && (memberName == entryPointName))
			{
				hasEntryPoint = true;
				functionObject.modifiers |= Function::Modifiers::_ENTRY_POINT;
				Debug("entry-point located in class: " + classObject.name);
			}
			if (classObject.ContainsMember(functionInnerName))
			{
				lexer->Prev();
				THROW("this member has already been declared: " + memberName);
			}
			classObject.InsertMethod(functionInnerName, std::move(functionObject));

			return true;
		}

		bool Parser::GetModifiers(ModifierList& modifiers, const std::vector<Token::Type>& stopTokens)
		{
			Token::Type modifier = lexer->Peek().type;
			while (!find(stopTokens, modifier))
			{
				if (lexer->End())
				{
					THROW("incorrect definition, reached end of file");
				}
				if (find(modifiers, modifier))
				{
					THROW("dublicate modifier: " + Token::ToString(modifier));
				}
				if ((modifier & Token::Type::MODIFIER) == 0)
				{
					THROW("invalid modifier: " + Token::ToString(modifier));
				}
				modifiers.push_back(modifier);
				lexer->Next();
				modifier = lexer->Peek().type;
			}
			return true;
		}

		bool Parser::ParseFunctionParamsDecl(std::vector<std::string>& parameters)
		{
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_O)
			{
				THROW("`(` expected before function parameter list");
			}
			do
			{
				lexer->Next();
				const Token token = lexer->Peek();
				if (token.type == Token::Type::ROUND_BRACKET_C)
				{
					lexer->Prev();
					if (lexer->Peek().type == Token::Type::COMMA)
					{
						THROW("empty function parameter");
					}
					lexer->Next();
					break;
				}
				if (lexer->End())
				{
					THROW("no closing bracket for function parameter list");
				}
				else if (lexer->Peek().type != Token::Type::OBJECT)
				{
					THROW("invalid parameter name");
				}

				std::string& paramName = lexer->Peek().value;
				if (std::find(parameters.begin(), parameters.end(), paramName) != parameters.end())
				{
					THROW("function parameter duplicate");
				}
				parameters.push_back(std::move(paramName));

				lexer->Next();
				if (lexer->Peek().type != Token::Type::COMMA && lexer->Peek().type != Token::Type::ROUND_BRACKET_C)
				{
					THROW("function parameters must be separeted with comma");
				}

			} while (lexer->Peek().type != Token::Type::ROUND_BRACKET_C);
			return true; // success
		}

		ExpressionList Parser::ParseExpressionBlock(Function& function)
		{
			ExpressionList block;
			if (lexer->Peek().type != Token::Type::BRACE_BRACKET_O)
			{
				block.push_back(ParseExpression(function));
				if (lexer->Peek().type == Token::Type::SEMICOLON) lexer->Next();
				return block;
			}
			lexer->Next();
			while (!lexer->End())
			{
				if (lexer->Peek().type == Token::Type::BRACE_BRACKET_C)
				{
					lexer->Next();
					return block;
				}
				unique_ptr<BaseExpression> expr = ParseExpression(function);
				if (expr == nullptr)
				{
					Error("parser hit fatal error, aborting...");
					return ExpressionList();
				}
				block.push_back(std::move(expr));
				if (lexer->Peek().type == Token::Type::SEMICOLON)
				{
					lexer->Next(); // skipping `;`
				}
			}
			Error("parser reached EOF");
			return ExpressionList(); // reached EOF
		}

		ExpressionList Parser::ParseFunctionArguments(Function& function)
		{
			ExpressionList args;
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_O)
			{
				Error("`(` expected in call expression");
				return ExpressionList();
			}
			lexer->Next();
			if (lexer->Peek().type == Token::Type::ROUND_BRACKET_C)
			{
				return args; // 0 parameterss
			}
			while (!lexer->End())
			{
				args.push_back(ParseRawExpression(function));
				if (lexer->Peek().type == Token::Type::ROUND_BRACKET_C)
				{
					return args;
				}
				if (lexer->Peek().type != Token::Type::COMMA)
				{
					Error("`,` expected in call expression");
				}
				lexer->Next();
			}
			Error("parser reached EOF");
			return ExpressionList(); // EOF reached
		}

		unique_ptr<BaseExpression> Parser::ParseIndexArgument(Function& function)
		{
			if (lexer->Peek().type != Token::Type::SQUARE_BRACKET_O)
			{
				Error("`[` expected in index expression");
				return nullptr;
			}
			lexer->Next(); // skipping `[` -> [object]
			if (lexer->Peek().type == Token::Type::SQUARE_BRACKET_C)
			{
				Error("index expression cannot be empty");
				return nullptr;
			}
			unique_ptr<BaseExpression> expr = ParseRawExpression(function);
			if (lexer->Peek().type != Token::Type::SQUARE_BRACKET_C)
			{
				Error("`]` exprected in index expression");
			}
			lexer->Next(); // skipping `]`
			return expr;
		}

		unique_ptr<BaseExpression> Parser::ParseExpression(Function& function)
		{
			switch (lexer->Peek().type)
			{
			case Token::Type::CONST: // `const var` or `var` possible
			case Token::Type::VARIABLE:
				return ParseVariableDecl(function);
			case Token::Type::IF:
				return ParseIfExpression(function);
			case Token::Type::FOR:
				return ParseForExpression(function);
			case Token::Type::FOREACH:
				return ParseForeachExpression(function);
			case Token::Type::WHILE:
				return ParseWhileExpression(function);
			case Token::Type::RETURN:
				return ParseReturnExpression(function);
			case Token::Type::TRY:
				return ParseTryExpression(function);
			default:
				return ParseRawExpression(function);
			}
		}

		unique_ptr<BaseExpression> Parser::ParseVariableDecl(Function& function)
		{
			unique_ptr<ObjectDeclareExpression> variable(std::make_unique<ObjectDeclareExpression>());
			variable->isConst = lexer->Peek().type == Token::Type::CONST;
			if (variable->isConst)
			{
				lexer->Next(); // skipping `const` -> `var`
			}

			if (lexer->Peek().type != Token::Type::VARIABLE)
			{
				Error("`var` expected");
			}
			lexer->Next(); // skipping `var` -> [Class]
			if (lexer->Peek().type != Token::Type::OBJECT)
			{
				Error("invalid variable name");
			}
			std::string& objectName = lexer->Peek().value;
			variable->objectName = objectName;
			if (function.ContainsLocal(objectName))
			{
				Error("variable already declared in function scope (function " + function.name + ')');
			}
			else if (function.ContainsDependency(objectName))
			{
				Warning("variable with same name (" + objectName + ") was accessed before its declaration");
			}
			else
			{
				function.InsertLocal(objectName);
			}
			lexer->Next(); // skipping [Class] -> `;` or `=`
			if (lexer->Peek().type == Token::Type::ASSIGN_OP)
			{
				lexer->Next();
				variable->assignment = ParseRawExpression(function);
			}
			else if (lexer->Peek().type != Token::Type::SEMICOLON)
			{
				Error("`;` or `=` expected after variable declaration");
			}
			return TO_BASE(variable);
		}

		unique_ptr<BaseExpression> Parser::ParseLambdaExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::LAMBDA)
			{
				Error("`lambda` expected");
			}
			else
			{
				Error("lambda currently not supported, please use default methods"); // lambda is not working now
			}

			// this code will never be executed
			unique_ptr<LambdaExpression> lambdaExpr(std::make_unique<LambdaExpression>());
			lexer->Next(); // skipping `lambda` -> `(`
			if (!ParseFunctionParamsDecl(lambdaExpr->params))
			{
				Error("could not parse parameter list in lambda definition");
			}
			lexer->Next(); // skipping `)` -> `{`
			lambdaExpr->body = ParseExpressionBlock(function);
			return TO_BASE(lambdaExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseTryExpression(Function& function)
		{
			// try { ... }
			if (lexer->Peek().type != Token::Type::TRY)
			{
				Error("`try` expected");
			}
			auto tryExpr = std::make_unique<TryExpression>();
			lexer->Next(); // skipping `try` -> `{`
			tryExpr->tryBody = ParseExpressionBlock(function);
			// catch { ... }
			if (lexer->Peek().type == Token::Type::CATCH)
			{
				lexer->Next(); // skipping `catch` -> `(`
				// catch (variable) { ... }
				if (lexer->Peek().type == Token::Type::ROUND_BRACKET_O)
				{
					lexer->Next(); // skipping `(` -> [variable]
					if (lexer->Peek().type != Token::Type::OBJECT)
						Error("object name expected before catch block");
					tryExpr->variable = lexer->Peek().value;
					function.InsertDependency(tryExpr->variable);
					function.InsertDependency("System");
					function.InsertDependency("Exception");
					function.InsertDependency("Instance_0");

					lexer->Next(); // skipping [variable] -> `)`
					if (lexer->Peek().type != Token::Type::ROUND_BRACKET_C)
						Error("`)` expected before `catch` keyword");
					lexer->Next(); // skipping `)` -> `catch` 
				}
				// catch body
				tryExpr->catchBody = ParseExpressionBlock(function);
			}
			return TO_BASE(tryExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseForExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::FOR)
			{
				Error("`for` expected");
			}
			unique_ptr<ForExpression> forExpr(std::make_unique<ForExpression>());
			lexer->Next(); // skipping `for` -> `(`
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_O)
			{
				Error("`(` expected in for loop expression");
			}
			lexer->Next(); // skipping `(`
			if (lexer->Peek().type == Token::Type::VARIABLE) // for(var i = ...
			{
				lexer->Next();
				std::string& objectName = lexer->Peek().value;
				if (function.ContainsLocal(objectName))
				{
					function.RemoveLocal(objectName);
				}
				lexer->Prev();
				forExpr->init = ParseVariableDecl(function);
			}
			else if (lexer->Peek().type != Token::Type::SEMICOLON) // has init expression
			{
				forExpr->init = ParseRawExpression(function);
			}
			lexer->Next(); // skipping `;`
			if (lexer->Peek().type == Token::Type::SEMICOLON) // no predicate
			{
				unique_ptr<ObjectExpression> trueStatement(std::make_unique<ObjectExpression>());
				trueStatement->object = Token(Token::Type::TRUE_CONSTANT, "true");
				function.InsertDependency(trueStatement->object.value);
				forExpr->predicate = TO_BASE(trueStatement);
			}
			else
			{
				forExpr->predicate = ParseRawExpression(function);
			}
			lexer->Next(); // skipping `;`
			if (lexer->Peek().type == Token::Type::ROUND_BRACKET_C)
			{
				forExpr->iteration = nullptr;
			}
			else
			{
				forExpr->iteration = ParseRawExpression(function);
			}
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_C)
			{
				Error("`)` expected in the end of for loop expression");
			}
			lexer->Next(); // skipping `)` -> `{`
			forExpr->body = ParseExpressionBlock(function);
			return TO_BASE(forExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseForeachExpression(Function& function)
		{
			unique_ptr<ForeachExpression> foreachExpr(std::make_unique<ForeachExpression>());
			function.InsertDependency("Begin_0");
			function.InsertDependency("Next_1");
			function.InsertDependency("GetByIter_1");
			function.InsertDependency("End_0");

			if (lexer->Peek().type != Token::Type::FOREACH)
			{
				Error("`foreach` expected");
			}

			lexer->Next(); // skipping `foreach` -> `(`
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_O)
			{
				Error("`(` expected in foreach statement");
			}

			lexer->Next(); // skipping `(` -> `var`
			if (lexer->Peek().type != Token::Type::VARIABLE)
			{
				Error("iterator declaration expected in foreach statement");
			}

			lexer->Next(); // skipping `var` -> [Class name]
			if (lexer->Peek().type != Token::Type::OBJECT)
			{
				Error("invalid iterator name");
			}
			std::string& iteratorName = lexer->Peek().value;
			if (!function.ContainsLocal(iteratorName))
			{
				function.InsertLocal(iteratorName);
			}
			foreachExpr->iterator = iteratorName;

			lexer->Next(); // skipping [Class name] -> `in`
			if (lexer->Peek().type != Token::Type::IN)
			{
				Error("`in` expected in foreach statement after iterator declaration");
			}

			lexer->Next(); // skipping `in` -> [container]
			unique_ptr<ObjectDeclareExpression> container(std::make_unique<ObjectDeclareExpression>());
			container->objectName = "__CONTAINER_COMPILE_" + std::to_string(function.labelInnerId);
			function.InsertDependency(container->objectName);
			container->assignment = ParseRawExpression(function);
			foreachExpr->container = TO_BASE(container);
			foreachExpr->iteratorIndex = "__ITERATOR_COMPILE_" + std::to_string(function.labelInnerId++);
			function.InsertDependency(foreachExpr->iteratorIndex);

			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_C)
			{
				Error("`)` expected in foreach statement");
			}
			lexer->Next();
			foreachExpr->body = ParseExpressionBlock(function);
			return TO_BASE(foreachExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseWhileExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::WHILE)
			{
				Error("`while` expected");
			}
			lexer->Next(); // skipping `while` -> `(`
			unique_ptr<WhileExpression> whileExpr(std::make_unique<WhileExpression>());
			whileExpr->predicate = ParseStatementInBrackets(function);
			lexer->Next(); // skipping `)` -> `{`
			whileExpr->body = ParseExpressionBlock(function);
			return TO_BASE(whileExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseIfExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::IF)
			{
				Error("`if` expected");
			}
			unique_ptr<IfExpression> ifExpr(std::make_unique<IfExpression>());

			lexer->Next(); // skipping `if` -> `(`
			ifExpr->ifStatements.push_back(ParseStatementInBrackets(function));
			lexer->Next(); // skipping `)` -> `{`
			ifExpr->bodies.push_back(ParseExpressionBlock(function));

			while (lexer->Peek().type == Token::Type::ELIF)
			{
				lexer->Next(); // skipping `elif` -> `(`
				ifExpr->ifStatements.push_back(ParseStatementInBrackets(function));
				lexer->Next(); // skipping `)` -> `{`
				ifExpr->bodies.push_back(ParseExpressionBlock(function));
			}

			if (lexer->Peek().type == Token::Type::ELSE)
			{
				lexer->Next(); // skipping `else` -> `{`
				ifExpr->bodies.push_back(ParseExpressionBlock(function));
			}
			return TO_BASE(ifExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseReturnExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::RETURN)
			{
				Error("`return` expected");
			}
			unique_ptr<ReturnExpression> returnExpr(std::make_unique<ReturnExpression>());
			lexer->Next(); // skipping `return` -> [expr]
			if (lexer->Peek().type == Token::Type::SEMICOLON)
			{
				returnExpr->returnValue = nullptr;
			}
			else
			{
				returnExpr->returnValue = ParseRawExpression(function);
			}
			return TO_BASE(returnExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseStatementInBrackets(Function& function)
		{
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_O)
			{
				Error("`(` expected in bracket statement");
			}
			lexer->Next(); // skipping `(` -> [expr]
			unique_ptr<BaseExpression> expr = ParseRawExpression(function);
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_C)
			{
				Error("`)` expected in the end of bracket statement expression");
			}
			return expr;
		}

		unique_ptr<BaseExpression> Parser::ParseNextVariable(Function& function, bool catchIndex)
		{
			if (lexer->Peek().type == Token::Type::ROUND_BRACKET_O)
			{
				unique_ptr<BaseExpression> statement = ParseStatementInBrackets(function);
				lexer->Next(); // skipping `)`
				return statement;
			}
			if (lexer->Peek().type & Token::Type::UNARY_OPERAND)
			{
				auto unaryExpr = std::make_unique<UnaryExpression>();
				unaryExpr->expressionType = lexer->Peek().type;
				lexer->Next(); // skipping [unary op]
				if (lexer->Peek().type & Token::Type::UNARY_OPERAND)
				{
					unaryExpr->expression = ParseNextVariable(function);
					return TO_BASE(unaryExpr);
				}
				if (lexer->Peek().type == Token::Type::ROUND_BRACKET_O || (lexer->Peek().type & Token::Type::VALUE_TYPE))
				{
					unique_ptr<BaseExpression> obj = ParseNextVariable(function);
					uint32_t currentPriority = unaryExpr->expressionType & Token::Type::PRIORITY;
					uint32_t nextPriority = lexer->Peek().type & Token::Type::PRIORITY;
					if (currentPriority < nextPriority)
					{
						unaryExpr->expression = ParseRawExpression(function, TO_BASE(obj));
					}
					else
					{
						unaryExpr->expression = std::move(obj);
					}
					return TO_BASE(unaryExpr);
				}
				else
				{
					THROW("another unary operand or value expected after unary operand");
				}
			}
			if (lexer->Peek().type & Token::Type::VALUE_TYPE)
			{

				Token& object = lexer->Peek();
				lexer->Next(); // skipping [var] -> `(` / `[`
				if (lexer->Peek().type == Token::Type::ROUND_BRACKET_O)
				{
					unique_ptr<CallExpression> callExpr(std::make_unique<CallExpression>());
					callExpr->parameters = ParseFunctionArguments(function);
					callExpr->functionName = Function::GenerateUniqueName(object.value, callExpr->parameters.size());
					function.InsertDependency(callExpr->functionName);
					lexer->Next(); // skipping `)`
					return TO_BASE(callExpr);
				} // catchIndex  must be fixed TO DO
				else if (catchIndex && lexer->Peek().type == Token::Type::SQUARE_BRACKET_O)
				{
					function.InsertDependency(object.value);
					auto objectExpr = std::make_unique<ObjectExpression>();
					objectExpr->object = object;
					auto expr = TO_BASE(objectExpr);

					auto indexExpr = std::make_unique<IndexExpression>();
					while (lexer->Peek().type == Token::Type::SQUARE_BRACKET_O)
					{
						indexExpr->caller = std::move(expr);
						indexExpr->parameter = ParseIndexArgument(function);
						expr = TO_BASE(indexExpr);
						indexExpr = std::make_unique<IndexExpression>();
					}
					return std::move(expr);
				}
				else
				{
					unique_ptr<ObjectExpression> ObjectExpr(std::make_unique<ObjectExpression>());
					if (object.type == Token::Type::THIS && (function.isStatic()))
					{
						Error("`this` reference inside a static function: " + function.name);
					}
					ObjectExpr->object = object;
					function.InsertDependency(object.value);
					return TO_BASE(ObjectExpr);
				}
			}
			Error("value or round brackets expression expected");
			return nullptr;
		}

		unique_ptr<BaseExpression> Parser::ParseRawExpression(Function& function, unique_ptr<BaseExpression> leftBranch, uint32_t returnPriority)
		{
			const Token& firstToken = lexer->Peek();
			if (firstToken.type == Token::Type::LAMBDA) // lambda cannot be used, so going into this block will cause error while parsing
			{
				unique_ptr<BaseExpression> expr = ParseLambdaExpression(function);
				return ParseRawExpression(function, std::move(expr));
			}
			if (firstToken.type & Token::Type::UNARY_OPERAND)
			{
				return ParseRawExpression(function, TO_BASE(ParseNextVariable(function)));
			}
			if (firstToken.type & Token::Type::BINARY_OPERAND) // left branch should not be `nullptr`
			{
				unique_ptr<BinaryExpression> binExpr(std::make_unique<BinaryExpression>());
				if (leftBranch == nullptr)
				{
					THROW("value type before binary operand expected");
				}
				binExpr->left = std::move(leftBranch);
				binExpr->expressionType = firstToken.type;
				uint32_t currentPriority = firstToken.type & Token::PRIORITY;
				lexer->Next(); // skipping `BINARY_OP`
				unique_ptr<BaseExpression> rightExpr = ParseNextVariable(function, firstToken.type != Token::Type::DOT);
				// TO DO fix here
				if (rightExpr == nullptr)
				{
					THROW("expression was not parsed, aborting...");
				}

				uint32_t nextPriority = lexer->Peek().type & Token::PRIORITY;
				if (currentPriority < nextPriority)
				{
					bool fastReturn =
						binExpr->expressionType == Token::Type::SUB_OP  ||
						binExpr->expressionType == Token::Type::SUM_OP  ||
						binExpr->expressionType == Token::Type::MULT_OP ||
						binExpr->expressionType == Token::Type::DIV_OP  ||
						binExpr->expressionType == Token::Type::MOD_OP  ||
						binExpr->expressionType == Token::Type::DOT;
					if (!fastReturn) currentPriority = 0;
					binExpr->right = ParseRawExpression(function, std::move(rightExpr), currentPriority);
					return ParseRawExpression(function, TO_BASE(binExpr));
				}
				else if (currentPriority == nextPriority)
				{
					// need to specify parent of call expression to avoid inserting
					// Class.Function() -> Class.[this].Function() because class was omitted
					if (rightExpr->type == ExpressionType::CALL)
					{
						CallExpression* call = reinterpret_cast<CallExpression*>(rightExpr.get());
						if (firstToken.type == Token::Type::DOT)
							call->hasParent = true;
					}
					binExpr->right = std::move(rightExpr);
					return ParseRawExpression(function, TO_BASE(binExpr));
				}
				else
				{
					// need to specify parent of call expression to avoid inserting
					// Class.Function() -> Class.[this].Function() because class was omitted
					if (rightExpr->type == ExpressionType::CALL)
					{
						CallExpression* call = reinterpret_cast<CallExpression*>(rightExpr.get());
						if (firstToken.type == Token::Type::DOT)
							call->hasParent = true;
					}
					nextPriority = lexer->Peek().type & Token::Type::PRIORITY;
					if (lexer->Peek().type == Token::Type::SQUARE_BRACKET_O) 
						nextPriority = Token::Type::PRIORITY + 1; // Index operator force binExpr to be forwarded

					binExpr->right = std::move(rightExpr);
					if(returnPriority >= nextPriority) return TO_BASE(binExpr);
					else return ParseRawExpression(function, TO_BASE(binExpr));
				}
			}
			if ((firstToken.type & Token::Type::VALUE_TYPE) || firstToken.type == Token::Type::ROUND_BRACKET_O)
			{
				if (leftBranch != nullptr)
				{
					Error("binary operand or expression end expected");
				}
				unique_ptr<BaseExpression> valueExpr = ParseNextVariable(function);
				return ParseRawExpression(function, std::move(valueExpr));
			}
			if (firstToken.type == Token::Type::BRACE_BRACKET_C)
			{
				lexer->Prev();
				Error("Error while parsing: bracket or semicolon probably missing");
				lexer->Next();
			}
			if (firstToken.type == Token::Type::SQUARE_BRACKET_O)
			{
				auto expr = std::make_unique<IndexExpression>();
				expr->caller = std::move(leftBranch);
				expr->parameter = ParseIndexArgument(function);
				return ParseRawExpression(function, TO_BASE(expr));
			}
			if (leftBranch == nullptr)
			{
				lexer->Prev();
				Error("binary operand expected, but not found");
				lexer->Next();

				lexer->Next();
				return nullptr;
			}
			return leftBranch;
		}

		void Parser::Notify(std::string message)
		{
			writeToStream(stream, message);
		}

		void Parser::Error(std::string message)
		{
			if (currentErrorCount < maxErrorCount)
			{
				if (mode & (MODE::ERROR_ONLY | MODE::NO_STACKTRACE))
				{
					message = "[ERROR]: " + message;
					Notify(message);
					Notify("\t on token: `" + lexer->Peek().value + "`");
				}
				if (success) ShowErrorLine(); // happens only on first call
				if (mode & MODE::NO_STACKTRACE)
				{
					mode &= MODE::NO_OUTPUT;
				}
				currentErrorCount++;
			}
			success = false;
		}

		void Parser::Debug(std::string message)
		{
			if (mode & MODE::DEBUG_ONLY)
			{
				message = "[DEBUG]: " + message;
				Notify(message);
			}
		}

		void Parser::Warning(std::string message)
		{
			if (mode & MODE::WARNING_ONLY)
			{
				message = "[WARNING]: " + message;
				Notify(message);
				ShowErrorLine();
			}
		}

		void Parser::ShowErrorLine()
		{
			int iterPos = lexer->GetIteratorPos();
			std::string lineCount = "[" + std::to_string(lexer->GetLineCount() + 1) + "] ";
			std::string filler(lexer->ToPrevLine().size() + lineCount.size() - 1, '~');
			filler += '^';
			std::string fileLine = lexer->ToNextLine();
			fileLine = beautify(fileLine);
			Notify(lineCount + fileLine + '\n' + filler);
			lexer->SetIteratorPos(iterPos);
		}

		const Assembly& Parser::GetAssembly() const
		{
			return assembly;
		}

		bool Parser::HasEntryPoint() const
		{
			return hasEntryPoint;
		}

		bool Parser::ParsingSuccess() const
		{
			return success;
		}

		#undef TO_BASE
		#undef THROW
	}
}