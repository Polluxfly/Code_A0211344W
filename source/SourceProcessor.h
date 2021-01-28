#pragma once
#include <sstream>
#include <fstream>
#include <vector>
#include "Database.h"
#include "Tokenizer.h"
#include "Statements.h"

using namespace std;

class SourceProcessor {
public:
	// method for processing the source program
	void process(string program);
	void stringProcessing(map<int, vector<string>> keyValuePair, string parentProcedure);
};