#include "parser.h"

namespace MSL
{
	namespace compiler
	{
		#define THROW(message) Error(message); return false;
		#define TO_BASE(derived_ptr) unique_ptr<BaseExpression>(derived_ptr.release())

		Parser::Parser(Lexer* lexer, std::ostream* errorStream, Mode mode)
			: lexer(lexer), stream(errorStream), mode(mode), hasEntryPoint(false) { }

		bool Parser::Parse()
		{
			success = true;
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
			if (!hasEntryPoint)
			{
				success = false;
				Notify("[Error]: Entry point must be defined: no Main function found");
			}
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
						if (assembly.ContainsNamespace(namespaceName))
						{
							THROW("namespace already declared: " + namespaceName);
						}
						lexer->Next(); // skipping [namespace name] -> `{`
						if (lexer->Peek().type != Token::Type::BRACE_BRACKET_O)
						{
							THROW("namespace must be opened with `{`: " + namespaceName);
						}
						lexer->Next(); // skipping `{`

						Namespace _namespace(namespaceName);

						if (!GenerateMembers(_namespace))
						{
							THROW("Error while generating namespace: " + namespaceName);
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
					THROW("Namespace must be closed with `{`: " + _namespace.getName());
				}

				ModifierList modifiers;
				if (!GetModifiers(modifiers, { Token::Type::CLASS, Token::Type::INTERFACE }))
				{
					THROW("incorrect namespace member declaration"); // unknown modifier or no `class` / `interface`
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
				else if (lexer->Peek().type == Token::Type::INTERFACE)
				{
					Class interfaceObject("unknown");
					if (ProcessInterface(_namespace, interfaceObject, modifiers))
					{
						std::string interfaceName = interfaceObject.name;
						if (_namespace.ContainsClass(interfaceName))
						{
							THROW("multiple class definition: interface " + interfaceName);
						}
						Debug("namespace: " + _namespace.getName() + ", interface added: " + interfaceName);
						_namespace.InsertClass(interfaceName, std::move(interfaceObject));
					}
					else
					{
						THROW("cannot process interface in namespace: " + _namespace.getName());
					}
				}
			}
			lexer->Next(); // skipping `}` of namespace decl
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
			if (!classObject.ContainsMember(classObject.name) && !classObject.IsStatic() && !classObject.IsAbstract())
			{
				Function constructor(classObject.name);
				constructor.modifiers = Function::Modifiers::_PUBLIC | Function::Modifiers::_CONSTRUCTOR;
				constructor.body = unique_ptr<ExpressionList>(new ExpressionList());
				constructor.body->push_back(std::unique_ptr<BaseExpression>(new ReturnExpression()));
				classObject.InsertMethod(classObject.name, std::move(constructor));
			}

			return true; // class is parsed successfully
		}

