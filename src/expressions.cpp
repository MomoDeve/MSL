#include "expressions.h"
#include "opcode.h"
#include "function.h"

#include <iostream>
#include <exception>

#define TO_BASE(derived_ptr) unique_ptr<BaseExpression>(derived_ptr.release())

namespace MSL
{
	namespace compiler
	{
		using namespace VM;

        template<typename T>
        void Write(std::ostream& out, T value)
        {
            out.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }

#define OFFSET_PRINT(...) std::cout << std::string(offset * 2, ' ') << __VA_ARGS__

        void ObjectNode::DebugPrint(int offset, const char* delim) const
        {
            if (this->type == Token::Type::STRING_CONSTANT)
            {
                OFFSET_PRINT('\"' << this->value << '\"');
            }
            else
            {
                OFFSET_PRINT(this->value);
            }
        }

        void ObjectNode::Destroy()
        {
            
        }

        void ObjectNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            function.InsertDependency(this->value);

            switch (type)
            {
            case Token::Type::STRING_CONSTANT:
                Write(out, OPCODE::PUSH_STRING);
                Write(out, function.GetHash(value));
                break;
            case Token::Type::INTEGER_CONSTANT:
                Write(out, OPCODE::PUSH_INTEGER);
                Write(out, function.GetHash(value));
                break;
            case Token::Type::FLOAT_CONSTANT:
                Write(out, OPCODE::PUSH_FLOAT);
                Write(out, function.GetHash(value));
                break;
            case Token::Type::OBJECT:
                Write(out, OPCODE::PUSH_OBJECT);
                Write(out, function.GetHash(value));
                break;
            case Token::Type::TRUE_CONSTANT:
                Write(out, OPCODE::PUSH_TRUE);
                break;
            case Token::Type::FALSE_CONSTANT:
                Write(out, OPCODE::PUSH_FALSE);
                break;
            case Token::Type::THIS:
                Write(out, OPCODE::PUSH_THIS);
                break;
            case Token::Type::NULLPTR:
                Write(out, OPCODE::PUSH_NULL);
                break;
            default:
                Write(out, OPCODE::ERROR_SYMBOL);
                break;
            }
        }

        void MemberCallNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("<");
            this->parent->DebugPrint(0, delim);
            std::cout << "." << this->object << ">";
        }

