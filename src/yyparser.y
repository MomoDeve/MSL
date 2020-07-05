%{
#include <iostream>
#include "lexer.h"
#include "expressions.h"

using namespace MSL::compiler;
using Type = Token::Type;

#define YYTOKENTYPE
typedef Token::Type yytokentype;

#define YYSTYPE_IS_DECLARED
struct yy_ast_node
{
    std::string str;
    Token* token;
    BaseAstNode* expr;
};
typedef yy_ast_node YYSTYPE;

static Lexer* yy_lexer;

void yyset_lexer(Lexer* lexer)
{
    yy_lexer = lexer;
}

extern YYSTYPE yylval;

int yylex()
{
    auto* token = &yy_lexer->Peek();
    yylval.token = token;
    //std::cout << token->value << ' ';
    yy_lexer->Next();
    return token->type;
}

void yyerror(BaseAstNode** root, const char* message)
{
    std::cout << "\n[Parser error]: " << message << std::endl;

    if(*root != nullptr) // destroy AST
    {
        (*root)->Destroy();
        delete *root;
        *root = nullptr;
    }
}
%}

%define parse.error verbose
%parse-param {BaseAstNode** AST}

%token ERROR
%token ENDLINE
%token APOS
%token COMMA
%token SEMICOLON
%token FOR
%token IF
%token ELSE
%token ELIF
%token WHILE
%token VARIABLE
%token FUNCTION
%token NAMESPACE
%token LAMBDA
%token FOREACH
%token IN
%token USING
%token TRY
%token CATCH
%token OBJECT
%token THIS
%token INTEGER_CONSTANT
%token FLOAT_CONSTANT
%token STRING_CONSTANT
%token TRUE_CONSTANT
%token FALSE_CONSTANT
%token NULLPTR
%token ROUND_BRACKET_O
%token ROUND_BRACKET_C
%token SQUARE_BRACKET_O
%token SQUARE_BRACKET_C
%token BRACE_BRACKET_O
%token BRACE_BRACKET_C
%token NEGATION_OP
%token NEGATIVE_OP
%token POSITIVE_OP
%token RETURN
%token DOT              
%token ASSIGN_OP         
%token SUM_ASSIGN_OP    
%token SUB_ASSIGN_OP     
%token MULT_ASSIGN_OP    
%token DIV_ASSIGN_OP     
%token MOD_ASSIGN_OP   
%token POWER_ASSIGN_OP
%token LOGIC_EQUALS     
%token LOGIC_NOT_EQUALS  
%token LOGIC_LESS      
%token LOGIC_GREATER  
%token LOGIC_LESS_EQUALS
%token LOGIC_GREATER_EQUALS
%token LOGIC_OR          
%token LOGIC_AND         
%token SUM_OP            
%token SUB_OP            
%token MULT_OP           
%token DIV_OP            
%token MOD_OP           
%token POWER_OP         
%token CONST
%token PUBLIC
%token PRIVATE
%token STATIC
%token CLASS

%right ASSIGN_OP SUM_ASSIGN_OP SUB_ASSIGN_OP MULT_ASSIGN_OP DIV_ASSIGN_OP MOD_ASSIGN_OP POWER_ASSIGN_OP
%left LOGIC_OR
%left LOGIC_AND
%left LOGIC_EQUALS LOGIC_NOT_EQUALS
%left LOGIC_LESS LOGIC_GREATER LOGIC_LESS_EQUALS LOGIC_GREATER_EQUALS
%left SUM_OP SUB_OP
%left MULT_OP DIV_OP MOD_OP
%left POWER_OP
%left NEGATION_OP POSITIVE_OP NEGATIVE_OP
%left DOT

%type<token> OBJECT THIS TRUE_CONSTANT FALSE_CONSTANT INTEGER_CONSTANT FLOAT_CONSTANT STRING_CONSTANT NULLPTR
%type<expr> VALUE INDEX_CALL STATEMENT METHOD_CALL METHOD_ARGUMENT_LIST BINARY_STATEMENT UNARY_STATEMENT RETURN_STATEMENT
%type<expr> VARIABLE_DECL CONST_VARIABLE_DECL VARIABLE_NAME_LIST ARGUMENT_LIST_DECL
%type<expr> INNER_EXPRESSION_BLOCK EXPRESSION_BLOCK EXPRESSION_LIST EXPRESSION
%type<expr> IF_STATEMENT IF_BLOCK ELIF_BLOCK ELIF_LIST ELSE_BLOCK WHILE_STATEMENT FOREACH_STATEMENT TRY_STATEMENT
%type<expr> OPTIONAL_FOR_INIT OPTIONAL_FOR_PRED OPTIONAL_FOR_ITER FOR_STATEMENT
%type<expr> METHOD_DECL ATTRIBUTE_DECL MEMBER_MODIFIER_LIST CLASS_MODIFIER_LIST CLASS_DECL MEMBER_LIST
%type<expr> NAMESPACE_LIST NAMESPACE_DECL NAMESPACE_MEMBER_LIST USING_STATEMENT

