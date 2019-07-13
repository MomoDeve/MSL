#pragma once

#include "lexer.h"
#include "function.h"
#include "class.h"
#include "stringExtensions.h"
#include "namespace.h"
#include "expressions.h"
#include "assembly.h"

#include <ostream>
#include <memory>

class Parser
{
	static const size_t constantMaxSize = UINT8_MAX;
	using ModifierList = std::vector<Token::Type>;
	template<typename T> using unique_ptr = std::unique_ptr<T>;

	Assembly assembly;
	Lexer* lexer;
	std::ostream* stream;
	bool success;
	bool hasEntryPoint;
	uint8_t mode;
	std::string entryPointName = "Main";
	const size_t maxErrorCount = 8;
	size_t currentErrorCount = 0;

	bool GenerateAssembly();
	bool GenerateMembers(Namespace& _namespace);
	bool ProcessClass(Namespace& _namespace, Class& classObject, const ModifierList& modifiers);
	bool ProcessInterface(Namespace& _namespace, Class& interfaceObject, const ModifierList& modifiers);
	bool ProcessVariable(Namespace& _namespace, Class& classObject, const ModifierList& modifiers);
	bool ProcessFunction(Namespace& _namespace, Class& classObject, const ModifierList& modifiers);
	bool GetModifiers(ModifierList& modifiers, const std::vector<Token::Type>& stopTokens);
	bool ParseFunctionParamsDecl(std::vector<std::string>& parameters);

	ExpressionList ParseExpressionBlock(Function& function);
	ExpressionList ParseFunctionArguments(Function& function);
	unique_ptr<BaseExpression> ParseIndexArgument(Function& function);
	unique_ptr<BaseExpression> ParseExpression(Function& function);
	unique_ptr<BaseExpression> ParseVariableDecl(Function& function);
	unique_ptr<BaseExpression> ParseLambdaExpression(Function& function);
	unique_ptr<BaseExpression> ParseForExpression(Function& function);
	unique_ptr<BaseExpression> ParseForeachExpression(Function& function);
	unique_ptr<BaseExpression> ParseWhileExpression(Function& function);
	unique_ptr<BaseExpression> ParseIfExpression(Function& function);
	unique_ptr<BaseExpression> ParseReturnExpression(Function& function);
	unique_ptr<BaseExpression> ParseStatementInBrackets(Function& function);
	unique_ptr<BaseExpression> ParseNextVariable(Function& function);
	unique_ptr<BaseExpression> ParseRawExpression(Function& function, unique_ptr<BaseExpression> leftBranch = nullptr);

	void Notify(std::string message);
	void Error(std::string message);
	void Debug(std::string message);
	void Warning(std::string message);
	void ShowErrorLine();
public:
	enum Mode
	{
		NO_OUTPUT = 0,
		ERROR_ONLY = 1,
		WARNING_ONLY = ERROR_ONLY << 1,
		DEBUG_ONLY = WARNING_ONLY << 1,
		NO_STACKTRACE = (DEBUG_ONLY << 1),
		NO_DEBUG = ERROR_ONLY | WARNING_ONLY,
		FULL_TRACE = DEBUG_ONLY | NO_DEBUG,
	};

	Parser(Lexer* lexer, std::ostream* errorStream, Mode mode);

	bool Parse();
	Assembly&& PullAssembly();

	const Assembly& GetAssembly() const;
	bool HasEntryPoint() const;
	bool ParsingSuccess() const;
};