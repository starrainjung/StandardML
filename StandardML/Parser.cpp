/*---------------------------------------------------
	this file implements the Parser
---------------------------------------------------*/
#include "Parser.h"

int Parser::GetTokPrecedence() {
	if (CurTok != tok_andalso && CurTok != tok_orelse && CurTok != tok_div &&
		CurTok != '>' && CurTok != '<>' && CurTok != '<=' && CurTok != '>=' &&
		CurTok != '=' && CurTok != '+' && CurTok != '-' && CurTok != '^' &&
		CurTok != '*' && CurTok != '<' && CurTok != '/')
		return -1;

	return BinopPrecedence[CurTok];
}

/*-------------------------------------------
				expression
-------------------------------------------*/

// parse bool value
unique_ptr<ExprAST> Parser::ParseBoolenExpr() {
	auto Result = llvm::make_unique<BoolExprAST>(getBoolVal());
	getNextToken(); // eat tok_bool
	return std::move(Result);
}

// parse int and real value
unique_ptr<ExprAST> Parser::ParseNumberExpr() {	
	if (CurTok == tok_int) {
		auto Result = llvm::make_unique<IntExprAST>((int)getNumVal());
		getNextToken(); // eat tok_int
		return std::move(Result);
	}
	else if (CurTok == tok_real) {
		auto Result = llvm::make_unique<RealExprAST>(getNumVal());	
		getNextToken(); // eat tok_real
		return std::move(Result);
	}
	else
		return nullptr;
}

// parse char and string value
unique_ptr<ExprAST> Parser::ParseStringExpr() {
	if (CurTok == tok_char) {
		auto Result = llvm::make_unique<CharExprAST>(getCharVal());
		getNextToken(); // eat tok_char
		return std::move(Result);
	}
	else if (CurTok == tok_string) {
		auto Result = llvm::make_unique<StringExprAST>(getStrVal());
		getNextToken(); // eat tok_string
		return std::move(Result);
	}
	else
		return nullptr;
}

// top level parser of constant
unique_ptr<ExprAST> Parser::ParseConstExpr() {
	switch (CurTok) {
	default: // actually is not necessary
		return ParserExprError("unknown token when expecting a constant");
	case tok_bool:
		return ParseBoolenExpr();
	case tok_int: case tok_real:
		return ParseNumberExpr();
	case tok_char: case tok_string:
		return ParseStringExpr();
	}
}

/// VariableExpr ::= Variable
///				 ::= Variable PrimaryExpr
unique_ptr<ExprAST> Parser::ParseVariableExpr() {
	string VariName = getIdentifierStr();
	vector<unique_ptr<ExprAST>> Args;
	getNextToken();  // eat identifier.

	//// make sure next is or not primary expression
	//if (CurTok != tok_identifier && CurTok != tok_bool &&
	//	CurTok != tok_int && CurTok != tok_real && CurTok != tok_char &&
	//	CurTok != tok_string && CurTok != tok_let && CurTok != '(')
	//	return llvm::make_unique<VariableExprAST>(VariName);

	if (CurTok != '(')
		return ParserExprError("lack of ( in function call");

	getNextToken(); // eat (
	if(CurTok == ')')
		return llvm::make_unique<CallExprAST>(VariName, std::move(Args));

	auto PrExpr = ParsePrimaryExpr();
	if (!PrExpr)
		// if error	
		return nullptr;

	if (CurTok == ',') {
		Args.push_back(move(PrExpr));
		while (CurTok == ',') {
			getNextToken(); // eat ,
			auto tmp = ParseExpression();
			if (!tmp)
				// if error or lack of expression
				return nullptr;
			Args.push_back(move(tmp));
		}
		if (CurTok != ')')
			return ParserExprError("coupled parenthses without ')'");
		getNextToken(); // eat )
		return llvm::make_unique<ParenExprAST>(std::move(Args));
	}

	return llvm::make_unique<CallExprAST>(VariName, std::move(Args));
}

/// ParenExpr ::= '(' ')' | '(' Expression (',' Expression)* ')'
unique_ptr<ExprAST> Parser::ParseParenExpr() {
	getNextToken(); // eat (
	vector<unique_ptr<ExprAST>> Contents;
	if (CurTok == ')') {
		getNextToken(); // eat )
		return llvm::make_unique<ParenExprAST>(std::move(Contents));
	}
	
	// CurTok not ), must expression
	auto expr = ParseExpression();
	if (!expr)
		// if error or lack of expression 
		return nullptr;

	if (CurTok == ',') {
		Contents.push_back(move(expr));
		while (CurTok == ',') {
			getNextToken(); // eat ,
			auto tmp = ParseExpression();
			if (!tmp)
				// if error or lack of expression
				return nullptr;
			Contents.push_back(move(tmp));
		}
		if (CurTok != ')')
			return ParserExprError("coupled parenthses without ')'");
		getNextToken(); // eat )
		return llvm::make_unique<ParenExprAST>(std::move(Contents));
	}

	if (CurTok != ')')
		return ParserExprError("coupled parenthses without ')'");
	getNextToken(); // eat )
	return move(expr);
}

