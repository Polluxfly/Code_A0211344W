#pragma once
#include <string>

enum class StatementList
{
	STMT,
	READ,
	PRINT,
	ASSIGN,
	CALL,
	WHILE,
	IF,
	INVALID // for local usage only
};

class Statement
{
public:
	int statementID;
	int statementRowID;
	StatementList statementType;
	std::string statementContent;
	std::string parentProcedure;

	struct Variable
	{
		int variableID;
		std::string variableName;
		int variableValue;
	}variable;

	struct ReadStatement
	{
		int readID;
	}read;

	struct PrintStatement
	{
		int printID;
	}print;

	struct ConstantStatement
	{
		int constantID;
	}constant;
	struct AssignStatement
	{
		int assignID;
		std::string symbol;
		std::string assignValue;
	}assign;
};

//struct ReadStatement
//{
//	std::string variableName;
//	int id;
//};

//struct PrintStatement
//{
//	std::string variableName;
//	int id;
//};
//
//struct AssignStatement
//{
//	std::string variableName;
//	std::string symbol;
//	int assignValue;
//	int id;
//};