%right IF ELIF ELSE

%%
NAMESPACE_LIST:
    {
        *AST = new AstNodeList();
        $$ = *AST;
    }
    |
    NAMESPACE_LIST NAMESPACE_DECL
    {
        auto* namespaces = static_cast<AstNodeList*>($1);
        namespaces->vec.push_back($2);
        $$ = $1;
    }

NAMESPACE_DECL:
    NAMESPACE OBJECT BRACE_BRACKET_O NAMESPACE_MEMBER_LIST BRACE_BRACKET_C
    {
        $$ = new NamespaceDeclNode($2->value, static_cast<AstNodeList*>($4));
    }

NAMESPACE_MEMBER_LIST:
    {
        $$ = new AstNodeList();
    }
    |
    NAMESPACE_MEMBER_LIST CLASS_DECL
    {
        auto* nsList = static_cast<AstNodeList*>($1);
        nsList->vec.push_back($2);
        $$ = $1;
    }
    |
    NAMESPACE_MEMBER_LIST USING_STATEMENT
    {
        auto* nsList = static_cast<AstNodeList*>($1);
        nsList->vec.push_back($2);
        $$ = $1;
    }

USING_STATEMENT:
    USING NAMESPACE OBJECT SEMICOLON
    {
        $$ = new UsingNamespaceNode($3->value);
    }

CLASS_DECL:
    CLASS_MODIFIER_LIST CLASS OBJECT BRACE_BRACKET_O MEMBER_LIST BRACE_BRACKET_C
    {
        auto* classNode = new ClassDeclNode($3->value);
        classNode->modifiers = static_cast<StringList*>($1);
        classNode->members = static_cast<AstNodeList*>($5);
        $$ = classNode;
    }

CLASS_MODIFIER_LIST:
    {
        $$ = new StringList();
    }
    |
    MEMBER_MODIFIER_LIST PUBLIC
    {
        auto* modifiers = static_cast<StringList*>($1);
        modifiers->vec.push_back("public");
        $$ = modifiers;
    }
    |
    MEMBER_MODIFIER_LIST PRIVATE
    {
        auto* modifiers = static_cast<StringList*>($1);
        modifiers->vec.push_back("private");
        $$ = modifiers;
    }
    |
    MEMBER_MODIFIER_LIST STATIC
    {
        auto* modifiers = static_cast<StringList*>($1);
        modifiers->vec.push_back("static");
        $$ = modifiers;
    }

MEMBER_LIST:
    {
        $$ = new AstNodeList();
    }
    |
    MEMBER_LIST MEMBER_MODIFIER_LIST METHOD_DECL
    {
        auto* members = static_cast<AstNodeList*>($1);
        auto* modifiers = static_cast<StringList*>($2);
        auto* method = static_cast<MethodDeclNode*>($3);
        method->modifiers = modifiers;
        members->vec.push_back(method);
        $$ = members;
    }
    |
    MEMBER_LIST MEMBER_MODIFIER_LIST ATTRIBUTE_DECL
    {
        auto* members = static_cast<AstNodeList*>($1);
        auto* modifiers = static_cast<StringList*>($2);
        auto* attribute = static_cast<AttributeDeclNode*>($3);
        attribute->modifiers = modifiers;
        members->vec.push_back(attribute);
        $$ = members;
    }

ATTRIBUTE_DECL:
    VARIABLE OBJECT SEMICOLON
    {
        $$ = new AttributeDeclNode($2->value);
    }

MEMBER_MODIFIER_LIST:
    {
        $$ = new StringList();
    }
    |
    MEMBER_MODIFIER_LIST PUBLIC
    {
        auto* modifiers = static_cast<StringList*>($1);
        modifiers->vec.push_back("public");
        $$ = modifiers;
    }
    |
    MEMBER_MODIFIER_LIST PRIVATE
    {
        auto* modifiers = static_cast<StringList*>($1);
        modifiers->vec.push_back("private");
        $$ = modifiers;
    }
    |
    MEMBER_MODIFIER_LIST STATIC
    {
        auto* modifiers = static_cast<StringList*>($1);
        modifiers->vec.push_back("static");
        $$ = modifiers;
    }
    |
    MEMBER_MODIFIER_LIST CONST
    {
        auto* modifiers = static_cast<StringList*>($1);
        modifiers->vec.push_back("const");
        $$ = modifiers;
    }

