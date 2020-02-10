#pragma once

#include "token.h"

namespace MSL
{
	namespace compiler
	{     
        class Method;

        struct BaseAstNode
        {
            virtual void DebugPrint(int offset, const char* delim) const = 0;
            virtual void Destroy() = 0;
            virtual void GenerateBytecode(std::ostream& out, Method& function) = 0;
            virtual ~BaseAstNode() = default;
        };

        struct ObjectNode : BaseAstNode
        {
            std::string value;
            Token::Type type;
            inline ObjectNode(const std::string& value, Token::Type type) : 
                value(value), type(type) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct MemberCallNode : BaseAstNode
        {
            BaseAstNode* parent;
            std::string object;

            inline MemberCallNode(BaseAstNode* parent, const std::string& object) :
                parent(parent), object(object) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct IndexCallNode : BaseAstNode
        {
            BaseAstNode* parent;
            BaseAstNode* index;
            inline IndexCallNode(BaseAstNode* parent, BaseAstNode* index) :
                parent(parent), index(index) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct AstNodeList : BaseAstNode
        {
            std::vector<BaseAstNode*> vec;

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct StringList : BaseAstNode
        {
            std::vector<std::string> vec;

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct MethodCallNode : BaseAstNode
        {
            std::string name;
            BaseAstNode* parent;
            AstNodeList* args;
            inline MethodCallNode(const std::string& name, BaseAstNode* parent, AstNodeList* args) :
                name(name), parent(parent), args(args) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct BinaryExprNode : BaseAstNode
        {
            BaseAstNode* left;
            BaseAstNode* right;
            Token::Type op;
            inline BinaryExprNode(BaseAstNode* left, BaseAstNode* right, Token::Type op) :
                left(left), right(right), op(op) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct UnaryExprNode : BaseAstNode
        {
            BaseAstNode* expr;
            Token::Type op;
            inline UnaryExprNode(BaseAstNode* expr, Token::Type op) :
                expr(expr), op(op) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct ReturnExprNode : BaseAstNode
        {
            BaseAstNode* value;
            inline ReturnExprNode(BaseAstNode* value) : value(value) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct VariableDeclNode : BaseAstNode
        {
            std::string name;
            bool isConst;
            BaseAstNode* value;
            inline VariableDeclNode(const std::string& name, bool isConst, BaseAstNode* value) :
                name(name), isConst(isConst), value(value) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct TryCatchNode : BaseAstNode
        {
            std::string excName;
            BaseAstNode* tryExpr;
            BaseAstNode* catchExpr;
            inline TryCatchNode(const std::string& excName, BaseAstNode* tryExpr, BaseAstNode* catchExpr) :
                excName(excName), tryExpr(tryExpr), catchExpr(catchExpr) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct ConditionalNode : BaseAstNode
        {
            BaseAstNode* predicate;
            BaseAstNode* body;
            inline ConditionalNode(BaseAstNode* predicate, BaseAstNode* body) :
                predicate(predicate), body(body) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct IfStatementNode : BaseAstNode
        {
            ConditionalNode* ifBlock;
            AstNodeList* elifBlocks;
            ConditionalNode* elseBlock;
            inline IfStatementNode(ConditionalNode* ifBlock, AstNodeList* elifBlocks, ConditionalNode* elseBlock) :
                ifBlock(ifBlock), elifBlocks(elifBlocks), elseBlock(elseBlock) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct ForExprNode : BaseAstNode
        {
            BaseAstNode* init;
            BaseAstNode* pred;
            BaseAstNode* iter;
            BaseAstNode* body;
            inline ForExprNode(BaseAstNode* init, BaseAstNode* pred, BaseAstNode* iter, BaseAstNode* body) :
                init(init), pred(pred), iter(iter), body(body) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct ForeachExprNode : BaseAstNode
        {
            std::string iterator;
            BaseAstNode* container;
            BaseAstNode* body;
            inline ForeachExprNode(const std::string& iterator, BaseAstNode* container, BaseAstNode* body) :
                iterator(iterator), container(container), body(body) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct MethodDeclNode : BaseAstNode
        {
            std::string name;
            StringList* args;
            AstNodeList* body;
            StringList* modifiers;
            inline MethodDeclNode(const std::string& name, StringList* args, AstNodeList* body) :
                name(name), args(args), body(body) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct AttributeDeclNode : BaseAstNode
        {
            std::string name;
            StringList* modifiers;
            inline AttributeDeclNode(const std::string& name) :
                name(name) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct ClassDeclNode : BaseAstNode
        {
            std::string name;
            StringList* modifiers;
            AstNodeList* members;
            inline ClassDeclNode(const std::string& name) :
                name(name) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct UsingNamespaceNode : BaseAstNode
        {
            std::string namespaceName;
            inline UsingNamespaceNode(const std::string& namespaceName) :
                namespaceName(namespaceName) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };

        struct NamespaceDeclNode : BaseAstNode
        {
            std::string name;
            AstNodeList* members;
            inline NamespaceDeclNode(const std::string& name, AstNodeList* members) :
                name(name), members(members) { }

            // Inherited via BaseAstNode
            virtual void DebugPrint(int offset, const char* delim) const override;
            virtual void Destroy() override;
            virtual void GenerateBytecode(std::ostream& out, Method& function) override;
        };
	}
}