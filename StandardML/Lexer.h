/*---------------------------------------------------
	the head file of Lexer
---------------------------------------------------*/
#ifndef LEXER_H

#define LEXER_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Token.h"

using namespace std;

class Lexer {
	int LastChar;
	string IdentifierStr;
	char CharVal;
	string StrVal;
	double NumVal;
	bool BoolVal;

	int readE(string* NumStr);
	int readPoint(string* NumStr);
	int readNum(bool isnegative);
	bool issymble(int LastChar);
	int readStr(bool isStr);
	int readEscapeSequence(bool isStr);
	int LexerError(const char* info);

public:

	int gettok();
	const string& getIdentifierStr() { return this->IdentifierStr; }
	const char& getCharVal() { return this->CharVal; }
	const string& getStrVal() { return this->StrVal; }
	const double& getNumVal() { return this->NumVal; }
	const bool& getBoolVal() { return this->BoolVal; }
	void resetLastChar() { this->LastChar = ' '; }
	Lexer() {
		this->LastChar = ' ';
		this->IdentifierStr = "";
		this->CharVal = 0x00;
		this->StrVal = "";
		this->NumVal = 0;
		this->BoolVal = 1;
	}
};

#endif // !LEXER_H