/// LetExpr ::= tok_let Declartion tok_in Expression tok_end
unique_ptr<ExprAST> Parser::ParseLetExpr() {
	getNextToken();  // eat let
	auto dec = ParseDeclaration();
	if (!dec)
		// if error or lack of declaration
		return nullptr;
	
	if (CurTok != tok_in)
		return ParserExprError("lack of token(in) in LET expression");

	getNextToken();  // eat in
	auto expr = ParseExpression();
	if (!expr)
		// if error or lack of expression 
		return nullptr;

	if (CurTok != tok_end)
		return ParserExprError("lack of token(end) in LET expression");
	
	getNextToken(); // eat end
	return llvm::make_unique<LetExprAST>(std::move(dec),std::move(expr));
}

/// PrimaryExpr
///   ::= VariableExpr
///   ::= ConstExpr
///   ::= LetExpr
///   ::= ParenExpr
unique_ptr<ExprAST> Parser::ParsePrimaryExpr() {
	switch (CurTok) {
	case tok_identifier:
		return ParseVariableExpr();
	case tok_bool: case tok_int: case tok_real:
	case tok_char: case tok_string:
		return ParseConstExpr();
	case tok_let:
		return ParseLetExpr();
	case '(':
		return ParseParenExpr();
	default: // unknown token (lack of primary expression)
		return ParserExprError("unknown token when primary expression");
	}
}

