/*---------------------------------------------------
	the head file of Lexer
---------------------------------------------------*/
#pragma once
#include "Utility.h"


class Lexer {
public:

	int gettok();
	const string& getIdentifierStr() { return this->IdentifierStr; }
	const char& getCharVal() { return this->CharVal; }
	const string& getStrVal() { return this->StrVal; }
	const double& getNumVal() { return this->NumVal; }
	const bool& getBoolVal() { return this->BoolVal; }
	void resetLastChar() { this->LastChar = ' '; }

private:

	int LastChar = ' ';
	string IdentifierStr = "";
	char CharVal = 0x00;
	string StrVal = "";
	double NumVal = 0;
	bool BoolVal = 1;
	
	int readE(string* NumStr);
	int readPoint(string* NumStr);
	int readNum(bool isnegative);
	bool issymble(int LastChar);
	int readStr(bool isStr);
	int readEscapeSequence(bool isStr);

};