METHOD_DECL:
    FUNCTION OBJECT ARGUMENT_LIST_DECL SEMICOLON
    {
        $$ = new MethodDeclNode($2->value, static_cast<StringList*>($3), nullptr);
    }
    |
    FUNCTION OBJECT ARGUMENT_LIST_DECL EXPRESSION_BLOCK
    {
        $$ = new MethodDeclNode($2->value, static_cast<StringList*>($3), static_cast<AstNodeList*>($4));
    }

ARGUMENT_LIST_DECL:
    ROUND_BRACKET_O VARIABLE_NAME_LIST ROUND_BRACKET_C
    {
        $$ = $2;
    }

VARIABLE_NAME_LIST:
    {
        $$ = new StringList();
    }
    |
    OBJECT
    {
        auto* strs = new StringList();
        strs->vec.push_back($1->value);
        $$ = strs;
    }
    |
    VARIABLE_NAME_LIST COMMA OBJECT
    {
        auto* strs = static_cast<StringList*>($1);
        strs->vec.push_back($3->value);
        $$ = strs;
    }

EXPRESSION_BLOCK:
    BRACE_BRACKET_O EXPRESSION_LIST BRACE_BRACKET_C
    {
        $$ = $2;
    }

EXPRESSION_LIST:
    {
        $$ = new AstNodeList();
    }
    |
    EXPRESSION_LIST EXPRESSION
    {
        auto* exprs = static_cast<AstNodeList*>($1);
        exprs->vec.push_back($2);
        $$ = exprs;
    }

EXPRESSION:
    IF_STATEMENT
    {
        $$ = $1;
    }
    |
    FOR_STATEMENT
    {
        $$ = $1;
    }
    |
    WHILE_STATEMENT
    {
        $$ = $1;
    }
    |
    FOREACH_STATEMENT
    {
        $$ = $1;
    }
    |
    TRY_STATEMENT
    {
        $$ = $1;
    }
    |
    RETURN_STATEMENT SEMICOLON
    {
        $$ = $1;
    }
    |
    VARIABLE_DECL SEMICOLON
    {
        $$ = $1;
    }
    |
    CONST_VARIABLE_DECL SEMICOLON
    {
        $$ = $1;
    }
    |
    STATEMENT SEMICOLON
    {
        $$ = $1;
    }

STATEMENT: 
    VALUE
    {
        $$ = $1;
    }
    |
    ROUND_BRACKET_O STATEMENT ROUND_BRACKET_C
    {
        $$ = $2;
    }
    |
    BINARY_STATEMENT
    {
        $$ = $1;
    }
    |
    UNARY_STATEMENT
    {
        $$ = $1;
    }

VALUE:
    METHOD_CALL
    {
        $$ = $1;
    }
    |
    VALUE DOT METHOD_CALL
    {
        auto* call = static_cast<MethodCallNode*>($3);
        call->parent = $1;
        $$ = call;
    }
    |
    INTEGER_CONSTANT
    {
        $$ = new ObjectNode($1->value, $1->type);
    }
    | 
    STRING_CONSTANT
    {
        $$ = new ObjectNode($1->value, $1->type);
    }
    | 
    FLOAT_CONSTANT
    {
       $$ = new ObjectNode($1->value, $1->type);
    }
    | 
    NULLPTR
    {
        $$ = new ObjectNode($1->value, $1->type);
    }
    |
    THIS
    {
        $$ = new ObjectNode($1->value, $1->type);
    }
    |
    OBJECT
    {
        $$ = new ObjectNode($1->value, $1->type);
    }
    |
    TRUE_CONSTANT
    {
        $$ = new ObjectNode($1->value, $1->type);
    }
    |
    FALSE_CONSTANT
    {
        $$ = new ObjectNode($1->value, $1->type);
    }
    |
    VALUE DOT OBJECT
    {
        $$ = new MemberCallNode($1, $3->value);
    }
    |
    INDEX_CALL
    {
        $$ = $1;
    }

INDEX_CALL:
    VALUE SQUARE_BRACKET_O STATEMENT SQUARE_BRACKET_C
    {
        $$ = new IndexCallNode($1, $3);
    }

METHOD_CALL:
    OBJECT ROUND_BRACKET_O ROUND_BRACKET_C
    {
        $<expr>$ = new MethodCallNode($1->value, nullptr, new AstNodeList());
    }
    |
    OBJECT ROUND_BRACKET_O METHOD_ARGUMENT_LIST ROUND_BRACKET_C
    {
        $<expr>$ = new MethodCallNode($1->value, nullptr, static_cast<AstNodeList*>($3));
    }

