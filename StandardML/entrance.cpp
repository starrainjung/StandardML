/*---------------------------------------------------
	the entrcance of the program StandardML
	the main fucntion lays below
---------------------------------------------------*/
#include "Utility.h"
#include "Parser.h"

int main() {

	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();

	Parser MainParser;

	// Prime the first token.
	fprintf(stderr, " - ");
	MainParser.getNextToken();

	TheJIT = llvm::make_unique<SMLJIT>();

	MainParser.InitializeModuleAndPassManager();

	// Run the main "interpreter loop" now.
	MainParser.MainLoop();

	return 0;
}

