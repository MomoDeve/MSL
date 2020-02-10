/* A Bison parser, made by GNU Bison 3.5.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "yyparser.y"

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

#line 121 "yyparser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ERROR = 258,
    ENDLINE = 259,
    APOS = 260,
    COMMA = 261,
    SEMICOLON = 262,
    FOR = 263,
    IF = 264,
    ELSE = 265,
    ELIF = 266,
    WHILE = 267,
    VARIABLE = 268,
    FUNCTION = 269,
    NAMESPACE = 270,
    LAMBDA = 271,
    FOREACH = 272,
    IN = 273,
    USING = 274,
    TRY = 275,
    CATCH = 276,
    OBJECT = 277,
    THIS = 278,
    INTEGER_CONSTANT = 279,
    FLOAT_CONSTANT = 280,
    STRING_CONSTANT = 281,
    TRUE_CONSTANT = 282,
    FALSE_CONSTANT = 283,
    NULLPTR = 284,
    ROUND_BRACKET_O = 285,
    ROUND_BRACKET_C = 286,
    SQUARE_BRACKET_O = 287,
    SQUARE_BRACKET_C = 288,
    BRACE_BRACKET_O = 289,
    BRACE_BRACKET_C = 290,
    NEGATION_OP = 291,
    NEGATIVE_OP = 292,
    POSITIVE_OP = 293,
    RETURN = 294,
    DOT = 295,
    ASSIGN_OP = 296,
    SUM_ASSIGN_OP = 297,
    SUB_ASSIGN_OP = 298,
    MULT_ASSIGN_OP = 299,
    DIV_ASSIGN_OP = 300,
    MOD_ASSIGN_OP = 301,
    POWER_ASSIGN_OP = 302,
    LOGIC_EQUALS = 303,
    LOGIC_NOT_EQUALS = 304,
    LOGIC_LESS = 305,
    LOGIC_GREATER = 306,
    LOGIC_LESS_EQUALS = 307,
    LOGIC_GREATER_EQUALS = 308,
    LOGIC_OR = 309,
    LOGIC_AND = 310,
    SUM_OP = 311,
    SUB_OP = 312,
    MULT_OP = 313,
    DIV_OP = 314,
    MOD_OP = 315,
    POWER_OP = 316,
    CONST = 317,
    PUBLIC = 318,
    PRIVATE = 319,
    STATIC = 320,
    CLASS = 321
  };
#endif

/* Value type.  */


extern YYSTYPE yylval;

int yyparse (BaseAstNode** AST);





#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   483

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  39
/* YYNRULES -- Number of rules.  */
#define YYNRULES  115
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  205

#define YYUNDEFTOK  2
#define YYMAXUTOK   321


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   144,   144,   149,   157,   163,   167,   174,   182,   188,
     197,   201,   208,   215,   223,   227,   237,   248,   254,   258,
     265,   272,   279,   287,   292,   298,   304,   308,   315,   323,
     329,   333,   341,   346,   351,   356,   361,   366,   371,   376,
     381,   387,   392,   397,   402,   408,   413,   420,   425,   430,
     435,   440,   445,   450,   455,   460,   465,   471,   477,   482,
     488,   495,   503,   508,   513,   518,   523,   528,   533,   538,
     543,   548,   553,   558,   563,   568,   573,   578,   584,   590,
     596,   602,   608,   615,   620,   625,   631,   637,   643,   648,
     654,   659,   665,   671,   675,   680,   686,   690,   694,   698,
     704,   713,   722,   731,   741,   747,   753,   760,   766,   772,
     779,   785,   792,   798,   803,   808
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "ERROR", "ENDLINE", "APOS", "COMMA",
  "SEMICOLON", "FOR", "IF", "ELSE", "ELIF", "WHILE", "VARIABLE",
  "FUNCTION", "NAMESPACE", "LAMBDA", "FOREACH", "IN", "USING", "TRY",
  "CATCH", "OBJECT", "THIS", "INTEGER_CONSTANT", "FLOAT_CONSTANT",
  "STRING_CONSTANT", "TRUE_CONSTANT", "FALSE_CONSTANT", "NULLPTR",
  "ROUND_BRACKET_O", "ROUND_BRACKET_C", "SQUARE_BRACKET_O",
  "SQUARE_BRACKET_C", "BRACE_BRACKET_O", "BRACE_BRACKET_C", "NEGATION_OP",
  "NEGATIVE_OP", "POSITIVE_OP", "RETURN", "DOT", "ASSIGN_OP",
  "SUM_ASSIGN_OP", "SUB_ASSIGN_OP", "MULT_ASSIGN_OP", "DIV_ASSIGN_OP",
  "MOD_ASSIGN_OP", "POWER_ASSIGN_OP", "LOGIC_EQUALS", "LOGIC_NOT_EQUALS",
  "LOGIC_LESS", "LOGIC_GREATER", "LOGIC_LESS_EQUALS",
  "LOGIC_GREATER_EQUALS", "LOGIC_OR", "LOGIC_AND", "SUM_OP", "SUB_OP",
  "MULT_OP", "DIV_OP", "MOD_OP", "POWER_OP", "CONST", "PUBLIC", "PRIVATE",
  "STATIC", "CLASS", "$accept", "NAMESPACE_LIST", "NAMESPACE_DECL",
  "NAMESPACE_MEMBER_LIST", "USING_STATEMENT", "CLASS_DECL",
  "CLASS_MODIFIER_LIST", "MEMBER_LIST", "ATTRIBUTE_DECL",
  "MEMBER_MODIFIER_LIST", "METHOD_DECL", "ARGUMENT_LIST_DECL",
  "VARIABLE_NAME_LIST", "EXPRESSION_BLOCK", "EXPRESSION_LIST",
  "EXPRESSION", "STATEMENT", "VALUE", "INDEX_CALL", "METHOD_CALL",
  "METHOD_ARGUMENT_LIST", "BINARY_STATEMENT", "UNARY_STATEMENT",
  "RETURN_STATEMENT", "VARIABLE_DECL", "CONST_VARIABLE_DECL",
  "FOR_STATEMENT", "OPTIONAL_FOR_INIT", "OPTIONAL_FOR_PRED",
  "OPTIONAL_FOR_ITER", "IF_STATEMENT", "IF_BLOCK", "ELIF_LIST",
  "ELIF_BLOCK", "ELSE_BLOCK", "INNER_EXPRESSION_BLOCK", "WHILE_STATEMENT",
  "FOREACH_STATEMENT", "TRY_STATEMENT", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321
};
# endif