		bool Parser::ProcessInterface(Namespace& _namespace, Class& interfaceObject, const ModifierList& modifiers)
		{
			if (lexer->Peek().type != Token::Type::INTERFACE)
			{
				THROW("interface declaration expected, found: " + lexer->Peek().value);
			}
			lexer->Next(); // skipping `interface` -> [interface name]
			Token interfaceName = lexer->Peek();
			if (interfaceName.type != Token::Type::OBJECT)
			{
				THROW("invalid interface name: " + interfaceName.value);
			}
			interfaceObject.name = interfaceName.value;
			bool isPublic = find(modifiers, Token::Type::PUBLIC);

			if (find(modifiers, Token::Type::ABSTRACT))
			{
				THROW("interfaces cannot be `abstract`: " + interfaceName.value);
			}
			if (find(modifiers, Token::Type::CONST))
			{
				THROW("interfaces cannot be `const`: " + interfaceName.value);
			}
			if (find(modifiers, Token::Type::INTERNAL) && find(modifiers, Token::Type::PUBLIC))
			{
				THROW("interfaces cannot be both `public` and `internal`: " + interfaceName.value);
			}
			if (!find(modifiers, Token::Type::INTERNAL) && !find(modifiers, Token::Type::PUBLIC))
			{
				Warning(interfaceName.value + ": no interface access modifiers provided, using `public` by default");
				isPublic = true; // set default access value
			}
			if (find(modifiers, Token::Type::STATIC))
			{
				Warning("interfaces are always `static`, ignored: " + interfaceName.value);
			}

			if (!isPublic)
			{
				interfaceObject.modifiers |= Class::Modifiers::_INTERNAL;
			}
			interfaceObject.modifiers |= (Class::Modifiers::_CONST | Class::Modifiers::_ABSTRACT | Class::Modifiers::_INTERFACE);

			lexer->Next(); // skipping [interface name] -> `{`
			if (lexer->Peek().type != Token::Type::BRACE_BRACKET_O)
			{
				THROW("open bracket not found in declaration of interface: " + interfaceName.value);
			}
			lexer->Next(); // skipping `{` -> [member declaration]
			while (lexer->Peek().type != Token::Type::BRACE_BRACKET_C)
			{
				if (lexer->End()) // `}` expected
				{
					THROW("closing bracket not found in interface: " + interfaceName.value);
				}

				ModifierList memberModifiers;
				if (!GetModifiers(memberModifiers, { Token::Type::VARIABLE, Token::Type::FUNCTION, Token::Type::BRACE_BRACKET_C }))
				{
					THROW("cannot process member in interface: " + interfaceName.value);
				}
				if (lexer->Peek().type == Token::Type::VARIABLE) // variables are not allowed
				{
					THROW("interfaces cannot contain variables: " + interfaceName.value);
				}
				else if (lexer->Peek().type == Token::Type::FUNCTION)
				{
					if (!ProcessMethod(_namespace, interfaceObject, memberModifiers))
					{
						THROW("cannot process method in interface: " + interfaceName.value);
					}
				}
			}
			lexer->Next(); // skipping `}` -> [next unit]

			if (interfaceObject.GetMethods().empty())
			{
				Warning(interfaceName.value + ": interface declared empty");
			}
			return true; // interface is parsed successfully
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

			if (classObject.IsInterface())
			{
				if (find(modifiers, Token::Type::PRIVATE))
				{
					THROW("interface methods cannot be `private`: " + memberName);
				}
				if (isStatic)
				{
					THROW("interface methods cannot be `static`: " + memberName);
				}
				if (isAbstract)
				{
					THROW("interface methods cannot be `abstract`: " + memberName);
				}
				if (isPublic)
				{
					Warning("interface methods are `public` by default: " + memberName);
				}
				isPublic = true;
				isAbstract = true;
				isStatic = true;
			}

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
			if (!isStatic)
			{
				functionObject.params.push_back("this");
			}
			functionObject.name = memberName;
			if (!ParseFunctionParamsDecl(functionObject.params))
			{
				THROW("Error while parsing function parameter list: " + functionObject.name);
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
				functionObject.modifiers |= Function::Modifiers::_CONSTRUCTOR;
			}

			lexer->Next(); // skipping `)` -> `{` 
			if (lexer->Peek().type == Token::Type::BRACE_BRACKET_O)
			{
				if (classObject.IsInterface())
				{
					lexer->Prev();
					THROW("interface method cannot contain body: " + memberName);
				}
				unique_ptr<ExpressionList> body(new ExpressionList(ParseExpressionBlock(functionObject)));
				if (body->empty() || dynamic_cast<ReturnExpression*>(body->back().get()) == nullptr) // if no return, return void
				{
					body->push_back(unique_ptr<BaseExpression>(new ReturnExpression()));
				}
				functionObject.body = std::move(body);
				lexer->Next(); // skipping `}`
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

			if (isStatic && !isAbstract && (memberName == entryPointName))
			{
				hasEntryPoint = true;
				functionObject.modifiers |= Function::Modifiers::_ENTRY_POINT;
				Debug("entry-point located in class: " + classObject.name);
			}
			if (classObject.ContainsMember(functionObject.name))
			{
				lexer->Prev();
				THROW("this member has already been declared: " + memberName);
			}
			if (classObject.ContainsMember(functionInnerName))
			{
				lexer->Prev();
				THROW("this member has already been declared: " + memberName);
			}
			classObject.InsertMethod(memberName, std::move(functionObject));

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

				const std::string paramName = lexer->Peek().value;
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
				Error("`{` expected, found: " + lexer->Peek().value);
				return ExpressionList(); // no `{` found
			}
			lexer->Next();
			while (!lexer->End())
			{
				if (lexer->Peek().type == Token::Type::BRACE_BRACKET_C)
				{
					return block;
				}
				unique_ptr<BaseExpression> expr = ParseExpression(function);
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
			ExpressionList args;
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
			unique_ptr<BaseExpression> expr = ParseNextVariable(function);
			if (lexer->Peek().type != Token::Type::SQUARE_BRACKET_C)
			{
				Error("`]` exprected in index expression");
			}
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
			default:
				return ParseRawExpression(function);
			}
		}