///  BinaryExpr ::= (Op PrimaryExpr)*	
unique_ptr<ExprAST> Parser::ParseBinaryExpr(int ExprPrec,
	std::unique_ptr<ExprAST> LHS) {
	// If this is a binop, find its precedence.
	while (true) {
		int CurPrec = GetTokPrecedence();

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (CurPrec < ExprPrec)
			return move(LHS);

		// Okay, we know this is a binop.
		int BinOp = CurTok;
		getNextToken(); // eat binop

		// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimaryExpr();
		if (!RHS)
			// if error or lack of primary expression
			return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence();
		if (CurPrec < NextPrec) {
			RHS = ParseBinaryExpr(CurPrec + 1, std::move(RHS));
			if (!RHS)
				// if error
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS =
			llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
	}
}

/// IfExpr ::= tok_if Expression tok_then Expression tok_else Expression
unique_ptr<ExprAST> Parser::ParseIfExpr() {
	getNextToken(); // eat if
	auto exprIF = ParseExpression();
	if (!exprIF)
		// if error or lack of expression
		return nullptr;
	
	if(CurTok != tok_then)
		return ParserExprError("lack of token(then) in IF expression");

	getNextToken(); // eat then
	auto exprTHEN = ParseExpression();
	if(!exprTHEN)
		// if error or lack of expression
		return nullptr;

	if (CurTok != tok_else)
		return ParserExprError("lack of token(else) in IF expression");

	getNextToken(); // eat else
	auto exprElSE = ParseExpression();
	if (!exprElSE)
		// if error or lack of expression
		return nullptr;

	return llvm::make_unique<IfExprAST>(move(exprIF), move(exprTHEN), move(exprElSE));
}

/// Expression ::= IfExpr 
///			   ::= PrimaryExpr BinaryExpr
unique_ptr<ExprAST> Parser::ParseExpression() {
	switch (CurTok) {
	default: // unknown token (lack of expression)
		return ParserExprError("unknown token when expected expression");
	case tok_if:
		return ParseIfExpr();
	case tok_identifier: case tok_bool: case tok_int: case tok_real:
	case tok_char: case tok_string: case tok_let: case '(': {
		auto LHS = ParsePrimaryExpr();
		if (!LHS)
			// if error or lack of lack of primary expression
			return nullptr;
		return ParseBinaryExpr(0, std::move(LHS));
	}
	}
}
	

/*-------------------------------------------
				declaration
-------------------------------------------*/

/// Function ::= tok_fun id Patt : Type '=' Expression
unique_ptr<FunctionDecAST> Parser::ParseFunction() {
	getNextToken(); // eat fun
	if (CurTok != tok_identifier) // lack of identifier(function name)
		return ParserFuncDecError("lack of function name in function declaration");
	
	string FuncName = getIdentifierStr();
	getNextToken(); // eat identifier

	auto FuncPatt = ParsePatt();
	if (!FuncPatt)
		// if error or lack of pattern 
		return nullptr;

	if (CurTok != ':')
		return ParserFuncDecError("lack of return type in function declaration");

	getNextToken(); // eat :
	auto RetType = ParseType();
	if (!RetType)
		// if error or lack of type
		return nullptr;

	auto proto = llvm::make_unique<PrototypeAST>(FuncName, move(FuncPatt), RetType);
	if (CurTok != '=')
		return ParserFuncDecError("lack of token(=) in function declaration");

	getNextToken(); // eat =
	auto FuncBody = ParseExpression();
	if (!FuncBody)
		// if error or lack of expression
		return nullptr;
	
	return llvm::make_unique<FunctionDecAST>(move(proto), move(FuncBody));
}

/// Value ::= tok_val Patt '=' Expression
unique_ptr<ValueDecAST> Parser::ParseValue() {
	getNextToken(); // eat val
	auto ValPatt = ParsePatt();
	if (!ValPatt)
		// if error or lack of pattern
		return nullptr;

	if (CurTok != '=')
		return ParserValDecError("lack of token(=) in function declaration");

	getNextToken(); // eat = 
	auto ValExpr = ParseExpression();
	if (!ValExpr)
		// if error or lack of expression
		return nullptr;

	return llvm::make_unique<ValueDecAST>(move(ValPatt), move(ValExpr));
}

// Declaration ::= Function
///				::= Value
unique_ptr<DecAST> Parser::ParseDeclaration() {
	if (CurTok == tok_fun)
		return ParseFunction();
	else if (CurTok == tok_val)
		return ParseValue();
	else
		// unknown token
		return ParserDecError("unknown token when expected declaration");
}


/*-------------------------------------------
				  pattern
-------------------------------------------*/

/// SinglePatt ::= id : type
unique_ptr<PattAST> Parser::ParseSinglePatt() {
	string VariName = getIdentifierStr();
	getNextToken(); // eat identifier
	if(CurTok != ':')
		return ParserPattError("lack of type for pattern");

	getNextToken(); // eat :
	auto type = ParseType();
	if (!type)
		return nullptr;

	auto patt = llvm::make_unique<PattAST>(VariName, true);
	patt->PattType = type;
	return move(patt);
}

/// ParenPatt ::= '(' ')' | '(' Patt (',' Patt)* ')'
unique_ptr<PattAST> Parser::ParseParenPatt() {
	getNextToken(); // eat (
	vector<unique_ptr<PattAST>> PattContents;
	vector<TypeAST*> TypeContents;
	if (CurTok == ')') {
		getNextToken(); // eat )
		auto SinglePatt = llvm::make_unique<PattAST>("()", true);
		auto PattType = llvm::make_unique<TypeAST>("unit",move(TypeContents));
		SinglePatt->PattType = PattType.get();
		TypeLoop.push_back(PattType);
		return move(SinglePatt);
	}
	
	auto PrPatt = ParsePatt();
	if (!PrPatt)
		// if error or lack of pattern
		return nullptr;

	if (CurTok == ',') {
		TypeContents.push_back(PrPatt->PattType);
		PattContents.push_back(move(PrPatt));
		while (CurTok == ',') {
			auto tmp = ParsePatt();
			if (!tmp)
				// if error or lack of pattern
				return nullptr;
			TypeContents.push_back(tmp->PattType);
			PattContents.push_back(move(tmp));
		}
		if (CurTok != ')')
			return ParserPattError("coupled parenthses without ')'");
		getNextToken(); // eat )
		auto MultiPatt = llvm::make_unique<PattAST>("(muti)", false);
		MultiPatt->Contents = move(PattContents);
		auto Type = llvm::make_unique<TypeAST>("tuple", move(TypeContents));
		MultiPatt->PattType = Type.get();
		TypeLoop.push_back(Type);
		return move(MultiPatt);
	}

	if (CurTok != ')')
		return ParserPattError("coupled parenthses without ')'");
	getNextToken(); // eat )
	return move(PrPatt);
}

/// Patt ::= SinglePatt
///		 ::= MutiPatt
unique_ptr<PattAST> Parser::ParsePatt() {
	if (CurTok == tok_identifier)
		return ParseSinglePatt();
	if (CurTok == '(')
		return ParseParenPatt();
	// unknown token
	return ParserPattError("unknown token when expected pattern");
}

/*-------------------------------------------
				  type
-------------------------------------------*/

/// SingleType ::= bool | int | real | char | string | unit
TypeAST* Parser::ParseSingleType() {
	vector<TypeAST*> Contents;
	string TypeName = getIdentifierStr();
	getNextToken(); // eat single type
	auto type = llvm::make_unique<TypeAST>(TypeName, move(Contents));
	auto typeIndex = type.get();
	TypeLoop.push_back(move(type));
	return typeIndex;
}

/// ParenType ::= '(' Type ')'
TypeAST* Parser::ParseParenType() {
	getNextToken(); // eat (
	auto Type = ParseType();
	if (!Type)
		// if error or lack of type
		return nullptr;

	if (CurTok != ')')
		return ParserTypeError("coupled parenthses without ')'");
	
	getNextToken(); // eat )
	return Type;
}

/// PrimaryType ::= SingleType
///				::= ParenType
TypeAST* Parser::ParsePrimaryType() {
	if (CurTok == tok_identifier) {
		if (getIdentifierStr() == "bool" || getIdentifierStr() == "int" ||
			getIdentifierStr() == "real" || getIdentifierStr() == "char" ||
			getIdentifierStr() == "string" || getIdentifierStr() == "unit")
			return ParseSingleType();
		else
			return ParserTypeError("unknown type written");
	}
	else if (CurTok == '(')
		return ParseParenType();
	else
		return ParserTypeError("unknown token when expected type");
}

/// Type ::= PrimaryType ('*' Type)*
TypeAST* Parser::ParseType() {
	auto PrType = ParsePrimaryType();
	if (!PrType)
		// if error or lack of primary type
		return nullptr;

	if (CurTok == '*') {
		vector<TypeAST*> Contents;
		Contents.push_back(PrType);
		while (CurTok == '*') {
			getNextToken(); // eat *
			auto tmp = ParseType();
			if (!tmp)
				// error or lack of lack of primary type
				return nullptr;
			Contents.push_back(move(tmp));
		}
		auto type = llvm::make_unique<TypeAST>("tuple", move(Contents));
		TypeAST* typeIndex = type.get();
		TypeLoop.push_back(move(type));
		return typeIndex;
	}

	return PrType;
}

void Parser::InitializeModuleAndPassManager(){
	// Open a new module.
	TheModule = llvm::make_unique<Module>("JIT", TheContext);
	TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

	// Create a new pass manager attached to it.
	TheFPM = llvm::make_unique<legacy::FunctionPassManager>(TheModule.get());

	// Do simple "peephole" optimizations and bit-twiddling optzns.
	TheFPM->add(createInstructionCombiningPass());
	// Reassociate expressions.
	TheFPM->add(createReassociatePass());
	// Eliminate Common SubExpressions.
	TheFPM->add(createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	TheFPM->add(createCFGSimplificationPass());

	TheFPM->doInitialization();
}

void Parser::HandleDeclaration() {
	if (CurTok == tok_fun) {
		if (auto funcAST = ParseFunction()) {
			if (auto* funcIR = funcAST->codegen()) {
				fprintf(stderr, "Read function definition:");
				funcIR->print(errs());
				fprintf(stderr, "\n");
				TheJIT->addModule(std::move(TheModule));
				InitializeModuleAndPassManager();
			}
		}
		else
			// Skip token for error recovery.
			getNextToken();
	}
	if (CurTok == tok_val) {
		if (auto ValAST = ParseValue()) {
			if (auto* ValIR = ValAST->codegen()) {
				fprintf(stderr, "Read variable definition:");
				ValIR->print(errs());
				fprintf(stderr, "\n");
				TheJIT->addModule(std::move(TheModule));
				InitializeModuleAndPassManager();
			}
		}
		else
			// Skip token for error recovery.
			getNextToken();
	}
}

void Parser::HandleTopLevelExpression() {
	// Evaluate a top-level expression into an anonymous function.
	if (auto ExprAST = ParseExpression()) {
		if (ExprAST->codegen()) {
			// JIT the module containing the anonymous expression, keeping a handle so
			// we can free it later.
			auto H = TheJIT->addModule(std::move(TheModule));
			InitializeModuleAndPassManager();

			// Search the JIT for the __anon_expr symbol.
			auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
			assert(ExprSymbol && "Function not found");

			// Get the symbol's address and cast it to the right type (takes no
			// arguments, returns a double) so we can call it as a native function.
			double (*FP)() = (double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());
			fprintf(stderr, "val it = %f\n", FP());

			// Delete the anonymous expression module from the JIT.
			TheJIT->removeModule(H);
		}
	}
	else
		// Skip token for error recovery.
		getNextToken();
}

void Parser::MainLoop(){
	while (true) {
		fprintf(stderr, " - ");
		switch (CurTok) {
		case tok_exit:
			return;
		case ';': // ignore top-level semicolons.
			getNextToken();
			break;
		case tok_val: case tok_fun:
			HandleDeclaration();
			break;
		default:
			HandleTopLevelExpression();
			break;
		}
	}
}