#define YYPACT_NINF (-166)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-14)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -166,    16,  -166,    -8,  -166,   -17,  -166,    -1,     5,  -166,
    -166,  -166,   -44,    27,    14,    42,  -166,   -39,     3,    10,
      61,    43,  -166,  -166,    44,  -166,    -2,    73,    74,  -166,
    -166,  -166,  -166,  -166,    91,    77,  -166,    86,    -5,  -166,
       1,  -166,  -166,  -166,    88,  -166,    58,  -166,    81,    83,
      87,    94,    96,    84,    97,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,   203,  -166,   203,   203,   203,   203,   116,  -166,
      93,   -19,  -166,  -166,  -166,  -166,   123,   124,   125,  -166,
    -166,    62,  -166,  -166,  -166,   165,   203,   203,   120,   144,
     141,   188,   220,  -166,   394,  -166,  -166,   143,  -166,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   146,  -166,  -166,  -166,   147,   133,    62,  -166,  -166,
     394,  -166,   159,   251,   282,   203,   157,   -11,  -166,   394,
       2,  -166,   139,   394,   394,   394,   394,   394,   394,   394,
     196,   196,    45,    45,    45,    45,   408,   422,    64,    64,
     121,   121,   121,  -166,   373,    97,  -166,  -166,  -166,  -166,
     203,  -166,  -166,   203,   147,   147,   394,   166,   163,  -166,
     203,  -166,   203,  -166,   313,   394,   189,  -166,  -166,   203,
     167,   394,   394,   147,   203,   344,    84,  -166,   394,   168,
     147,  -166,   147,  -166,  -166
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     3,     0,     5,    18,     0,     4,
       7,     6,     0,     0,     0,     0,    22,    19,    20,    21,
       0,     0,     8,    14,    18,     9,     0,     0,     0,    19,
      20,    21,    16,    15,     0,     0,    17,    26,     0,    27,
       0,    23,    30,    24,     0,    25,     0,    28,     0,     0,
       0,     0,     0,     0,    52,    51,    47,    49,    48,    53,
      54,    50,     0,    29,     0,    86,     0,     0,     0,    31,
       0,    41,    56,    45,    43,    44,     0,     0,     0,    33,
      32,   100,    34,    35,    36,    93,     0,     0,    88,     0,
     113,     0,     0,    83,    87,    84,    85,     0,    40,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    37,    38,    39,     0,     0,   101,   105,   103,
      95,    94,     0,     0,     0,     0,     0,     0,    58,    60,
       0,    42,    90,    76,    77,    78,    79,    80,    81,    82,
      68,    69,    70,    71,    72,    73,    74,    75,    62,    63,
      64,    65,    66,    67,     0,    55,    46,   110,   109,   108,
       0,   106,   102,    96,     0,     0,    89,     0,     0,   114,
       0,    59,     0,    57,     0,    97,     0,   104,   111,     0,
       0,    61,    91,     0,    98,     0,     0,   107,    99,     0,
       0,   115,     0,   112,    92
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,  -166,   173,
    -166,  -166,  -166,   -38,  -166,   154,   -61,  -166,  -166,    85,
    -166,  -166,  -166,  -166,   117,  -166,  -166,  -166,  -166,  -166,
    -166,  -166,  -166,    78,    80,  -165,  -166,  -166,  -166
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     4,     7,    10,    11,    12,    24,    32,    13,
      33,    38,    40,   167,    46,   168,    70,    71,    72,    73,
     140,    74,    75,    76,    77,    78,    79,   132,   186,   199,
      80,    81,   127,   128,   129,   169,    82,    83,    84
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      43,    92,    41,    93,    94,    95,    96,    44,   180,   187,
     188,    27,    28,   120,     5,    90,     2,     6,     8,   178,
      14,   121,    15,    42,   130,   133,   134,   -11,   197,    42,
     139,     3,    45,   181,     9,   203,    20,   204,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
      16,    29,    30,    31,    21,   -10,    48,    49,    22,   -12,
      50,    51,   125,   126,   176,    52,   -13,    23,    53,    25,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    16,
      17,    18,    19,    63,    64,    34,    35,    65,    36,   179,
      98,   114,   115,   116,   117,   118,   119,    37,    39,   184,
      47,    85,   185,    86,    66,    67,    88,    87,    42,   191,
      68,   192,   116,   117,   118,   119,    89,    91,   195,    97,
     122,   123,   124,   198,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,    48,    49,   136,   201,    50,
      51,   135,   137,   170,    52,   142,   173,    53,   165,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    51,   177,
     182,    42,   119,    64,   189,   190,    65,    54,    55,    56,
      57,    58,    59,    60,    61,    62,   194,    26,   196,   202,
      69,    64,   131,    66,    67,   171,   166,   172,     0,    68,
      54,    55,    56,    57,    58,    59,    60,    61,    62,   138,
       0,    66,    67,     0,    64,    54,    55,    56,    57,    58,
      59,    60,    61,    62,     0,     0,     0,     0,     0,    64,
       0,     0,     0,     0,    66,    67,   108,   109,   110,   111,
       0,   141,   114,   115,   116,   117,   118,   119,     0,    66,
      67,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   174,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   175,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   193,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   200,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   183,     0,     0,     0,
       0,     0,     0,     0,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,    99,   100,   101,   102,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   106,   107,   108,   109,
     110,   111,     0,   113,   114,   115,   116,   117,   118,   119,
     106,   107,   108,   109,   110,   111,     0,     0,   114,   115,
     116,   117,   118,   119
};