METHOD_ARGUMENT_LIST:
    STATEMENT
    {
        auto* args = new AstNodeList();
        args->vec.push_back($1);
        $$ = args;
    }
    |
    METHOD_ARGUMENT_LIST COMMA STATEMENT
    {
        auto* args = static_cast<AstNodeList*>($1);
        args->vec.push_back($3);
        $$ = args;
    }

BINARY_STATEMENT:
    STATEMENT SUM_OP STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::SUM_OP);
    }
    | 
    STATEMENT SUB_OP STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::SUB_OP);
    }
    | 
    STATEMENT MULT_OP STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::MULT_OP);
    }
    | 
    STATEMENT DIV_OP STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::DIV_OP);
    }
    | 
    STATEMENT MOD_OP STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::MOD_OP);
    }
    | 
    STATEMENT POWER_OP STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::POWER_OP);
    }
    |
    STATEMENT LOGIC_EQUALS STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_EQUALS);
    }
    |
    STATEMENT LOGIC_NOT_EQUALS STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_NOT_EQUALS);
    }
    |
    STATEMENT LOGIC_LESS STATEMENT   
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_LESS);
    }
    |
    STATEMENT LOGIC_GREATER STATEMENT  
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_GREATER);
    }
    |
    STATEMENT LOGIC_LESS_EQUALS STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_LESS_EQUALS);
    }
    |
    STATEMENT LOGIC_GREATER_EQUALS STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_GREATER_EQUALS);
    }
    |
    STATEMENT LOGIC_OR STATEMENT    
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_OR);
    }
    |
    STATEMENT LOGIC_AND STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::LOGIC_AND);
    }
    |
    STATEMENT ASSIGN_OP STATEMENT
    {
        $$ = new BinaryExprNode($1, $3, Type::ASSIGN_OP);
    }
    |
    STATEMENT SUM_ASSIGN_OP STATEMENT
    {
        auto* expr = new BinaryExprNode($1, $3, Type::SUM_OP);
        $$ = new BinaryExprNode($1, expr, Type::ASSIGN_OP);
    }
    |
    STATEMENT SUB_ASSIGN_OP STATEMENT
    {
        auto* expr = new BinaryExprNode($1, $3, Type::SUB_OP);
        $$ = new BinaryExprNode($1, expr, Type::ASSIGN_OP);
    }
    |
    STATEMENT MULT_ASSIGN_OP STATEMENT
    {
        auto* expr = new BinaryExprNode($1, $3, Type::MULT_OP);
        $$ = new BinaryExprNode($1, expr, Type::ASSIGN_OP);
    }
    |
    STATEMENT DIV_ASSIGN_OP STATEMENT
    {
        auto* expr = new BinaryExprNode($1, $3, Type::DIV_OP);
        $$ = new BinaryExprNode($1, expr, Type::ASSIGN_OP);
    }
    |
    STATEMENT MOD_ASSIGN_OP STATEMENT
    {
        auto* expr = new BinaryExprNode($1, $3, Type::MOD_OP);
        $$ = new BinaryExprNode($1, expr, Type::ASSIGN_OP);
    }
    |
    STATEMENT POWER_ASSIGN_OP STATEMENT
    {
        auto* expr = new BinaryExprNode($1, $3, Type::POWER_OP);
        $$ = new BinaryExprNode($1, expr, Type::ASSIGN_OP);
    }

UNARY_STATEMENT:
    NEGATION_OP STATEMENT
    {
        $$ = new UnaryExprNode($2, Type::NEGATION_OP);
    }
    |
    SUM_OP STATEMENT %prec POSITIVE_OP
    {
        auto* expr = new UnaryExprNode($2, Type::POSITIVE_OP);
    }
    |
    SUB_OP STATEMENT %prec NEGATIVE_OP
    {
        auto* expr = new UnaryExprNode($2, Type::NEGATIVE_OP);
    }

RETURN_STATEMENT:
    RETURN
    {
        $$ = new ReturnExprNode(new ObjectNode("null", Token::Type::NULLPTR));
        // maybe check if function is a constructor?
    }
    |
    RETURN STATEMENT
    {
         $$ = new ReturnExprNode($2);
    }

VARIABLE_DECL:
    VARIABLE OBJECT
    {
        $$ = new VariableDeclNode($2->value, false, nullptr);
    }
    |
    VARIABLE OBJECT ASSIGN_OP STATEMENT
    {
        $$ = new VariableDeclNode($2->value, false, $4);
    }

