/*---------------------------------------------------
	the entrcance of the program StandardML
	the main fucntion lays below
---------------------------------------------------*/
#include "Lexer.h"





int main() {

	Lexer lexer;
	while (true) {

		fprintf(stdout, "%d ",lexer.gettok());
		
	}
}

