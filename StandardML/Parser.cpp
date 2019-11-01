/*---------------------------------------------------
	this file implements the Parser


---------------------------------------------------*/

#include "Parser.h"



int Parser::getNextToken() {
	return lexer.gettok();
}