CONST_VARIABLE_DECL:
    CONST VARIABLE OBJECT
    {
        $$ = new VariableDeclNode($3->value, true, new ObjectNode("null", Token::Type::NULLPTR));
    }
    |
    CONST VARIABLE OBJECT ASSIGN_OP STATEMENT
    {
        $$ = new VariableDeclNode($3->value, true, $5);
    }

FOR_STATEMENT:
    FOR ROUND_BRACKET_O OPTIONAL_FOR_INIT SEMICOLON OPTIONAL_FOR_PRED SEMICOLON OPTIONAL_FOR_ITER ROUND_BRACKET_C INNER_EXPRESSION_BLOCK
    {
        $$ = new ForExprNode($3, $5, $7, $9);
    }

OPTIONAL_FOR_INIT:
    {
        $$ = nullptr;
    }
    |
    VARIABLE_DECL
    {
        $$ = $1;
    }
    |
    STATEMENT
    {
        $$ = $1;
    }

OPTIONAL_FOR_PRED:
    {
        $$ = nullptr;
    }
    |
    STATEMENT
    ;

OPTIONAL_FOR_ITER:
    {
        $$ = nullptr;
    }
    |
    STATEMENT
    {
        $$ = $1;
    }

IF_STATEMENT:
    IF_BLOCK %prec IF
    {
        $$ = new IfStatementNode(
            static_cast<ConditionalNode*>($1), 
            new AstNodeList(), 
            nullptr
        );
    }
    |
    IF_BLOCK ELIF_LIST %prec ELIF
    {
        $$ = new IfStatementNode(
            static_cast<ConditionalNode*>($1), 
            static_cast<AstNodeList*>($2), 
            nullptr
        );
    }
    |
    IF_BLOCK ELIF_LIST ELSE_BLOCK
    {
        $$ = new IfStatementNode(
            static_cast<ConditionalNode*>($1), 
            static_cast<AstNodeList*>($2), 
            static_cast<ConditionalNode*>($3)
        );
    }
    |
    IF_BLOCK ELSE_BLOCK
    {
        $$ = new IfStatementNode(
            static_cast<ConditionalNode*>($1), 
            new AstNodeList(), 
            static_cast<ConditionalNode*>($2)
        );
    }

IF_BLOCK:
    IF ROUND_BRACKET_O STATEMENT ROUND_BRACKET_C INNER_EXPRESSION_BLOCK
    {
        $$ = new ConditionalNode($3, $5);
    }
    
ELIF_LIST:
    ELIF_BLOCK
    {
        auto* elifs = new AstNodeList();
        elifs->vec.push_back($1);
    }
    |
    ELIF_LIST ELIF_BLOCK
    {
        auto* elifs = static_cast<AstNodeList*>($1);
        elifs->vec.push_back($2);
    }

ELIF_BLOCK:
    ELIF ROUND_BRACKET_O STATEMENT ROUND_BRACKET_C INNER_EXPRESSION_BLOCK
    {
        $$ = new ConditionalNode($3, $5);
    }

ELSE_BLOCK:
    ELSE INNER_EXPRESSION_BLOCK
    {
        $$ = new ConditionalNode(nullptr, $2);
    }

INNER_EXPRESSION_BLOCK:
    EXPRESSION
    {
        auto* exprs = new AstNodeList();
        exprs->vec.push_back($1);
        $$ = exprs;
    }
    |
    EXPRESSION_BLOCK
    {
        $$ = $1;
    }

WHILE_STATEMENT:
    WHILE ROUND_BRACKET_O STATEMENT ROUND_BRACKET_C INNER_EXPRESSION_BLOCK
    {
        $$ = new ForExprNode(nullptr, $3, nullptr, $5);
        // init and iter are empty in while statement
    }

FOREACH_STATEMENT:
    FOREACH ROUND_BRACKET_O VARIABLE OBJECT IN STATEMENT ROUND_BRACKET_C INNER_EXPRESSION_BLOCK
    {
        $$ = new ForeachExprNode($4->value, $6, $8);
    }

TRY_STATEMENT:
    TRY EXPRESSION_BLOCK
    {
        $$ = new TryCatchNode("", $2, nullptr);
    }
    |
    TRY EXPRESSION_BLOCK CATCH EXPRESSION_BLOCK
    {
        $$ = new TryCatchNode("", $2, $4);
    }
    |
    TRY EXPRESSION_BLOCK CATCH ROUND_BRACKET_O OBJECT ROUND_BRACKET_C EXPRESSION_BLOCK
    {
        $$ = new TryCatchNode($5->value, $2, $7);
    }
%%