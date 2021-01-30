#include "Database.h"

sqlite3* Database::dbConnection;
vector<vector<string>> Database::dbResults;
char* Database::errorMessage;

// method to connect to the database and initialize tables in the database
void Database::initialize() {
	// open a database connection and store the pointer into dbConnection
	sqlite3_open("database.db", &dbConnection);

	const vector<string>sqlCleaning{
		"DROP TABLE IF EXISTS procedures",
		"DROP TABLE IF EXISTS statement",
		"DROP TABLE IF EXISTS variable",
		"DROP TABLE IF EXISTS print",
		"DROP TABLE IF EXISTS read",
		"DROP TABLE IF EXISTS assign",
		"DROP TABLE IF EXISTS statement_list",
		"DROP TABLE IF EXISTS constant",
	};

	const vector<string>sqlCreation{
		"CREATE TABLE procedures (procedureID INTEGER PRIMARY KEY, procedureName VARCHAR(255))",
		"CREATE TABLE statement ( statementID INTEGER PRIMARY KEY, statementType INT, statementRowID INT, parentProcedure VARCHAR(255), "
			"CONSTRAINT FK_StatToProc FOREIGN KEY (parentProcedure) REFERENCES procedures(procedureName))",
		
		"CREATE TABLE variable ( variableID INTEGER PRIMARY KEY, variableName VARCHAR(255), variableValue INT)",
		"CREATE TABLE print ( printID INTEGER PRIMARY KEY, rowID INT, variableName VARCHAR(255))",
			//"CONSTRAINT FK_printToStatement FOREIGN KEY (rowID) REFERENCES procedures(procedureName))"
		"CREATE TABLE read ( readID INTEGER PRIMARY KEY, rowID INT, variableName VARCHAR(255))",
		"CREATE TABLE assign ( assignID INTEGER PRIMARY KEY, rowID INT, variableName VARCHAR(255))",
		"CREATE TABLE constant ( constantID INTEGER PRIMARY KEY, rowID INT, variableValue INT)",
		"CREATE TABLE statement_list (statementIndex INTEGER PRIMARY KEY, statementName VARCHAR(255))"
	};

	//// drop the existing tables (if any)
	for (const auto& string : sqlCleaning)
	{
		sqlite3_exec(dbConnection, string.c_str(), NULL, 0, &errorMessage);
	}
	
	// create tables
	for (const auto& string : sqlCreation)
	{
		sqlite3_exec(dbConnection, string.c_str(), NULL, 0, &errorMessage);
	}

	const string statementListFillUp {
		"INSERT INTO statement_list ('statementIndex','statementName' )"
		" VALUES ('0', 'STMT'),"
		" ('1', 'READ'), ('2', 'PRINT'),"
		" ('3', 'ASSIGN'), ('4', 'CALL'),"
		" ('5', 'WHILE'), ('6', 'IF')"
	};
	sqlite3_exec(dbConnection, statementListFillUp.c_str(), NULL, 0, &errorMessage);
	
	// initialize the result vector
	dbResults = vector<vector<string>>();
}

// method to close the database connection
void Database::close() {
	sqlite3_close(dbConnection);
}

// method to insert a procedure into the database
void Database::insertProcedure(string procedureName) {
	string insertProcedureSQL = "INSERT INTO procedures ('procedureName') VALUES ('" + procedureName + "');";
	sqlite3_exec(dbConnection, insertProcedureSQL.c_str(), NULL, 0, &errorMessage);
}

//print/read/statement/assign return rowiD, constant return Num, variable/prcedure return name
void Database::pushToDB(Statement* statement, StatementList statementType)
{
	vector<string>sqlInsertCommand;

	sqlInsertCommand.push_back("INSERT INTO statement ('statementType', 'statementRowID', 'parentProcedure') "
		"VALUES ('" + to_string(static_cast<int>(statement->statementType)) + "', "
		"'" + to_string(statement->statementRowID) + "','" + statement->parentProcedure + "')");
	
	if(!isVariableNameExist(statement->variable.variableName))
	{
		sqlInsertCommand.push_back("INSERT INTO variable ('variableName', 'variableValue') "
			"VALUES ('" + statement->variable.variableName + "', '" + to_string(statement->variable.variableValue) + "')");
	}

	switch (statementType)
	{
	case StatementList::READ:			
		sqlInsertCommand.push_back("INSERT INTO read ('variableName', 'rowID') "
			"VALUES ('" + statement->variable.variableName + "', '" + to_string(statement->read.rowID) + "')");
		break;
		
	case StatementList::PRINT:
		sqlInsertCommand.push_back("INSERT INTO print ('variableName', 'rowID') "
			"VALUES ('" + statement->variable.variableName + "', '" + to_string(statement->print.rowID) + "')");
		break;
		
	case StatementList::ASSIGN:
		sqlInsertCommand.push_back("INSERT INTO constant ('variableValue', 'rowID') "
			"VALUES ('" + to_string(statement->constant.variableValue) + "', '" + to_string(statement->assign.rowID) + "')");
		
		sqlInsertCommand.push_back("INSERT INTO assign ('variableName', 'rowID') "
			"VALUES ('" + statement->variable.variableName + "', '" + to_string(statement->assign.rowID) + "')");
		break;
		
	case StatementList::STMT: break;
	case StatementList::CALL: break;
	case StatementList::WHILE: break;
	case StatementList::IF: break;
	case StatementList::INVALID: break;
	default:
		break;
	}

	if (sqlInsertCommand.empty())
		return;
	
	for (const auto& string : sqlInsertCommand)
	{
		sqlite3_exec(dbConnection, string.c_str(), NULL, 0, &errorMessage);
	}
}

