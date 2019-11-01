/*---------------------------------------------------
	the head file of Lexer
	

---------------------------------------------------*/
#pragma once
#include "Utility.h"


class Lexer {
public:

	
	string IdentifierStr;
	char CharVal;
	string StrVal;
	double NumVal;
	

	int gettok();



private:
	//int LastChar;

	int readNum(string NumStr, int* LastChar);
	bool issymble(int LastChar);
	int readStr(bool isStr, int* LastChar);
};