		unique_ptr<BaseExpression> Parser::ParseVariableDecl(Function& function)
		{
			unique_ptr<ObjectDeclareExpression> variable(new ObjectDeclareExpression());
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
			unique_ptr<LambdaExpression> lambdaExpr(new LambdaExpression());
			lexer->Next(); // skipping `lambda` -> `(`
			if (!ParseFunctionParamsDecl(lambdaExpr->params))
			{
				Error("could not parse parameter list in lambda definition");
			}
			lexer->Next(); // skipping `)` -> `{`
			lambdaExpr->body = ParseExpressionBlock(function);
			lexer->Next(); // skipping `}`
			return TO_BASE(lambdaExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseForExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::FOR)
			{
				Error("`for` expected");
			}
			unique_ptr<ForExpression> forExpr(new ForExpression());
			lexer->Next(); // skipping `for` -> `(`
			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_O)
			{
				Error("`(` expected in for loop expression");
			}
			lexer->Next(); // skipping `(`
			if (lexer->Peek().type == Token::Type::VARIABLE) // for(var i = ...
			{
				forExpr->init = ParseVariableDecl(function);
			}
			else // for(i = ...
			{
				forExpr->init = ParseRawExpression(function);
			}
			lexer->Next(); // skipping `;`
			if (lexer->Peek().type == Token::Type::SEMICOLON) // no predicate
			{
				unique_ptr<ObjectExpression> trueStatement(new ObjectExpression());
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
			lexer->Next(); // skipping `}`
			return TO_BASE(forExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseForeachExpression(Function& function)
		{
			unique_ptr<ForeachExpression> foreachExpr(new ForeachExpression());
			function.InsertDependency("Begin_0");
			function.InsertDependency("Next_0");
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
			if (function.ContainsLocal(iteratorName))
			{
				Error("iterator variable has already been declared in function scope (function " + function.name + ')');
			}
			function.InsertLocal(iteratorName);
			foreachExpr->iterator = iteratorName;

			lexer->Next(); // skipping [Class name] -> `in`
			if (lexer->Peek().type != Token::Type::IN)
			{
				Error("`in` expected in foreach statement after iterator declaration");
			}

			lexer->Next(); // skipping `in` -> [container]
			unique_ptr<ObjectDeclareExpression> container(new ObjectDeclareExpression());
			container->objectName = "__CONTAINER_COMPILE_" + std::to_string(function.labelInnerId++);
			function.InsertDependency(container->objectName);
			container->assignment = ParseRawExpression(function);
			foreachExpr->container = TO_BASE(container);

			if (lexer->Peek().type != Token::Type::ROUND_BRACKET_C)
			{
				Error("`)` expected in foreach statement");
			}
			lexer->Next();
			foreachExpr->body = ParseExpressionBlock(function);
			lexer->Next(); // skipping `}`
			return TO_BASE(foreachExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseWhileExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::WHILE)
			{
				Error("`while` expected");
			}
			lexer->Next(); // skipping `while` -> `(`
			unique_ptr<WhileExpression> whileExpr(new WhileExpression());
			whileExpr->predicate = ParseStatementInBrackets(function);
			lexer->Next(); // skipping `)` -> `{`
			whileExpr->body = ParseExpressionBlock(function);
			lexer->Next(); // skipping `}`
			return TO_BASE(whileExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseIfExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::IF)
			{
				Error("`if` expected");
			}
			unique_ptr<IfExpression> ifExpr(new IfExpression());

			lexer->Next(); // skipping `if` -> `(`
			ifExpr->ifStatements.push_back(ParseStatementInBrackets(function));
			lexer->Next(); // skipping `)` -> `{`
			ifExpr->bodies.push_back(ParseExpressionBlock(function));
			lexer->Next(); // skipping `}`

			while (lexer->Peek().type == Token::Type::ELIF)
			{
				lexer->Next(); // skipping `elif` -> `(`
				ifExpr->ifStatements.push_back(ParseStatementInBrackets(function));
				lexer->Next(); // skipping `)` -> `{`
				ifExpr->bodies.push_back(ParseExpressionBlock(function));
				lexer->Next(); // skipping `}`
			}

			if (lexer->Peek().type == Token::Type::ELSE)
			{
				lexer->Next(); // skipping `else` -> `{`
				ifExpr->bodies.push_back(ParseExpressionBlock(function));
				lexer->Next(); // skipping `}`
			}
			return TO_BASE(ifExpr);
		}

		unique_ptr<BaseExpression> Parser::ParseReturnExpression(Function& function)
		{
			if (lexer->Peek().type != Token::Type::RETURN)
			{
				Error("`return` expected");
			}
			unique_ptr<ReturnExpression> returnExpr(new ReturnExpression());
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

		unique_ptr<BaseExpression> Parser::ParseNextVariable(Function& function)
		{
			if (lexer->Peek().type & Token::Type::UNARY_OPERAND)
			{
				unique_ptr<UnaryExpression> unaryExpr(new UnaryExpression());
				unaryExpr->type = lexer->Peek().type;
				lexer->Next(); // skipping [unary_op]
				unaryExpr->expression = ParseNextVariable(function);
				return TO_BASE(unaryExpr);
			}
			if (lexer->Peek().type == Token::Type::ROUND_BRACKET_O)
			{
				unique_ptr<BaseExpression> statement = ParseStatementInBrackets(function);
				lexer->Next(); // skipping `)`
				return statement;
			}
			if (lexer->Peek().type & Token::Type::VALUE_TYPE)
			{
				Token& object = lexer->Peek();
				lexer->Next(); // skipping [var] -> `(` / `[`
				Token& nextToken = lexer->Peek();
				if (nextToken.type == Token::Type::ROUND_BRACKET_O)
				{
					unique_ptr<CallExpression> callExpr(new CallExpression());
					callExpr->parameters = ParseFunctionArguments(function);
					callExpr->functionName = Function::GenerateUniqueName(object.value, callExpr->parameters.size());
					function.InsertDependency(callExpr->functionName);
					lexer->Next(); // skipping `)`
					return TO_BASE(callExpr);
				}
				else if (nextToken.type == Token::Type::SQUARE_BRACKET_O)
				{
					unique_ptr<IndexExpression> indexExpr(new IndexExpression());
					indexExpr->objectName = object.value;
					function.InsertDependency(object.value);
					indexExpr->parameter = ParseIndexArgument(function);
					lexer->Next(); // skipping `]`
					return TO_BASE(indexExpr);
				}
				else
				{
					unique_ptr<ObjectExpression> expr(new ObjectExpression());
					if (object.type == Token::Type::THIS && (function.isStatic()))
					{
						Error("`this` reference inside a static function: " + function.name);
					}
					expr->object = object;
					function.InsertDependency(object.value);
					return TO_BASE(expr);
				}
			}
			Error("value, bracket expression or unary operand expected");
			return nullptr;
		}

		unique_ptr<BaseExpression> Parser::ParseRawExpression(Function& function, unique_ptr<BaseExpression> leftBranch)
		{
			const Token& firstToken = lexer->Peek();
			if (firstToken.type == Token::Type::LAMBDA) // lambda cannot be used, so going into this if will cause error while parsing
			{
				unique_ptr<BaseExpression> expr = ParseLambdaExpression(function);
				return ParseRawExpression(function, std::move(expr));
			}
			if (firstToken.type & Token::Type::UNARY_OPERAND)
			{
				if (leftBranch != nullptr)
				{
					Error("binary operand expected, but unary found");
				}
				unique_ptr<BaseExpression> expr = ParseNextVariable(function);
				return ParseRawExpression(function, std::move(expr));
			}
			if (firstToken.type & Token::Type::BINARY_OPERAND) // left branch should not be `nullptr`
			{
				unique_ptr<BinaryExpression> binExpr(new BinaryExpression());
				if (leftBranch == nullptr)
				{
					Error("value type before binary operand expected");
				}
				binExpr->left = std::move(leftBranch);
				binExpr->type = firstToken.type;
				uint32_t currentPriority = firstToken.type & Token::PRIORITY;
				lexer->Next(); // skipping `BINARY_OP`
				unique_ptr<BaseExpression> rightExpr = ParseNextVariable(function);
				uint32_t nextPriority = lexer->Peek().type & Token::PRIORITY;
				if (currentPriority < nextPriority)
				{
					binExpr->right = ParseRawExpression(function, std::move(rightExpr));
					return TO_BASE(binExpr);
				}
				else
				{
					CallExpression* call = dynamic_cast<CallExpression*>(rightExpr.get());
					if (call != nullptr)
					{
						if (firstToken.type == Token::Type::DOT)
							call->hasParent = true;
					}
					binExpr->right = std::move(rightExpr);
					return ParseRawExpression(function, TO_BASE(binExpr));
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
			if (leftBranch == nullptr)
			{
				lexer->Prev();
				Error("binary operand expected, but not found");
				lexer->Next();

				lexer->Next();
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
				if (mode & (Mode::ERROR_ONLY | Mode::NO_STACKTRACE))
				{
					message = "[ERROR]: " + message;
					Notify(message);
					Notify("\t on token: `" + lexer->Peek().value + "`");
				}
				if (success) ShowErrorLine(); // happens only on first call
				if (mode & Mode::NO_STACKTRACE)
				{
					mode &= Mode::NO_OUTPUT;
				}
				currentErrorCount++;
			}
			success = false;
		}

		void Parser::Debug(std::string message)
		{
			if (mode & Mode::DEBUG_ONLY)
			{
				message = "[DEBUG]: " + message;
				Notify(message);
			}
		}

		void Parser::Warning(std::string message)
		{
			if (mode & Mode::WARNING_ONLY)
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
			Notify(lineCount + lexer->ToNextLine() + '\n' + filler);
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