// method to get all the procedures from the database
void Database::getProcedures(vector<string>& results){
	// clear the existing results
	dbResults.clear();

	// retrieve the procedures from the procedure table
	// The callback method is only used when there are results to be returned.
	string getProceduresSQL = "SELECT * FROM procedures;";
	sqlite3_exec(dbConnection, getProceduresSQL.c_str(), callback, 0, &errorMessage);

	// postprocess the results from the database so that the output is just a vector of procedure names
	for (vector<string> dbRow : dbResults) {
		string result;
		result = dbRow.at(1);
		results.push_back(result);
	}
}

void Database::getConstant(vector<string>& results)
{
	dbResults.clear();

	string sqlCommand = "SELECT * FROM constant;";
	sqlite3_exec(dbConnection, sqlCommand.c_str(), callback, 0, &errorMessage);
	for (vector<string> dbRow : dbResults)
	{
		results.push_back(dbRow.at(2));
	}
}

void Database::getVariable(vector<string>& results)
{
	dbResults.clear();

	string sqlCommand = "SELECT * FROM variable;";
	sqlite3_exec(dbConnection, sqlCommand.c_str(), callback, 0, &errorMessage);
	for (vector<string> dbRow : dbResults)
	{
		results.push_back(dbRow.at(1));
	}
}

bool Database::isVariableNameExist(string variableName) {
	// clear the existing results
	dbResults.clear();

	// retrieve the procedures from the procedure table
	// The callback method is only used when there are results to be returned.
	string getProceduresSQL = "SELECT * FROM variable;";
	sqlite3_exec(dbConnection, getProceduresSQL.c_str(), callback, 0, &errorMessage);

	// postprocess the results from the database so that the output is just a vector of procedure names
	for (vector<string> dbRow : dbResults)
	{
		if (variableName == dbRow.at(1))
			return true;
	}

	return false;
}

void Database::getResultString(vector<string>& results, StatementList statement)
{
	dbResults.clear();
	string sqlCommand;
	switch (statement)
	{
	case StatementList::PRINT:
		sqlCommand = "SELECT * FROM print;";
		sqlite3_exec(dbConnection, sqlCommand.c_str(), callback, 0, &errorMessage);
		for (vector<string> dbRow : dbResults)
		{
			results.push_back(dbRow.at(1));
		}
		break;
	case StatementList::READ:
		sqlCommand = "SELECT * FROM read;";
		sqlite3_exec(dbConnection, sqlCommand.c_str(), callback, 0, &errorMessage);
		for (vector<string> dbRow : dbResults)
		{
			results.push_back(dbRow.at(1));
		}
		break;
	case StatementList::ASSIGN:
		sqlCommand = "SELECT * FROM assign;";
		sqlite3_exec(dbConnection, sqlCommand.c_str(), callback, 0, &errorMessage);
		for (vector<string> dbRow : dbResults)
		{
			results.push_back(dbRow.at(1));
		}
		break;
	case StatementList::STMT:
		sqlCommand = "SELECT * FROM statement;";
		sqlite3_exec(dbConnection, sqlCommand.c_str(), callback, 0, &errorMessage);
		for (vector<string> dbRow : dbResults)
		{
			results.push_back(dbRow.at(2));
		}
		break;
	default: 
		break;
	}
}

// callback method to put one row of results from the database into the dbResults vector
// This method is called each time a row of results is returned from the database
int Database::callback(void* NotUsed, int argc, char** argv, char** azColName) {
	NotUsed = 0;
	vector<string> dbRow;

	// argc is the number of columns for this row of results
	// argv contains the values for the columns
	// Each value is pushed into a vector.
	for (int i = 0; i < argc; i++) {
		dbRow.push_back(argv[i]);
	}

	// The row is pushed to the vector for storing all rows of results 
	dbResults.push_back(dbRow);

	return 0;
}
