#include "parser.h"

#include <set>
#include <sstream>

using namespace MSL::utils;

void yyset_lexer(MSL::compiler::Lexer* lexer);
int yyparse(MSL::compiler::BaseAstNode** root);

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
            yyset_lexer(this->lexer);
            BaseAstNode* AST = nullptr;
            yyparse(&AST);

            if (AST == nullptr)
            {
                success = false;
                return false;
            }

             AST->DebugPrint(0, "\n");
            
            CompileAST(AST);
            delete AST;
            return true;
		}

		Assembly&& Parser::PullAssembly()
		{
			return std::move(assembly);
		}

        std::stringstream Parser::CompileMethodBody(Method& method, AstNodeList* body)
        {
            std::stringstream out;

            if (dynamic_cast<ReturnExprNode*>(body->vec.back()) == nullptr)
            {
                body->vec.push_back(new ReturnExprNode(nullptr));
            }

            body->GenerateBytecode(out, method);

            for (const auto& error : method.errors)
            {
                Error(error + " in method: " + method.name);
            }

            return out;
        }

        Method Parser::CompileMethod(MethodDeclNode* method)
        {
            std::set<std::string> modifiers;
            Method _method(method->name);
            _method.params = method->args->vec;
            if (method->body != nullptr) 
                _method.bytecode = CompileMethodBody(_method, method->body);

            for (const auto& modifier : method->modifiers->vec)
            {
                if (modifiers.find(modifier) != modifiers.end())
                {
                    Warning("`" + modifier + "` modifier already exists: " + method->name);
                }
                else if (modifier == "private")
                {
                    if (modifiers.find("public") != modifiers.end())
                    {
                        Error("`public` and `private` modifiers cannot be applied together in method: " + method->name);
                    }
                }
                else if (modifier == "public")
                {
                    if (modifiers.find("private") != modifiers.end())
                    {
                        Error("`public` and `private` modifiers cannot be applied together in method: " + method->name);
                    }
                    else
                    {
                        _method.modifiers |= Method::_PUBLIC;
                    }
                }
                else if (modifier == "static")
                {
                    _method.modifiers |= Method::_STATIC;
                }
                else if (modifier == "abstract")
                {
                    if (method->body != nullptr)
                    {
                        Error("method with body cannot be abstract: " + method->name);
                    }
                    _method.modifiers |= Method::_ABSTRACT;
                }
                else
                {
                    Error("`" + modifier + "` cannot be used with method: " + method->name);
                }
                modifiers.insert(modifier);
            }

            if (!_method.IsAbstract() && method->body == nullptr)
            {
                Error("non-abstract method must have body: " + _method.name);
            }

            return _method;
        }

        Attribute Parser::CompileAttribute(AttributeDeclNode* attr)
        {
            std::set<std::string> modifiers;
            Attribute _attribute(attr->name);

            for (const auto& modifier : attr->modifiers->vec)
            {
                if (modifiers.find(modifier) != modifiers.end())
                {
                    Warning("`" + modifier + "` modifier already exists: " + attr->name);
                }
                else if (modifier == "const")
                {
                    _attribute.modifiers |= Attribute::_CONST;
                }
                else if (modifier == "private")
                {
                    if (modifiers.find("public") != modifiers.end())
                    {
                        Error("`public` and `private` modifiers cannot be applied together in attribute: " + attr->name);
                    }
                }
                else if (modifier == "public")
                {
                    if (modifiers.find("private") != modifiers.end())
                    {
                        Error("`public` and `private` modifiers cannot be applied together in attribute: " + attr->name);
                    }
                    else
                    {
                        _attribute.modifiers |= Attribute::_PUBLIC;
                    }
                }
                else if (modifier == "static")
                {
                    _attribute.modifiers |= Attribute::_STATIC;
                }
                else
                {
                    Error("`" + modifier + "` cannot be used with attribute: " + attr->name);
                }
                modifiers.insert(modifier);
            }
            return _attribute;
        }

        Class Parser::CompileClass(ClassDeclNode* cl)
        {
            std::set<std::string> modifiers;
            Class _class(cl->name);

            for (const auto& member : cl->members->vec)
            {
                if (dynamic_cast<AttributeDeclNode*>(member) != nullptr)
                {
                    auto attr = static_cast<AttributeDeclNode*>(member);
                    auto attribute = CompileAttribute(attr);
                    if (!attribute.IsStatic() && _class.IsStatic())
                    {
                        Warning("attribute `" + attr->name + "` is not declared `static` but located in static class: " + _class.name);
                        attribute.modifiers |= Attribute::_STATIC;
                    }

                    if (_class.ContainsMember(attr->name))
                    {
                        Error("attribute `" + attr->name + "` already declared in class: " + cl->name);
                    }
                    else
                    {
                        _class.InsertAttribute(attr->name, std::move(attribute));
                    }
                }
                else
                {
                    auto m = static_cast<MethodDeclNode*>(member);
                    auto method = CompileMethod(m);
                    if (!method.IsStatic() && _class.IsStatic())
                    {
                        Warning("method `" + m->name + "` is not declared `static` but located in static class: " + _class.name);
                        method.modifiers |= Method::_STATIC;
                    }
                    if (method.name == cl->name)
                    {
                        method.modifiers |= Method::_CONSTRUCTOR;
                        if (method.IsStatic())
                            method.modifiers |= Method::_STATIC_CONSTRUCTOR;
                    }
                    if (method.name == entryPointName)
                    {
                        method.modifiers |= Method::_ENTRY_POINT;
                    }
                    if (!method.IsStatic() && !method.isConstructor())
                    {
                        method.params.insert(method.params.begin(), "this");
                    }

                    std::string methodName = Method::GenerateUniqueName(m->name, method.params.size());
                    if ((method.modifiers & Method::_STATIC_CONSTRUCTOR) != 0)
                        methodName += "static";
                    
                    if (_class.ContainsMember(methodName))
                    {
                        Error("method `" + m->name + "` already declared in class: " + cl->name);
                    }
                    else
                    {
                        _class.InsertMethod(methodName, std::move(method));
                    }
                }
            }

            for (const auto& modifier : cl->modifiers->vec)
            {
                if (modifiers.find(modifier) != modifiers.end())
                {
                    Warning("`" + modifier + "` modifier already exists: " + cl->name);
                }
                else if (modifier == "private")
                {
                    if (modifiers.find("public") != modifiers.end())
                    {
                        Error("`public` and `private` modifiers cannot be applied together in class: " + cl->name);
                    }
                    else
                    {
                        _class.modifiers |= Class::_PRIVATE;
                    }
                }
                else if (modifier == "public")
                {
                    if (modifiers.find("private") != modifiers.end())
                    {
                        Error("`public` and `private` modifiers cannot be applied together in class: " + cl->name);
                    }
                }
                else if (modifier == "static")
                {
                    _class.modifiers |= Class::_STATIC;
                }
                else if (modifier == "abstract")
                {
                    for (const auto& method : _class.GetMethods())
                    {
                        if (!method.IsAbstract())
                        {
                            Error("class '" + cl->name + "' declared `abstract`, but has non-abstract method: " + method.name);
                        }
                    }
                    _class.modifiers |= Class::_ABSTRACT;
                }
                else
                {
                    Error("`" + modifier + "` cannot be used with class: " + cl->name);
                }
                modifiers.insert(modifier);
            }

            bool hasAnyConstructor = false;
            for (const auto& method : _class.GetMethods())
            {
                if (method.isConstructor() && !method.IsStatic()) hasAnyConstructor = true;

                if (method.isConstructor() && method.IsStatic())
                {
                    if (!method.params.empty())
                    {
                        Error("static constructor cannot have arguments: " + cl->name);
                    }
                    else if ((_class.modifiers & Class::_HAS_STATIC_CONSTRUCTOR) != 0)
                    {
                        Error("static constructor cannot be declared twice: " + cl->name);
                    }
                    else
                    {
                        _class.modifiers |= Class::_HAS_STATIC_CONSTRUCTOR;
                    }
                }

                if (method.IsEntryPoint())
                {
                    if (this->hasEntryPoint)
                    {
                        Error("entry point method (" + this->entryPointName + ") declared twice: " + cl->name);
                    }
                    else
                    {
                        this->hasEntryPoint = true;
                    }
                }
            }

            if (_class.name == entryPointName)
            {
                Error("class cannot be same name as entry point method: " + _class.name);
            }
            
            if (!hasAnyConstructor && !_class.IsAbstract() && !_class.IsStatic())
            {
                std::string constructorName = _class.name + "_0";
                Method defaultConstructor(constructorName);
                defaultConstructor.modifiers = Method::Modifiers::_PUBLIC | Method::Modifiers::_CONSTRUCTOR;

                auto body = std::make_unique<AstNodeList>();
                body->vec.push_back(new ReturnExprNode(nullptr));
                defaultConstructor.bytecode = CompileMethodBody(defaultConstructor, body.get());

                defaultConstructor.name = _class.name;
                _class.InsertMethod(constructorName, std::move(defaultConstructor));
            }

            return _class;
        }

        Namespace Parser::CompileNamespace(NamespaceDeclNode* ns)
        {
            Namespace _namespace(ns->name);
            for (auto member : ns->members->vec)
            {
                if (dynamic_cast<UsingNamespaceNode*>(member) != nullptr)
                {
                    auto nsFriend = static_cast<UsingNamespaceNode*>(member)->namespaceName;
                    if (_namespace.friendNamespaces.find(nsFriend) != _namespace.friendNamespaces.end())
                    {
                        Warning("namespace " + nsFriend + " is already a friend namespace to " + _namespace.GetName());
                    }
                    _namespace.friendNamespaces.insert(nsFriend);
                }
                else
                {
                    Class _class = CompileClass(static_cast<ClassDeclNode*>(member));
                    if (_namespace.ContainsClass(_class.name))
                    {
                        Error("class " + _class.name + " already exists in namespace " + ns->name);
                    }
                    else
                    {
                        _namespace.InsertClass(_class.name, std::move(_class));
                    }
                }
            }
            return _namespace;
        }

        void Parser::CompileAST(BaseAstNode* AST)
        {
            auto namespaces = static_cast<AstNodeList*>(AST)->vec;
            for (auto ns : namespaces)
            {
                auto _ns = static_cast<NamespaceDeclNode*>(ns);
                Namespace _namespace = CompileNamespace(_ns);
                this->assembly.InsertNamespace(_namespace.GetName(), std::move(_namespace));
            }
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
					//Notify("\t on token: `" + lexer->Peek().value + "`");
				}
				//if (success) ShowErrorLine(); // happens only on first call
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