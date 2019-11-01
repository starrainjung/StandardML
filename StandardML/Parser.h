/*---------------------------------------------------
	the head file of Parser

	
---------------------------------------------------*/

#pragma once
#include "Utility.h"
#include "Lexer.h"

class Parser
{
public:
	Lexer lexer;
	
	

private:
	int curToken = 0;

	int getNextToken();


};

