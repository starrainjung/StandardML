/*---------------------------------------------------
	the head file of Lexer
---------------------------------------------------*/
#pragma once
#include "Utility.h"


class Lexer {
public:

	string IdentifierStr = "";
	char CharVal = 0x00;
	string StrVal = "";
	double NumVal = 0;
	bool BoolVal = 1;

	int gettok();


private:
	
	int readE(int* LastChar, string* NumStr);
	int readPoint(int* LastChar, string* NumStr);
	int readNum(bool isnegative, int* LastChar);
	bool issymble(int LastChar);
	int readStr(bool isStr, int* LastChar);
	int readEscapeSequence(bool isStr, int* LastChar;
};