static const yytype_int16 yycheck[] =
{
      38,    62,     7,    64,    65,    66,    67,     6,     6,   174,
     175,    13,    14,    32,    22,    53,     0,    34,    19,    30,
      15,    40,    66,    34,    85,    86,    87,    66,   193,    34,
      91,    15,    31,    31,    35,   200,    22,   202,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
      62,    63,    64,    65,    22,    66,     8,     9,     7,    66,
      12,    13,    10,    11,   135,    17,    66,    34,    20,    35,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    62,
      63,    64,    65,    35,    36,    22,    22,    39,     7,   137,
       7,    56,    57,    58,    59,    60,    61,    30,    22,   170,
      22,    30,   173,    30,    56,    57,    22,    30,    34,   180,
      62,   182,    58,    59,    60,    61,    30,    30,   189,    13,
       7,     7,     7,   194,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,     8,     9,    13,   196,    12,
      13,    41,    21,    30,    17,    22,     7,    20,    22,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    13,    22,
      41,    34,    61,    36,    18,    22,    39,    22,    23,    24,
      25,    26,    27,    28,    29,    30,     7,    24,    31,    31,
      46,    36,    85,    56,    57,   127,   121,   127,    -1,    62,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      -1,    56,    57,    -1,    36,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    -1,    -1,    -1,    -1,    -1,    36,
      -1,    -1,    -1,    -1,    56,    57,    50,    51,    52,    53,
      -1,    31,    56,    57,    58,    59,    60,    61,    -1,    56,
      57,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    31,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    31,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    31,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    33,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    48,    49,    50,    51,
      52,    53,    -1,    55,    56,    57,    58,    59,    60,    61,
      48,    49,    50,    51,    52,    53,    -1,    -1,    56,    57,
      58,    59,    60,    61
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    68,     0,    15,    69,    22,    34,    70,    19,    35,
      71,    72,    73,    76,    15,    66,    62,    63,    64,    65,
      22,    22,     7,    34,    74,    35,    76,    13,    14,    63,
      64,    65,    75,    77,    22,    22,     7,    30,    78,    22,
      79,     7,    34,    80,     6,    31,    81,    22,     8,     9,
      12,    13,    17,    20,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    35,    36,    39,    56,    57,    62,    82,
      83,    84,    85,    86,    88,    89,    90,    91,    92,    93,
      97,    98,   103,   104,   105,    30,    30,    30,    22,    30,
      80,    30,    83,    83,    83,    83,    83,    13,     7,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      32,    40,     7,     7,     7,    10,    11,    99,   100,   101,
      83,    91,    94,    83,    83,    41,    13,    21,    31,    83,
      87,    31,    22,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    83,    83,    83,    83,    22,    86,    80,    82,   102,
      30,   100,   101,     7,    31,    31,    83,    22,    30,    80,
       6,    31,    41,    33,    83,    83,    95,   102,   102,    18,
      22,    83,    83,    31,     7,    83,    31,   102,    83,    96,
      31,    80,    31,   102,   102
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    67,    68,    68,    69,    70,    70,    70,    71,    72,
      73,    73,    73,    73,    74,    74,    74,    75,    76,    76,
      76,    76,    76,    77,    77,    78,    79,    79,    79,    80,
      81,    81,    82,    82,    82,    82,    82,    82,    82,    82,
      82,    83,    83,    83,    83,    84,    84,    84,    84,    84,
      84,    84,    84,    84,    84,    84,    84,    85,    86,    86,
      87,    87,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    89,    89,    89,    90,    90,    91,    91,
      92,    92,    93,    94,    94,    94,    95,    95,    96,    96,
      97,    97,    97,    97,    98,    99,    99,   100,   101,   102,
     102,   103,   104,   105,   105,   105
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     5,     0,     2,     2,     4,     6,
       0,     2,     2,     2,     0,     3,     3,     3,     0,     2,
       2,     2,     2,     4,     4,     3,     0,     1,     3,     3,
       0,     2,     1,     1,     1,     1,     1,     2,     2,     2,
       2,     1,     3,     1,     1,     1,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     4,     3,     4,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     2,     2,     1,     2,     2,     4,
       3,     5,     9,     0,     1,     1,     0,     1,     0,     1,
       1,     2,     3,     2,     5,     1,     2,     5,     2,     1,
       1,     5,     8,     2,     4,     7
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (AST, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, AST); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, BaseAstNode** AST)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (AST);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep, BaseAstNode** AST)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep, AST);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule, BaseAstNode** AST)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              , AST);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, AST); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, BaseAstNode** AST)
{
  YYUSE (yyvaluep);
  YYUSE (AST);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (BaseAstNode** AST)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 144 "yyparser.y"
    {
        *AST = new AstNodeList();
        (yyval.expr) = *AST;
    }
#line 1625 "yyparser.cpp"
    break;

  case 3:
#line 150 "yyparser.y"
    {
        auto* namespaces = static_cast<AstNodeList*>((yyvsp[-1].expr));
        namespaces->vec.push_back((yyvsp[0].expr));
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1635 "yyparser.cpp"
    break;

  case 4:
#line 158 "yyparser.y"
    {
        (yyval.expr) = new NamespaceDeclNode((yyvsp[-3].token)->value, static_cast<AstNodeList*>((yyvsp[-1].expr)));
    }
#line 1643 "yyparser.cpp"
    break;

  case 5:
#line 163 "yyparser.y"
    {
        (yyval.expr) = new AstNodeList();
    }
#line 1651 "yyparser.cpp"
    break;

  case 6:
#line 168 "yyparser.y"
    {
        auto* nsList = static_cast<AstNodeList*>((yyvsp[-1].expr));
        nsList->vec.push_back((yyvsp[0].expr));
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1661 "yyparser.cpp"
    break;

  case 7:
#line 175 "yyparser.y"
    {
        auto* nsList = static_cast<AstNodeList*>((yyvsp[-1].expr));
        nsList->vec.push_back((yyvsp[0].expr));
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1671 "yyparser.cpp"
    break;

  case 8:
#line 183 "yyparser.y"
    {
        (yyval.expr) = new UsingNamespaceNode((yyvsp[-1].token)->value);
    }
#line 1679 "yyparser.cpp"
    break;

  case 9:
#line 189 "yyparser.y"
    {
        auto* classNode = new ClassDeclNode((yyvsp[-3].token)->value);
        classNode->modifiers = static_cast<StringList*>((yyvsp[-5].expr));
        classNode->members = static_cast<AstNodeList*>((yyvsp[-1].expr));
        (yyval.expr) = classNode;
    }
#line 1690 "yyparser.cpp"
    break;

  case 10:
#line 197 "yyparser.y"
    {
        (yyval.expr) = new StringList();
    }
#line 1698 "yyparser.cpp"
    break;

  case 11:
#line 202 "yyparser.y"
    {
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        modifiers->vec.push_back("public");
        (yyval.expr) = modifiers;
    }
#line 1708 "yyparser.cpp"
    break;

  case 12:
#line 209 "yyparser.y"
    {
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        modifiers->vec.push_back("private");
        (yyval.expr) = modifiers;
    }
#line 1718 "yyparser.cpp"
    break;

  case 13:
#line 216 "yyparser.y"
    {
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        modifiers->vec.push_back("static");
        (yyval.expr) = modifiers;
    }
#line 1728 "yyparser.cpp"
    break;

  case 14:
#line 223 "yyparser.y"
    {
        (yyval.expr) = new AstNodeList();
    }
#line 1736 "yyparser.cpp"
    break;

  case 15:
#line 228 "yyparser.y"
    {
        auto* members = static_cast<AstNodeList*>((yyvsp[-2].expr));
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        auto* method = static_cast<MethodDeclNode*>((yyvsp[0].expr));
        method->modifiers = modifiers;
        members->vec.push_back(method);
        (yyval.expr) = members;
    }
#line 1749 "yyparser.cpp"
    break;

  case 16:
#line 238 "yyparser.y"
    {
        auto* members = static_cast<AstNodeList*>((yyvsp[-2].expr));
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        auto* attribute = static_cast<AttributeDeclNode*>((yyvsp[0].expr));
        attribute->modifiers = modifiers;
        members->vec.push_back(attribute);
        (yyval.expr) = members;
    }
#line 1762 "yyparser.cpp"
    break;

  case 17:
#line 249 "yyparser.y"
    {
        (yyval.expr) = new AttributeDeclNode((yyvsp[-1].token)->value);
    }
#line 1770 "yyparser.cpp"
    break;

  case 18:
#line 254 "yyparser.y"
    {
        (yyval.expr) = new StringList();
    }
#line 1778 "yyparser.cpp"
    break;

  case 19:
#line 259 "yyparser.y"
    {
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        modifiers->vec.push_back("public");
        (yyval.expr) = modifiers;
    }
#line 1788 "yyparser.cpp"
    break;

  case 20:
#line 266 "yyparser.y"
    {
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        modifiers->vec.push_back("private");
        (yyval.expr) = modifiers;
    }
#line 1798 "yyparser.cpp"
    break;

  case 21:
#line 273 "yyparser.y"
    {
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        modifiers->vec.push_back("static");
        (yyval.expr) = modifiers;
    }
#line 1808 "yyparser.cpp"
    break;

  case 22:
#line 280 "yyparser.y"
    {
        auto* modifiers = static_cast<StringList*>((yyvsp[-1].expr));
        modifiers->vec.push_back("const");
        (yyval.expr) = modifiers;
    }
#line 1818 "yyparser.cpp"
    break;

  case 23:
#line 288 "yyparser.y"
    {
        (yyval.expr) = new MethodDeclNode((yyvsp[-2].token)->value, static_cast<StringList*>((yyvsp[-1].expr)), nullptr);
    }
#line 1826 "yyparser.cpp"
    break;

  case 24:
#line 293 "yyparser.y"
    {
        (yyval.expr) = new MethodDeclNode((yyvsp[-2].token)->value, static_cast<StringList*>((yyvsp[-1].expr)), static_cast<AstNodeList*>((yyvsp[0].expr)));
    }
#line 1834 "yyparser.cpp"
    break;

  case 25:
#line 299 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1842 "yyparser.cpp"
    break;

  case 26:
#line 304 "yyparser.y"
    {
        (yyval.expr) = new StringList();
    }
#line 1850 "yyparser.cpp"
    break;

  case 27:
#line 309 "yyparser.y"
    {
        auto* strs = new StringList();
        strs->vec.push_back((yyvsp[0].token)->value);
        (yyval.expr) = strs;
    }
#line 1860 "yyparser.cpp"
    break;

  case 28:
#line 316 "yyparser.y"
    {
        auto* strs = static_cast<StringList*>((yyvsp[-2].expr));
        strs->vec.push_back((yyvsp[0].token)->value);
        (yyval.expr) = strs;
    }
#line 1870 "yyparser.cpp"
    break;

  case 29:
#line 324 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1878 "yyparser.cpp"
    break;

  case 30:
#line 329 "yyparser.y"
    {
        (yyval.expr) = new AstNodeList();
    }
#line 1886 "yyparser.cpp"
    break;

  case 31:
#line 334 "yyparser.y"
    {
        auto* exprs = static_cast<AstNodeList*>((yyvsp[-1].expr));
        exprs->vec.push_back((yyvsp[0].expr));
        (yyval.expr) = exprs;
    }
#line 1896 "yyparser.cpp"
    break;

  case 32:
#line 342 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 1904 "yyparser.cpp"
    break;

  case 33:
#line 347 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 1912 "yyparser.cpp"
    break;

  case 34:
#line 352 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 1920 "yyparser.cpp"
    break;

  case 35:
#line 357 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 1928 "yyparser.cpp"
    break;

  case 36:
#line 362 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 1936 "yyparser.cpp"
    break;

  case 37:
#line 367 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1944 "yyparser.cpp"
    break;

  case 38:
#line 372 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1952 "yyparser.cpp"
    break;

  case 39:
#line 377 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1960 "yyparser.cpp"
    break;

  case 40:
#line 382 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1968 "yyparser.cpp"
    break;

  case 41:
#line 388 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 1976 "yyparser.cpp"
    break;

  case 42:
#line 393 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[-1].expr);
    }
#line 1984 "yyparser.cpp"
    break;

  case 43:
#line 398 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 1992 "yyparser.cpp"
    break;

  case 44:
#line 403 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 2000 "yyparser.cpp"
    break;

  case 45:
#line 409 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 2008 "yyparser.cpp"
    break;

  case 46:
#line 414 "yyparser.y"
    {
        auto* call = static_cast<MethodCallNode*>((yyvsp[0].expr));
        call->parent = (yyvsp[-2].expr);
        (yyval.expr) = call;
    }
#line 2018 "yyparser.cpp"
    break;

  case 47:
#line 421 "yyparser.y"
    {
        (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2026 "yyparser.cpp"
    break;

  case 48:
#line 426 "yyparser.y"
    {
        (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2034 "yyparser.cpp"
    break;

  case 49:
#line 431 "yyparser.y"
    {
       (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2042 "yyparser.cpp"
    break;

  case 50:
#line 436 "yyparser.y"
    {
        (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2050 "yyparser.cpp"
    break;

  case 51:
#line 441 "yyparser.y"
    {
        (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2058 "yyparser.cpp"
    break;

  case 52:
#line 446 "yyparser.y"
    {
        (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2066 "yyparser.cpp"
    break;

  case 53:
#line 451 "yyparser.y"
    {
        (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2074 "yyparser.cpp"
    break;

  case 54:
#line 456 "yyparser.y"
    {
        (yyval.expr) = new ObjectNode((yyvsp[0].token)->value, (yyvsp[0].token)->type);
    }
#line 2082 "yyparser.cpp"
    break;

  case 55:
#line 461 "yyparser.y"
    {
        (yyval.expr) = new MemberCallNode((yyvsp[-2].expr), (yyvsp[0].token)->value);
    }
#line 2090 "yyparser.cpp"
    break;

  case 56:
#line 466 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 2098 "yyparser.cpp"
    break;

  case 57:
#line 472 "yyparser.y"
    {
        (yyval.expr) = new IndexCallNode((yyvsp[-3].expr), (yyvsp[-1].expr));
    }
#line 2106 "yyparser.cpp"
    break;

  case 58:
#line 478 "yyparser.y"
    {
        (yyval.expr) = new MethodCallNode((yyvsp[-2].token)->value, nullptr, new AstNodeList());
    }
#line 2114 "yyparser.cpp"
    break;

  case 59:
#line 483 "yyparser.y"
    {
        (yyval.expr) = new MethodCallNode((yyvsp[-3].token)->value, nullptr, static_cast<AstNodeList*>((yyvsp[-1].expr)));
    }
#line 2122 "yyparser.cpp"
    break;

  case 60:
#line 489 "yyparser.y"
    {
        auto* args = new AstNodeList();
        args->vec.push_back((yyvsp[0].expr));
        (yyval.expr) = args;
    }
#line 2132 "yyparser.cpp"
    break;

  case 61:
#line 496 "yyparser.y"
    {
        auto* args = static_cast<AstNodeList*>((yyvsp[-2].expr));
        args->vec.push_back((yyvsp[0].expr));
        (yyval.expr) = args;
    }
#line 2142 "yyparser.cpp"
    break;

  case 62:
#line 504 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::SUM_OP);
    }
#line 2150 "yyparser.cpp"
    break;

  case 63:
#line 509 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::SUB_OP);
    }
#line 2158 "yyparser.cpp"
    break;

  case 64:
#line 514 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::MULT_OP);
    }
#line 2166 "yyparser.cpp"
    break;

  case 65:
#line 519 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::DIV_OP);
    }
#line 2174 "yyparser.cpp"
    break;

  case 66:
#line 524 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::MOD_OP);
    }
#line 2182 "yyparser.cpp"
    break;

  case 67:
#line 529 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::POWER_OP);
    }
#line 2190 "yyparser.cpp"
    break;

  case 68:
#line 534 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_EQUALS);
    }
#line 2198 "yyparser.cpp"
    break;

  case 69:
#line 539 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_NOT_EQUALS);
    }
#line 2206 "yyparser.cpp"
    break;

  case 70:
#line 544 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_LESS);
    }
#line 2214 "yyparser.cpp"
    break;

  case 71:
#line 549 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_GREATER);
    }
#line 2222 "yyparser.cpp"
    break;

  case 72:
#line 554 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_LESS_EQUALS);
    }
#line 2230 "yyparser.cpp"
    break;

  case 73:
#line 559 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_GREATER);
    }
#line 2238 "yyparser.cpp"
    break;

  case 74:
#line 564 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_OR);
    }
#line 2246 "yyparser.cpp"
    break;

  case 75:
#line 569 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::LOGIC_AND);
    }
#line 2254 "yyparser.cpp"
    break;

  case 76:
#line 574 "yyparser.y"
    {
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::ASSIGN_OP);
    }
#line 2262 "yyparser.cpp"
    break;

  case 77:
#line 579 "yyparser.y"
    {
        auto* expr = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::SUM_OP);
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), expr, Type::ASSIGN_OP);
    }
