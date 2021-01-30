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
		int rowID;
		std::string variableName;
	}read;

	struct PrintStatement
	{
		int printID;
		int rowID;
		std::string variableName;
		
	}print;

	struct ConstantStatement
	{
		int constantID;
		int rowID;
		int variableValue;
	}constant;
	
	struct AssignStatement
	{
		int assignID;
		int rowID;
		std::string variableName;
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