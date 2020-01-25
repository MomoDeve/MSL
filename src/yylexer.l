%{
#include "token.h"

using namespace MSL::compiler;
using Type = Token::Type;

static Type prev = Type::ENDOFFILE;
void yy_set_last(Type type)
{
    prev = type;
}

Type yy_get_last()
{
    return prev;
}

bool fits_unary(Type prev)
{
    return prev == Type::ROUND_BRACKET_O || prev == Type::SQUARE_BRACKET_O ||
		   prev == Type::BRACE_BRACKET_O || prev == Type::ENDOFFILE ||
		   (prev & Type::BINARY_OPERAND) || (prev & Type::UNARY_OPERAND) ||
		   prev == Type::SEMICOLON       || prev == Type::COMMA || prev == Type::ENDLINE;
}

#define yyterminate() return Type::ENDOFFILE;
#define YY_DECL Token yy_lex()
#define YY_SKIP_YYWRAP
int yywrap() { return 0; }
%}

%%
[ \t\r]   {                                    }
__EOF__   { return Type::ENDOFFILE;            }
\n        { return Type::ENDLINE;      	   	   }
\.        { return Type::DOT;          		   }
'         { return Type::APOS;				   }
,         { return Type::COMMA;				   }
;         { return Type::SEMICOLON;			   }
\(        { return Type::ROUND_BRACKET_O;	   }
\)        { return Type::ROUND_BRACKET_C;	   }
\[        { return Type::SQUARE_BRACKET_O;	   }
\]        { return Type::SQUARE_BRACKET_C;	   }
\{        { return Type::BRACE_BRACKET_O;	   }
\}        { return Type::BRACE_BRACKET_C;	   }
=         { return Type::ASSIGN_OP;			   }
!         { return Type::NEGATION_OP;		   }
\*        { return Type::MULT_OP;			   }
\/        { return Type::DIV_OP;			   }
\%        { return Type::MOD_OP;			   }
&&        { return Type::LOGIC_AND;			   }
\|\|      { return Type::LOGIC_OR;			   }
==        { return Type::LOGIC_EQUALS;		   }
!=        { return Type::LOGIC_NOT_EQUALS;	   }
\<        { return Type::LOGIC_LESS;		   }
\>        { return Type::LOGIC_GREATER;		   }
\<=       { return Type::LOGIC_LESS_EQUALS;    }
\>=       { return Type::LOGIC_GREATER_EQUALS; }
\*\*      { return Type::POWER_OP;			   }
\+=       { return Type::SUM_ASSIGN_OP;		   }
\-=       { return Type::SUB_ASSIGN_OP;		   }
\*=       { return Type::MULT_ASSIGN_OP;	   }
\/=       { return Type::DIV_ASSIGN_OP;		   }
\%=       { return Type::MOD_ASSIGN_OP;		   }
class     { return Type::CLASS;				   }
for       { return Type::FOR;				   }
if        { return Type::IF;				   }
else      { return Type::ELSE;				   }
elif      { return Type::ELIF;				   }
while     { return Type::WHILE;				   }
static    { return Type::STATIC;			   }
var       { return Type::VARIABLE;			   }
function  { return Type::FUNCTION;			   }
const     { return Type::CONST;				   }
public    { return Type::PUBLIC;			   }
private   { return Type::PRIVATE;			   }
internal  { return Type::INTERNAL;			   }
abstract  { return Type::ABSTRACT;			   }
namespace { return Type::NAMESPACE;			   }
return    { return Type::RETURN;			   }
lambda    { return Type::LAMBDA;			   }
this      { return Type::THIS;				   }
in        { return Type::IN;				   }
foreach   { return Type::FOREACH;			   }
true      { return Type::TRUE_CONSTANT;		   }
false     { return Type::FALSE_CONSTANT;	   }
null      { return Type::NULLPTR;			   }
using     { return Type::USING;				   }
try       { return Type::TRY;				   }
catch     { return Type::CATCH;				   }

\+        { return fits_unary(yy_get_last()) ? Type::POSITIVE_OP : Type::SUM_OP; }
\-        { return fits_unary(yy_get_last()) ? Type::NEGATIVE_OP : Type::SUB_OP; }

[0-9]+                      { return { Type::INTEGER_CONSTANT, yytext }; }
[0-9]+\.[0-9]+              { return { Type::FLOAT_CONSTANT,   yytext }; }
__USER_DEFINED_STRING[0-9]+ { return { Type::STRING_CONSTANT,  yytext }; }
[a-zA-Z][a-zA-Z0-9]*        { return { Type::OBJECT,           yytext }; }
.                           { return { Type::ERROR,            yytext }; }
%%