#line 2271 "yyparser.cpp"
    break;

  case 78:
#line 585 "yyparser.y"
    {
        auto* expr = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::SUB_OP);
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), expr, Type::ASSIGN_OP);
    }
#line 2280 "yyparser.cpp"
    break;

  case 79:
#line 591 "yyparser.y"
    {
        auto* expr = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::MULT_OP);
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), expr, Type::ASSIGN_OP);
    }
#line 2289 "yyparser.cpp"
    break;

  case 80:
#line 597 "yyparser.y"
    {
        auto* expr = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::DIV_OP);
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), expr, Type::ASSIGN_OP);
    }
#line 2298 "yyparser.cpp"
    break;

  case 81:
#line 603 "yyparser.y"
    {
        auto* expr = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::MOD_OP);
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), expr, Type::ASSIGN_OP);
    }
#line 2307 "yyparser.cpp"
    break;

  case 82:
#line 609 "yyparser.y"
    {
        auto* expr = new BinaryExprNode((yyvsp[-2].expr), (yyvsp[0].expr), Type::POWER_OP);
        (yyval.expr) = new BinaryExprNode((yyvsp[-2].expr), expr, Type::ASSIGN_OP);
    }
#line 2316 "yyparser.cpp"
    break;

  case 83:
#line 616 "yyparser.y"
    {
        (yyval.expr) = new UnaryExprNode((yyvsp[0].expr), Type::NEGATION_OP);
    }
