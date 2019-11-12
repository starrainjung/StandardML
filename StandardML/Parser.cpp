/*---------------------------------------------------
	this file implements the Parser
---------------------------------------------------*/

#include "Parser.h"



int Parser::getNextToken() {
	return CurTok = lexer.gettok();
}

int Parser::GetTokPrecedence() {
	if (!isascii(CurTok))
		return -1; 

	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[CurTok];
	if (TokPrec <= 0)
		return -1;
	return TokPrec;
}