#define FREE_AST(node) node->Destroy(); delete node; node = nullptr;

        void MemberCallNode::Destroy()
        {
            FREE_AST(parent)
        }

        void MemberCallNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            function.InsertDependency(this->object);

            this->parent->GenerateBytecode(out, function);
            Write(out, OPCODE::GET_MEMBER);
            Write(out, function.GetHash(this->object));
        }

        void IndexCallNode::DebugPrint(int offset, const char* delim) const
        {
            this->parent->DebugPrint(offset, delim);
            std::cout << "["; 
            this->index->DebugPrint(0, delim);
            std::cout << "]";
        }

        void IndexCallNode::Destroy()
        {
            FREE_AST(parent)
        }

        void IndexCallNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            this->parent->GenerateBytecode(out, function);
            this->index->GenerateBytecode(out, function);
            Write(out, OPCODE::GET_INDEX); 
        }

        void AstNodeList::DebugPrint(int offset, const char* delim) const
        {
            for (int i = 0; i < this->vec.size(); i++)
            {
                this->vec[i]->DebugPrint(offset, delim);
                if (i + 1 != this->vec.size() || delim[0] != ',') 
                    std::cout << delim;
            }
        }

        void AstNodeList::Destroy()
        {
            for (auto& node : this->vec)
            {
                FREE_AST(node);
            }
            this->vec.clear();
        }

        void AstNodeList::GenerateBytecode(std::ostream& out, Method& function)
        {
            for (const auto& expression : this->vec)
            {
                expression->GenerateBytecode(out, function);
                if (
                    dynamic_cast<BinaryExprNode*>(expression) != nullptr ||
                    dynamic_cast<UnaryExprNode*>(expression) != nullptr ||
                    dynamic_cast<MethodCallNode*>(expression) != nullptr ||
                    dynamic_cast<IndexCallNode*>(expression) != nullptr ||
                    dynamic_cast<VariableDeclNode*>(expression) != nullptr ||
                    dynamic_cast<ObjectNode*>(expression) != nullptr
                    )
                {
                    Write(out, OPCODE::POP_STACK_TOP); // if result of these expressions is not needed, it should be discarded by stack_pop
                }
            }
        }

        void StringList::DebugPrint(int offset, const char* delim) const
        {
            for (int i = 0; i < this->vec.size(); i++)
            {
                std::cout << this->vec[i];
                if (i + 1 != this->vec.size() || delim[0] != ',')
                    std::cout << delim;
            }
        }

        void StringList::Destroy()
        {
            this->vec.clear();
        }

        void StringList::GenerateBytecode(std::ostream& out, Method& function)
        {
            throw std::exception("StringList node does not implement GenerateBytecode");
        }

        void MethodCallNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT('<');
            if (this->parent != nullptr)
            {
                this->parent->DebugPrint(0, delim);
                std::cout << '.';
            }

            std::cout << this->name << "(";
            this->args->DebugPrint(0, ", ");
            std::cout << ")>";
        }

        void MethodCallNode::Destroy()
        {
            FREE_AST(parent)
            FREE_AST(args)
        }

        void MethodCallNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            if (this->parent != nullptr)
            {
                this->parent->GenerateBytecode(out, function);
            }
            else
            {
                // method with no arguments is called by `this`
                Write(out, OPCODE::PUSH_THIS);
            }

            for (const auto& arg : this->args->vec)
            {
                arg->GenerateBytecode(out, function);
            }
            std::string methodName = this->name + '_' + std::to_string(this->args->vec.size());
            function.InsertDependency(methodName);
            Write(out, OPCODE::CALL_FUNCTION);
            Write(out, function.GetHash(methodName));
            Write(out, static_cast<uint8_t>(this->args->vec.size()));
        }


        void BinaryExprNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("(");
            this->left->DebugPrint(0, delim);
            std::cout << " " << Token::ToString(this->op) << " ";
            this->right->DebugPrint(0, delim);
            std::cout << ")";
        }

        void BinaryExprNode::Destroy()
        {
            this->left->Destroy();
            this->right->Destroy();
            delete left;
            delete right;
            left = right = nullptr;
        }

        void BinaryExprNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            this->left->GenerateBytecode(out, function);
            this->right->GenerateBytecode(out, function);
            switch (this->op)
            {
            case Token::Type::SUM_OP:
                Write(out, OPCODE::SUM_OP);
                break;
            case Token::Type::SUB_OP:
                Write(out, OPCODE::SUB_OP);
                break;
            case Token::Type::MULT_OP:
                Write(out, OPCODE::MULT_OP);
                break;
            case Token::Type::DIV_OP:
                Write(out, OPCODE::DIV_OP);
                break;
            case Token::Type::MOD_OP:
                Write(out, OPCODE::MOD_OP);
                break;
            case Token::Type::POWER_OP:
                Write(out, OPCODE::POWER_OP);
                break;
            case Token::Type::ASSIGN_OP:
                Write(out, OPCODE::ASSIGN_OP);
                break;
            case Token::Type::LOGIC_EQUALS:
                Write(out, OPCODE::CMP_EQ);
                break;
            case Token::Type::LOGIC_NOT_EQUALS:
                Write(out, OPCODE::CMP_NEQ);
                break;
            case Token::Type::LOGIC_LESS:
                Write(out, OPCODE::CMP_L);
                break;
            case Token::Type::LOGIC_GREATER:
                Write(out, OPCODE::CMP_G);
                break;
            case Token::Type::LOGIC_LESS_EQUALS:
                Write(out, OPCODE::CMP_LE);
                break;
            case Token::Type::LOGIC_GREATER_EQUALS:
                Write(out, OPCODE::CMP_GE);
                break;
            case Token::Type::LOGIC_OR:
                Write(out, OPCODE::CMP_OR);
                break;
            case Token::Type::LOGIC_AND:
                Write(out, OPCODE::CMP_AND);
                break;
            default:
                Write(out, OPCODE::ERROR_SYMBOL);
                break;
            }
        }

        void ReturnExprNode::DebugPrint(int offset, const char* delim) const
        {
            if (this->value != nullptr)
            {
                OFFSET_PRINT("return ");
                this->value->DebugPrint(0, delim);
            }
            else
            {
                OFFSET_PRINT("return");
            }
        }

        void ReturnExprNode::Destroy()
        {
            FREE_AST(value)
        }

        void ReturnExprNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            if(this->value == nullptr)
            {
                Write(out, OPCODE::RETURN);
            }
            else
            {
                value->GenerateBytecode(out, function);
                Write(out, OPCODE::POP_TO_RETURN);
            }
        }

        void VariableDeclNode::DebugPrint(int offset, const char* delim) const
        {
            if (this->isConst)
            {
                OFFSET_PRINT("const var " << this->name);
            }
            else
            {
                OFFSET_PRINT("var " << this->name);
            }
            if (this->value == nullptr)
                std::cout << ";";
            else
            {
                std::cout << " = ";
                this->value->DebugPrint(0, delim);
            }
        }

        void VariableDeclNode::Destroy()
        {
            FREE_AST(value)
        }

        void VariableDeclNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            if (function.ContainsLocal(this->name))
            {
                function.PushError("local variable already exists: " + this->name);
            }
            else
            {
                function.InsertLocal(this->name);
            }

            if (this->isConst)
            {
                Write(out, OPCODE::ALLOC_CONST_VAR);
            }
            else
            {
                Write(out, OPCODE::ALLOC_VAR);
            }
            Write(out, function.GetHash(this->name));

            if (this->value != nullptr)
            {
                this->value->GenerateBytecode(out, function);
            }
            else
            {
               Write(out, OPCODE::PUSH_NULL);
            }
            Write(out, OPCODE::ASSIGN_OP);
        }

        void MethodDeclNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("");
            this->modifiers->DebugPrint(0, " ");
            std::cout << "function " << this->name << "(";
            this->args->DebugPrint(0, ", ");
            std::cout << ")\n";
            if (this->body != nullptr)
            {
                OFFSET_PRINT("{\n");
                this->body->DebugPrint(offset + 2, ";\n");
                if (dynamic_cast<AstNodeList*>(this->body) == nullptr)
                    std::cout << ";\n";
                OFFSET_PRINT("}\n");
            }
            else
            {
                std::cout << ";\n";
            }
        }

        void MethodDeclNode::Destroy()
        {
            FREE_AST(body);
            FREE_AST(modifiers);
            FREE_AST(args);
        }

        void MethodDeclNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            throw std::exception("MethodDecl node does not implement GenerateBytecode");
        }

        void ForeachExprNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("foreach(var " << this->iterator << " in ");
            this->container->DebugPrint(0, delim);
            std::cout << ")\n";
            OFFSET_PRINT("{\n");
            this->body->DebugPrint(offset + 2, ";\n");
            if (dynamic_cast<AstNodeList*>(this->body) == nullptr)
                std::cout << ";\n";
            OFFSET_PRINT("}");
        }

        void ForeachExprNode::Destroy()
        {
            FREE_AST(body)

            FREE_AST(container)
        }

        void ForeachExprNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            function.InsertDependency(this->iterator);
            Write(out, OPCODE::ALLOC_VAR);
            Write(out, function.GetHash(this->iterator));
            Write(out, OPCODE::POP_STACK_TOP);

            uint16_t labelId = function.labelInnerId;
            function.labelInnerId += 2; // predicate and end-foreach label

            // var iteratorIndex = container.Begin();
            std::string iteratorIndex = "__BEGIN_" + std::to_string(function.labelInnerId);
            function.InsertDependency(iteratorIndex);
            Write(out, OPCODE::ALLOC_VAR);
            Write(out, function.GetHash(iteratorIndex));

            this->container->GenerateBytecode(out, function);
            std::string container = "__CONTAINER_" + std::to_string(function.labelInnerId);
            function.InsertDependency(container);
            Write(out, OPCODE::ALLOC_VAR);
            Write(out, function.GetHash(container));
            Write(out, OPCODE::ASSIGN_OP);

            // call of .Begin(this);
            function.InsertDependency("Begin_0");
            Write(out, OPCODE::CALL_FUNCTION);
            Write(out, function.GetHash("Begin_0"));
            Write<uint8_t>(out, 0);

            Write(out, OPCODE::ASSIGN_OP); // init iterator with begin of container
            Write(out, OPCODE::POP_STACK_TOP);

            Write(out, OPCODE::SET_LABEL); // predicate
            Write(out, labelId);

            // if(iteratorIndex == container.End()) jump_end;
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(iteratorIndex));
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(container));

            // call of .End(this);
            function.InsertDependency("End_0");
            Write(out, OPCODE::CALL_FUNCTION);
            Write(out, function.GetHash("End_0"));
            Write<uint8_t>(out, 0);

            Write(out, OPCODE::CMP_EQ);
            Write(out, OPCODE::JUMP_IF_TRUE);
            Write<uint16_t>(out, labelId + 1); // to end-foreach

            // iterator = container.GetByIter(iteratorIndex);
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(iterator));
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(container));
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(iteratorIndex));

            // call of .GetByIter(this, iter);
            function.InsertDependency("GetByIter_1");
            Write(out, OPCODE::CALL_FUNCTION);
            Write(out, function.GetHash("GetByIter_1"));
            Write<uint8_t>(out, 1);

            Write(out, OPCODE::ASSIGN_OP);
            Write(out, OPCODE::POP_STACK_TOP);

            // generate body inside { }
            this->body->GenerateBytecode(out, function);

            // iteratorIndex = container.Next(iteratorIndex);
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(iteratorIndex));
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(container));
            Write(out, OPCODE::PUSH_OBJECT);
            Write(out, function.GetHash(iteratorIndex));

            // call of .Next(this, iter);
            function.InsertDependency("Next_1");
            Write(out, OPCODE::CALL_FUNCTION);
            Write(out, function.GetHash("Next_1"));
            Write(out, (uint8_t)1);

            Write(out, OPCODE::ASSIGN_OP);
            Write(out, OPCODE::POP_STACK_TOP);


            Write(out, OPCODE::JUMP);
            Write(out, labelId); // to predicate
            Write(out, OPCODE::SET_LABEL);
            Write<uint16_t>(out, labelId + 1); // end-foreach
        }

        void ForExprNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("for(");
            if (this->init != nullptr)
                this->init->DebugPrint(0, delim);
            std::cout << "; ";
            if (this->pred != nullptr)
                this->pred->DebugPrint(0, delim);
            std::cout << "; ";
            if (this->iter != nullptr)
                this->iter->DebugPrint(0, delim);
            std::cout << ")\n";
            OFFSET_PRINT("{\n");
            this->body->DebugPrint(offset + 2, ";\n");
            if (dynamic_cast<AstNodeList*>(this->body) == nullptr)
                std::cout << ";\n";
            OFFSET_PRINT("}");
        }

        void ForExprNode::Destroy()
        {
            FREE_AST(body)

            if (this->init != nullptr)
            {
                FREE_AST(init)
            }
            if (this->pred != nullptr)
            {
                FREE_AST(pred)
            }
            if (this->iter != nullptr)
            {
                FREE_AST(iter)
            }
        }

        void ForExprNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            if (this->init != nullptr)
            {
                size_t errCount = function.errors.size();
                this->init->GenerateBytecode(out, function);
                Write(out, OPCODE::POP_STACK_TOP);

                if (errCount < function.errors.size()) // iterator already declared somewhere
                    function.errors.pop_back();
            }
            uint16_t labelId = function.labelInnerId;
            function.labelInnerId += 2; // predicate and end-for label

            Write(out, OPCODE::SET_LABEL); // predicate
            Write(out, labelId);

            if (this->pred != nullptr)
            {
                this->pred->GenerateBytecode(out, function);
                Write(out, OPCODE::JUMP_IF_FALSE);
                Write<uint16_t>(out, labelId + 1); // to end-for
            }

            this->body->GenerateBytecode(out, function);
            if (this->iter != nullptr)
            {
                this->iter->GenerateBytecode(out, function);
                Write(out, OPCODE::POP_STACK_TOP);
            }

            Write(out, OPCODE::JUMP);
            Write(out, labelId); // to predicate
            Write(out, OPCODE::SET_LABEL);
            Write<uint16_t>(out, labelId + 1); // end-for
        }

        void IfStatementNode::DebugPrint(int offset, const char* delim) const
        {
            this->ifBlock->DebugPrint(offset, "a");
            if (this->elifBlocks != nullptr)
                this->elifBlocks->DebugPrint(offset, " ");
            if (this->elseBlock != nullptr)
                this->elseBlock->DebugPrint(offset, " ");
        }

        void IfStatementNode::Destroy()
        {
            FREE_AST(ifBlock)

            elifBlocks->Destroy();
            delete elifBlocks;
            elifBlocks = nullptr;
            
            if (this->elseBlock != nullptr)
            {
                FREE_AST(elseBlock)
            }
        }

        void IfStatementNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            uint16_t labelId = function.labelInnerId;
            function.labelInnerId += (uint16_t)(this->elifBlocks->vec.size() + 1);

            uint16_t endIf = function.labelInnerId;
            function.labelInnerId++;

            auto GenIfBytecode = [&](ConditionalNode* block)
            {
                block->predicate->GenerateBytecode(out, function);
                Write(out, OPCODE::JUMP_IF_FALSE);
                Write(out, labelId);
                block->body->GenerateBytecode(out, function);
                Write(out, OPCODE::JUMP);
                Write(out, endIf);
                Write(out, OPCODE::SET_LABEL);
                Write(out, labelId);
            };

            GenIfBytecode(this->ifBlock);
            labelId++;

            for (size_t i = 0; i < this->elifBlocks->vec.size(); i++)
            {
                auto elifBlock = static_cast<ConditionalNode*>(elifBlocks->vec[i]);
                GenIfBytecode(elifBlock);
                labelId++;
            }

            if (this->elseBlock != nullptr)
            {
                this->elseBlock->body->GenerateBytecode(out, function);
                Write(out, OPCODE::JUMP);
                Write(out, endIf);
            }
            Write(out, OPCODE::SET_LABEL);
            Write(out, endIf);
        }

        void ConditionalNode::DebugPrint(int offset, const char* delim) const
        {
            if (this->predicate != nullptr)
            {
                if (delim == "a")
                {
                    OFFSET_PRINT("if(");
                }
                else
                {
                    std::cout << '\n';
                    OFFSET_PRINT("elif(");
                }
                this->predicate->DebugPrint(0, delim);
                std::cout << ")\n";
            }
            else
            {
                std::cout << '\n';
                OFFSET_PRINT("else\n");
            }
            OFFSET_PRINT("{\n");
            this->body->DebugPrint(offset + 2, ";\n");
            if (dynamic_cast<AstNodeList*>(this->body) == nullptr)
                std::cout << ";\n";
            OFFSET_PRINT("}");
        }

        void ConditionalNode::Destroy()
        {
            if (this->predicate != nullptr)
            {
                FREE_AST(predicate)
            }
            FREE_AST(body)
        }

        void ConditionalNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            throw std::exception("Conditional node does not implement GenerateBytecode");
        }

        void TryCatchNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("try\n");
            OFFSET_PRINT("{\n");
            this->tryExpr->DebugPrint(offset + 2, ";\n");
            if (dynamic_cast<AstNodeList*>(this->tryExpr) == nullptr)
                std::cout << ";\n";
            if (catchExpr != nullptr)
            {
                if (!this->excName.empty())
                {
                    OFFSET_PRINT("catch(" << excName << ")\n");                    
                }
                else
                {
                    OFFSET_PRINT("catch\n");
                    OFFSET_PRINT("{\n");
                }
                this->catchExpr->DebugPrint(offset + 2, ";\n");
                if (dynamic_cast<AstNodeList*>(this->catchExpr) == nullptr)
                    std::cout << ";\n";
                OFFSET_PRINT("}");
            }
        }

        void TryCatchNode::Destroy()
        {
            FREE_AST(tryExpr)
            if (this->catchExpr != nullptr)
            {
                FREE_AST(catchExpr)
            }
        }

        void TryCatchNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            uint16_t labelId = function.labelInnerId;
            function.labelInnerId += 2; // for catch block and end of catch block

            // try { ... }
            Write(out, OPCODE::PUSH_CATCH);
            Write(out, labelId);
            this->tryExpr->GenerateBytecode(out, function);
            Write(out, OPCODE::POP_CATCH);
            Write(out, OPCODE::JUMP);
            Write<uint16_t>(out, labelId + 1);

            Write(out, OPCODE::SET_LABEL);
            Write(out, labelId);

            if (!this->excName.empty())
            {
                function.InsertDependency(excName);
                function.InsertDependency("System");
                function.InsertDependency("Exception");
                function.InsertDependency("Instance_0");

                // [variable] = System.Exception.Instance();
                Write(out, OPCODE::ALLOC_CONST_VAR);
                Write(out, function.GetHash(excName));
                Write(out, OPCODE::PUSH_OBJECT);
                Write(out, function.GetHash("System"));
                Write(out, OPCODE::GET_MEMBER);
                Write(out, function.GetHash("Exception"));
                Write(out, OPCODE::CALL_FUNCTION);
                Write(out, function.GetHash("Instance_0"));
                Write(out, (uint8_t)0);
                Write(out, OPCODE::ASSIGN_OP);
                Write(out, OPCODE::POP_STACK_TOP);
            }

            if (this->catchExpr != nullptr)
            {
                this->catchExpr->GenerateBytecode(out, function);
            }
            Write(out, OPCODE::SET_LABEL);
            Write<uint16_t>(out, labelId + 1);
        }

        void UnaryExprNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("(" << Token::ToString(this->op));
            this->expr->DebugPrint(0, delim);
            std::cout << ")";
        }

        void UnaryExprNode::Destroy()
        {
            FREE_AST(expr)
        }

        void UnaryExprNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            this->expr->GenerateBytecode(out, function);
            switch (this->op)
            {
            case Token::Type::NEGATION_OP:
                Write(out, OPCODE::NEGATION_OP);
                break;
            case Token::Type::NEGATIVE_OP:
                Write(out, OPCODE::NEGATIVE_OP);
                break;
            case Token::Type::POSITIVE_OP:
                Write(out, OPCODE::POSITIVE_OP);
                break;
            default:
                Write(out, OPCODE::ERROR_SYMBOL);
                break;
            }
        }

        void AttributeDeclNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("");
            this->modifiers->DebugPrint(0, " ");
            std::cout << "var " << this->name << ";\n";
        }

        void AttributeDeclNode::Destroy()
        {
            FREE_AST(modifiers);
        }

        void AttributeDeclNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            throw std::exception("AttributeDecl node does not implement GenerateBytecode");
        }

        void ClassDeclNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("");
            this->modifiers->DebugPrint(0, " ");
            std::cout << "class " << this->name << '\n';
            OFFSET_PRINT("{\n");
            this->members->DebugPrint(offset + 2, "\n");
            OFFSET_PRINT("}\n");
        }

        void ClassDeclNode::Destroy()
        {
            FREE_AST(members);
            FREE_AST(modifiers);
        }

        void ClassDeclNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            throw std::exception("ClassDecl node does not implement GenerateBytecode");
        }

        void UsingNamespaceNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("using namespace " << this->namespaceName << ';');
        }

        void UsingNamespaceNode::Destroy() { }

        void UsingNamespaceNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            throw std::exception("UsingNamespace node does not implement GenerateBytecode");
        }

        void NamespaceDeclNode::DebugPrint(int offset, const char* delim) const
        {
            OFFSET_PRINT("namespace " << this->name << '\n');
            OFFSET_PRINT("{\n");
            this->members->DebugPrint(offset + 2, "\n\n");
            OFFSET_PRINT("}");
        }

        void NamespaceDeclNode::Destroy()
        {
            FREE_AST(members);
        }

        void NamespaceDeclNode::GenerateBytecode(std::ostream& out, Method& function)
        {
            throw std::exception("NamespaceDecl node does not implement GenerateBytecode");
        }
}
}