#line 2324 "yyparser.cpp"
    break;

  case 84:
#line 621 "yyparser.y"
    {
        auto* expr = new UnaryExprNode((yyvsp[0].expr), Type::POSITIVE_OP);
    }
#line 2332 "yyparser.cpp"
    break;

  case 85:
#line 626 "yyparser.y"
    {
        auto* expr = new UnaryExprNode((yyvsp[0].expr), Type::NEGATIVE_OP);
    }
#line 2340 "yyparser.cpp"
    break;

  case 86:
#line 632 "yyparser.y"
    {
        (yyval.expr) = new ReturnExprNode(new ObjectNode("null", Token::Type::NULLPTR));
        // maybe check if function is a constructor?
    }
#line 2349 "yyparser.cpp"
    break;

  case 87:
#line 638 "yyparser.y"
    {
         (yyval.expr) = new ReturnExprNode((yyvsp[0].expr));
    }
#line 2357 "yyparser.cpp"
    break;

  case 88:
#line 644 "yyparser.y"
    {
        (yyval.expr) = new VariableDeclNode((yyvsp[0].token)->value, false, nullptr);
    }
#line 2365 "yyparser.cpp"
    break;

  case 89:
#line 649 "yyparser.y"
    {
        (yyval.expr) = new VariableDeclNode((yyvsp[-2].token)->value, false, (yyvsp[0].expr));
    }
