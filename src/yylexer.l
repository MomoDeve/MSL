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

#define yyterminate() return Type::ENDOFFILE;
#define YY_DECL Token yy_lex()
#define YY_SKIP_YYWRAP
int yywrap() { return 0; }
%}

%%
[ \t\r]   {                                                }
__EOF__   { return { Type::ENDOFFILE, "__EOF__" };         }
\n        { return { Type::ENDLINE, yytext };              }
\.        { return { Type::DOT, yytext };                  }
'         { return { Type::APOS, yytext };                 }
,         { return { Type::COMMA, yytext };                }
;         { return { Type::SEMICOLON, yytext };            }
\(        { return { Type::ROUND_BRACKET_O, yytext };      }  
\)        { return { Type::ROUND_BRACKET_C, yytext };      }
\[        { return { Type::SQUARE_BRACKET_O, yytext };     }
\]        { return { Type::SQUARE_BRACKET_C, yytext };     }
\{        { return { Type::BRACE_BRACKET_O, yytext };      }
\}        { return { Type::BRACE_BRACKET_C, yytext };      }
=         { return { Type::ASSIGN_OP, yytext };            }
!         { return { Type::NEGATION_OP, yytext };          }  
\+        { return { Type::SUM_OP, yytext };               }
\-        { return { Type::SUB_OP, yytext };               }
\*        { return { Type::MULT_OP, yytext };              }  
\/        { return { Type::DIV_OP, yytext };               }
\%        { return { Type::MOD_OP, yytext };               }
&&        { return { Type::LOGIC_AND, yytext };            }
\|\|      { return { Type::LOGIC_OR, yytext };             }
==        { return { Type::LOGIC_EQUALS, yytext };         }
!=        { return { Type::LOGIC_NOT_EQUALS, yytext };     }
\<        { return { Type::LOGIC_LESS, yytext };           }
\>        { return { Type::LOGIC_GREATER, yytext };        }
\<=       { return { Type::LOGIC_LESS_EQUALS, yytext };    }
\>=       { return { Type::LOGIC_GREATER_EQUALS, yytext }; }
\*\*      { return { Type::POWER_OP, yytext };             }
\+=       { return { Type::SUM_ASSIGN_OP, yytext };        }
\-=       { return { Type::SUB_ASSIGN_OP, yytext };        }
\*=       { return { Type::MULT_ASSIGN_OP, yytext };       }
\/=       { return { Type::DIV_ASSIGN_OP, yytext };        }
\%=       { return { Type::MOD_ASSIGN_OP, yytext };        }
\*\*=     { return { Type::POWER_ASSIGN_OP, yytext };      }
class     { return { Type::CLASS, yytext };                }
for       { return { Type::FOR, yytext };                  }  
if        { return { Type::IF, yytext };                   }
else      { return { Type::ELSE, yytext };                 }
elif      { return { Type::ELIF, yytext };                 }
while     { return { Type::WHILE, yytext };                }
static    { return { Type::STATIC, yytext };               }
var       { return { Type::VARIABLE, yytext };             }
function  { return { Type::FUNCTION, yytext };             }
const     { return { Type::CONST, yytext };                }
public    { return { Type::PUBLIC, yytext };               }
private   { return { Type::PRIVATE, yytext };              }  
namespace { return { Type::NAMESPACE, yytext };            }
return    { return { Type::RETURN, yytext };               }
lambda    { return { Type::LAMBDA, yytext };               }
this      { return { Type::THIS, yytext };                 }
in        { return { Type::IN, yytext };                   }
foreach   { return { Type::FOREACH, yytext };              }
true      { return { Type::TRUE_CONSTANT, yytext };        }
false     { return { Type::FALSE_CONSTANT, yytext };       }
null      { return { Type::NULLPTR, yytext };              }
using     { return { Type::USING, yytext };                }
try       { return { Type::TRY, yytext };                  }
catch     { return { Type::CATCH, yytext };                }
[0-9]+                      { return { Type::INTEGER_CONSTANT, yytext }; }
[0-9]+\.[0-9]+              { return { Type::FLOAT_CONSTANT,   yytext }; }
__USER_DEFINED_STRING[0-9]+ { return { Type::STRING_CONSTANT,  yytext }; }
[a-zA-Z][a-zA-Z0-9]*        { return { Type::OBJECT,           yytext }; }
.                           { return { Type::ERROR,            yytext }; }
%%