#include "SourceProcessor.h"
#include <map>

//test for insert ROW ID + Content to DB, currently no need to deal with that
void pushRowInfoToTable(map<int, vector<string>> keyValuePair)
{
	for (const auto& pair : keyValuePair)
	{
		const int currentRowID = pair.first;
		string content = "";
		for (unsigned int i = 0; i < pair.second.size(); i++)
		{
			if (i == pair.second.size() - 1)
			{
				content += pair.second[i];
			}
			content += (pair.second[i] + " ");
		}
	}
}

// method for processing the source program
// This method currently only inserts the procedure name into the database
// using some highly simplifed logic.
// You should modify this method to complete the logic for handling all the required syntax.
void SourceProcessor::process(string program) {
	// initialize the datbase
	Database::initialize();

	// tokenize the program
	Tokenizer tk;
	vector<string> tokens;
	tk.tokenize(program, tokens);
	map<int, vector<string>> keyValuePair = tk.tokenize(program, tokens);
	// This logic is highly simplified based on iteration 1 requirements and 
	// the assumption that the programs are valid.
	string procedureName = keyValuePair[0].at(1);
	// insert the procedure into the database
	Database::insertProcedure(procedureName);
	pushRowInfoToTable(keyValuePair);
	stringProcessing(keyValuePair, procedureName);
	
}



StatementList ConvertStringToEnum(string textBlock)
{
	if (textBlock == "read")
		return StatementList::READ;
	if (textBlock == "print")
		return StatementList::PRINT;
	if (textBlock == "=" || textBlock == "-"
		|| textBlock == "+")
		return StatementList::ASSIGN;
	
	return StatementList::INVALID;
};


void SourceProcessor::stringProcessing(map<int, vector<string>> keyValuePair, string parentProcedure)
{
	
	//Variable* variable = NULL;
	int statementID = 0;
	int variableID = 0;
	int tableIndex = 0;//will create function in DB to get count of current table in future;

	for (const auto& pair : keyValuePair) 
	{
		vector<string> tokens = pair.second;
		const int currentRowID = pair.first;
		
		for (unsigned i = 0; i < tokens.size(); i++)
		{

			Statement* statement = new Statement();

			switch (ConvertStringToEnum(tokens[i]))
			{
			case StatementList::READ:
				//fill up information
				statement->statementID = statementID++;
				statement->statementType = StatementList::READ;
				statement->statementRowID = currentRowID;
				statement->parentProcedure = parentProcedure;
				statement->variable.variableID = variableID++;
				statement->variable.variableValue = NULL;
				statement->variable.variableName = tokens[i + 1];
				statement->read.readID = tableIndex++;

				//upload to DB
				Database::pushToDB(statement, StatementList::READ);
				break;
			case StatementList::PRINT:  // NOLINT(bugprone-branch-clone)
				statement->statementID = statementID++;
				statement->statementType = StatementList::READ;
				statement->statementRowID = currentRowID;
				statement->parentProcedure = parentProcedure;
				statement->variable.variableID = variableID++;
				statement->variable.variableValue = NULL;
				statement->variable.variableName = tokens[i + 1];
				statement->print.printID = tableIndex++;

				//upload to DB
				Database::pushToDB(statement, StatementList::PRINT);
				break;
			case StatementList::ASSIGN:
				statement->statementID = statementID++;
				statement->statementType = StatementList::READ;
				statement->statementRowID = currentRowID;
				statement->parentProcedure = parentProcedure;
				statement->variable.variableID = variableID++;
				statement->variable.variableValue = NULL; // need to cal before UPDATE DB
				statement->variable.variableName = tokens[i - 1];
				statement->assign.assignID = tableIndex++;
				statement->assign.symbol = tokens[i];
				statement->assign.assignValue = tokens[i + 1];
				//upload to DB
				Database::pushToDB(statement, StatementList::ASSIGN);
				break;
			case StatementList::STMT: break;
			case StatementList::CALL: break;
			case StatementList::WHILE: break;
			case StatementList::IF: break;
			case StatementList::INVALID: break;
			default:
				break;
			}
		}
	}
	

}