#line 2373 "yyparser.cpp"
    break;

  case 90:
#line 655 "yyparser.y"
    {
        (yyval.expr) = new VariableDeclNode((yyvsp[0].token)->value, true, new ObjectNode("null", Token::Type::NULLPTR));
    }
#line 2381 "yyparser.cpp"
    break;

  case 91:
#line 660 "yyparser.y"
    {
        (yyval.expr) = new VariableDeclNode((yyvsp[-2].token)->value, true, (yyvsp[0].expr));
    }
#line 2389 "yyparser.cpp"
    break;

  case 92:
#line 666 "yyparser.y"
    {
        (yyval.expr) = new ForExprNode((yyvsp[-6].expr), (yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 2397 "yyparser.cpp"
    break;

  case 93:
#line 671 "yyparser.y"
    {
        (yyval.expr) = nullptr;
    }
#line 2405 "yyparser.cpp"
    break;

  case 94:
#line 676 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 2413 "yyparser.cpp"
    break;

  case 95:
#line 681 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 2421 "yyparser.cpp"
    break;

  case 96:
#line 686 "yyparser.y"
    {
        (yyval.expr) = nullptr;
    }
#line 2429 "yyparser.cpp"
    break;

  case 98:
#line 694 "yyparser.y"
    {
        (yyval.expr) = nullptr;
    }
#line 2437 "yyparser.cpp"
    break;

  case 99:
#line 699 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 2445 "yyparser.cpp"
    break;

  case 100:
#line 705 "yyparser.y"
    {
        (yyval.expr) = new IfStatementNode(
            static_cast<ConditionalNode*>((yyvsp[0].expr)), 
            new AstNodeList(), 
            nullptr
        );
    }
#line 2457 "yyparser.cpp"
    break;

  case 101:
#line 714 "yyparser.y"
    {
        (yyval.expr) = new IfStatementNode(
            static_cast<ConditionalNode*>((yyvsp[-1].expr)), 
            static_cast<AstNodeList*>((yyvsp[0].expr)), 
            nullptr
        );
    }
#line 2469 "yyparser.cpp"
    break;

  case 102:
#line 723 "yyparser.y"
    {
        (yyval.expr) = new IfStatementNode(
            static_cast<ConditionalNode*>((yyvsp[-2].expr)), 
            static_cast<AstNodeList*>((yyvsp[-1].expr)), 
            static_cast<ConditionalNode*>((yyvsp[0].expr))
        );
    }
#line 2481 "yyparser.cpp"
    break;

  case 103:
#line 732 "yyparser.y"
    {
        (yyval.expr) = new IfStatementNode(
            static_cast<ConditionalNode*>((yyvsp[-1].expr)), 
            new AstNodeList(), 
            static_cast<ConditionalNode*>((yyvsp[0].expr))
        );
    }
#line 2493 "yyparser.cpp"
    break;

  case 104:
#line 742 "yyparser.y"
    {
        (yyval.expr) = new ConditionalNode((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 2501 "yyparser.cpp"
    break;

  case 105:
#line 748 "yyparser.y"
    {
        auto* elifs = new AstNodeList();
        elifs->vec.push_back((yyvsp[0].expr));
    }
#line 2510 "yyparser.cpp"
    break;

  case 106:
#line 754 "yyparser.y"
    {
        auto* elifs = static_cast<AstNodeList*>((yyvsp[-1].expr));
        elifs->vec.push_back((yyvsp[0].expr));
    }
#line 2519 "yyparser.cpp"
    break;

  case 107:
#line 761 "yyparser.y"
    {
        (yyval.expr) = new ConditionalNode((yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 2527 "yyparser.cpp"
    break;

  case 108:
#line 767 "yyparser.y"
    {
        (yyval.expr) = new ConditionalNode(nullptr, (yyvsp[0].expr));
    }
#line 2535 "yyparser.cpp"
    break;

  case 109:
#line 773 "yyparser.y"
    {
        auto* exprs = new AstNodeList();
        exprs->vec.push_back((yyvsp[0].expr));
        (yyval.expr) = exprs;
    }
#line 2545 "yyparser.cpp"
    break;

  case 110:
#line 780 "yyparser.y"
    {
        (yyval.expr) = (yyvsp[0].expr);
    }
#line 2553 "yyparser.cpp"
    break;

  case 111:
#line 786 "yyparser.y"
    {
        (yyval.expr) = new ForExprNode(nullptr, (yyvsp[-2].expr), nullptr, (yyvsp[0].expr));
        // init and iter are empty in while statement
    }
#line 2562 "yyparser.cpp"
    break;

  case 112:
#line 793 "yyparser.y"
    {
        (yyval.expr) = new ForeachExprNode((yyvsp[-4].token)->value, (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 2570 "yyparser.cpp"
    break;

  case 113:
#line 799 "yyparser.y"
    {
        (yyval.expr) = new TryCatchNode("", (yyvsp[0].expr), nullptr);
    }
#line 2578 "yyparser.cpp"
    break;

  case 114:
#line 804 "yyparser.y"
    {
        (yyval.expr) = new TryCatchNode("", (yyvsp[-2].expr), (yyvsp[0].expr));
    }
#line 2586 "yyparser.cpp"
    break;

  case 115:
#line 809 "yyparser.y"
    {
        (yyval.expr) = new TryCatchNode((yyvsp[-2].token)->value, (yyvsp[-5].expr), (yyvsp[0].expr));
    }
#line 2594 "yyparser.cpp"
    break;


#line 2598 "yyparser.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (AST, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (AST, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, AST);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, AST);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (AST, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, AST);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, AST);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 812 "yyparser.y"
