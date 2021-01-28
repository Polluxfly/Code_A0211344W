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
		"DROP TABLE IF EXISTS variable",
		"DROP TABLE IF EXISTS statement",
		"DROP TABLE IF EXISTS print",
		"DROP TABLE IF EXISTS read",
		"DROP TABLE IF EXISTS assign",
		"DROP TABLE IF EXISTS statement_list",
		"DROP TABLE IF EXISTS statement_content",
	};

	const vector<string>sqlCreation{
		"CREATE TABLE procedures (procedureID INTEGER PRIMARY KEY, procedureName VARCHAR(255));",
		"CREATE TABLE statement ( statementID INTEGER PRIMARY KEY, statementType INT, statementRowID INT,statementContent VARCHAR(255), parentProcedure VARCHAR(255), "
			"CONSTRAINT FK_StatToProc FOREIGN KEY (parentProcedure) REFERENCES procedures(procedureName))",
		"CREATE TABLE variable ( variableID INTEGER PRIMARY KEY, statementID INT, variableName VARCHAR(255), variableValue INT, isConstant bit(1) DEFAULT 0"
			"CONSTRAINT FK_VarToStat FOREIGN KEY (statementID) REFERENCES statement(statementID))",

		"CREATE TABLE statement_content ( statementID INTEGER PRIMARY KEY, statementContent VARCHAR(255), "
			"CONSTRAINT FK_statID FOREIGN KEY (statementID) REFERENCES statement(statementID))",

		"CREATE TABLE print ( printID INTEGER PRIMARY KEY, variableID INT, variableName VARCHAR(255),"
			"CONSTRAINT FK_PrintToVar FOREIGN KEY (variableName) REFERENCES variable(variableName))",
		"CREATE TABLE read ( readID INTEGER PRIMARY KEY, variableID INT, variableName VARCHAR(255),"
			"CONSTRAINT FK_ReadToVar FOREIGN KEY (variableName) REFERENCES variable(variableName))",
		"CREATE TABLE assign ( assignID INTEGER PRIMARY KEY, variableID INT, variableName VARCHAR(255), symbol VARCHAR(10), assignValue INT,"
			"CONSTRAINT FK_AssignToVar FOREIGN KEY (variableName) REFERENCES variable(variableName))",

		"CREATE TABLE statement_list (statementIndex INTEGER PRIMARY KEY, statementName VARCHAR(255));"
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

//template <typename T>
void Database::pushToDB(Statement* statement, StatementList statementType)
{
	vector<string>sqlInsertCommand;
	switch (statementType)
	{
	case StatementList::READ:
		sqlInsertCommand.push_back("INSERT INTO statement ('statementType', 'statementRowID', 'statementContent', 'parentProcedure') "
			"VALUES ('" + to_string(static_cast<int>(statement->statementType)) + "', "
			"'" + to_string(statement->statementRowID) + "', '"  "',  '" + statement->parentProcedure + "')");
		sqlInsertCommand.push_back("INSERT INTO variable ('statementID','variableName', 'variableValue') "
			"VALUES ('" + to_string(statement->statementID) + "', "
			"'" + statement->variable.variableName + "', '" + to_string(statement->variable.variableValue) + "')");
		sqlInsertCommand.push_back("INSERT INTO read ('variableName')"
			" VALUES ('" + statement->variable.variableName + "')");
		break;
		
	case StatementList::PRINT:
		sqlInsertCommand.push_back("INSERT INTO statement ('statementID', 'statementType', 'statementRowID', 'statementContent', 'parentProcedure') "
			"VALUES ('" + to_string(statement->statementID) + "', '" + to_string(static_cast<int>(statement->statementType)) + "', "
			"'" + to_string(statement->statementRowID) + "', '"  "',  '" + statement->parentProcedure + "')");
		sqlInsertCommand.push_back("INSERT INTO variable ('variableID','statementID','variableName', 'variableValue') "
			"VALUES ('" + to_string(statement->variable.variableID) + "', '" + to_string(statement->statementID) + "', "
			"'" + statement->variable.variableName + "', '" + to_string(statement->variable.variableValue) + "')");
		sqlInsertCommand.push_back("INSERT INTO print ('variableName')"
			" VALUES ('" + statement->variable.variableName + "')");
		break;
		
	case StatementList::ASSIGN:
		sqlInsertCommand.push_back("INSERT INTO statement ('statementID', 'statementType', 'statementRowID', 'statementContent', 'parentProcedure') "
			"VALUES ('" + to_string(statement->statementID) + "', '" + to_string(static_cast<int>(statement->statementType)) + "', "
			"'" + to_string(statement->statementRowID) + "', '"  "',  '" + statement->parentProcedure + "')");
		sqlInsertCommand.push_back("INSERT INTO variable ('variableID','statementID','variableName', 'variableValue') "
			"VALUES ('" + to_string(statement->variable.variableID) + "', '" + to_string(statement->statementID) + "', "
			"'" + statement->variable.variableName + "', '" + to_string(statement->variable.variableValue) + "')");
		sqlInsertCommand.push_back("INSERT INTO assign ('assignID', 'symbol', 'assignValue', 'variableName')"
			" VALUES ('" + to_string(statement->assign.assignID) + "', '" + statement->assign.symbol + "',"
			" '" + statement->assign.assignValue + "', '" + statement->variable.variableName + "')");
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
//vector<string>sqlInsertCommand{
//	"INSERT INTO statement ('statementID') VALUES ('" + to_string(statement->statementID) + "')",
//	"INSERT INTO statement ('statementType') VALUES ('" + to_string(static_cast<int>(statement->statementType)) + "')",
//	"INSERT INTO statement ('statementRowID') VALUES ('" + to_string(statement->statementRowID) + "')",
//	"INSERT INTO statement ('statementContent') VALUES ('"  "')", // wait a min = = later will create function to get row content
//	"INSERT INTO statement ('parentProcedure') VALUES ('" + statement->parentProcedure + "')",
//	"INSERT INTO variable ('variableID') VALUES ('" + statement->parentProcedure + "')",
//	"INSERT INTO variable ('statementID') VALUES ('" + to_string(statement->statementID) + "')", //later set FK
//	"INSERT INTO variable ('variableName') VALUES ('" + statement->variable.variableName + "')",
//	"INSERT INTO variable ('variableValue') VALUES ('" + to_string(statement->variable.variableValue) + "')",
//	"INSERT INTO read ('readID') VALUES ('" + to_string(statement->read.readID) + "')",

//};