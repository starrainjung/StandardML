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
	getNextToken();
	return std::move(Result);
}

// parse int and real value
unique_ptr<ExprAST> Parser::ParseNumberExpr() {
	if (CurTok == tok_int) {
		auto Result = llvm::make_unique<IntExprAST>((int)getNumVal());
		getNextToken();
		return std::move(Result);
	}
	else if (CurTok == tok_real) {
		auto Result = llvm::make_unique<RealExprAST>(getNumVal());
		getNextToken();
		return std::move(Result);
	}
	else
		return nullptr;
}

// parse char and string value
unique_ptr<ExprAST> Parser::ParseStringExpr() {
	if (CurTok == tok_char) {
		auto Result = llvm::make_unique<CharExprAST>(getCharVal());
		getNextToken();
		return std::move(Result);
	}
	else if (CurTok == tok_string) {
		auto Result = llvm::make_unique<StringExprAST>(getStrVal());
		getNextToken();
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
	getNextToken();  // eat identifier.

	// make sure next is or not primary expression
	if (CurTok != tok_identifier && CurTok != tok_bool &&
		CurTok != tok_int && CurTok != tok_real && CurTok != tok_char &&
		CurTok != tok_string && CurTok != tok_let && CurTok != '(')
		return llvm::make_unique<VariableExprAST>(VariName);

	auto ActualArgs = ParsePrimaryExpr();
	if (!ActualArgs)
		// if error	
		return nullptr;

	return llvm::make_unique<CallExprAST>(VariName, std::move(ActualArgs));
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

///   ::= (Op PrimaryExpr)*	
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

/// Expression ::= IfExpr (':' Type)
///			   ::= PrimaryExpr (BinaryExpr)* (':' Type)
unique_ptr<ExprAST> Parser::ParseExpression() {
	switch (CurTok) {
	default: // unknown token (lack of expression)
		return ParserExprError("unknown token when expected expression");
	case tok_if: {
		auto IfExpr = ParseIfExpr();
		if(CurTok != ':')
			return move(IfExpr);
		else {
			getNextToken(); // eat :
			auto type = ParseType();
			if (!type)
				// if error or lack of type
				return nullptr;

		}

	}
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

/// Function ::= tok_fun id Patt (: Type) '=' Expression
unique_ptr<DecAST> Parser::ParseFunction() {
	unique_ptr<TypeAST> RetType = nullptr;
	getNextToken(); // eat fun
	if (CurTok != tok_identifier) // lack of identifier(function name)
		return ParserDecError("lack of function name in function declaration");
	
	string FuncName = getIdentifierStr();
	getNextToken(); // eat identifier

	auto FuncPatt = ParsePatt();
	if (!FuncPatt)
		// if error or lack of pattern 
		return nullptr;

	if (CurTok == ':') {
		getNextToken(); // eat :
		RetType = ParseType();
		if (!RetType)
			// if error or lack of type
			return nullptr;
	}

	if (CurTok != '=')
		return ParserDecError("lack of token(=) in function declaration");

	getNextToken(); // eat =
	auto FuncBody = ParseExpression();
	if (!FuncBody)
		// if error or lack of expression
		return nullptr;
	
	return make_unique<FunctionDecAST>(FuncName,move(FuncPatt),move(FuncBody),RetType);
	
}

/// Value ::= tok_val Patt '=' Expression
unique_ptr<DecAST> Parser::ParseValue() {
	getNextToken(); // eat val
	auto ValPatt = ParsePatt();
	if (!ValPatt)
		// if error or lack of pattern
		return nullptr;

	if (CurTok != '=')
		return ParserDecError("lack of token(=) in function declaration");

	getNextToken(); // eat = 
	auto ValExpr = ParseExpression();
	if (!ValExpr)
		// if error or lack of expression
		return nullptr;

	return make_unique<ValueDecAST>(move(ValPatt), move(ValExpr));
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

/// SinglePatt ::= id
unique_ptr<PattAST> Parser::ParseSinglePatt() {
	string VariName = getIdentifierStr();
	getNextToken(); // eat identifier
	return make_unique<SinglePattAST>(VariName);
}

/// MutiPatt ::= '(' ')' | '(' Patt (',' Patt)* ')'
unique_ptr<PattAST> Parser::ParseMutiPatt() {
	getNextToken(); // eat (
	vector<unique_ptr<PattAST>> PattContents;
	vector<unique_ptr<TypeAST>> TypeContents;
	if (CurTok == ')') {
		getNextToken(); // eat )
		auto SinglePatt = make_unique<MultiplePattAST>(move(PattContents));
		auto PattType = make_unique<TypeAST>("unit",move(TypeContents));
		SinglePatt->PattType = move(PattType);
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
		auto MultiPatt = make_unique<MultiplePattAST>(move(PattContents));
		auto Type = make_unique<TypeAST>("tuple", move(TypeContents));
		MultiPatt->PattType = move(Type);
		return move(MultiPatt);
	}

	if (CurTok != ')')
		return ParserPattError("coupled parenthses without ')'");
	getNextToken(); // eat )
	return move(PrPatt);
}

/// Patt ::= id (: Type)
///		 ::= '(' ')' | '(' id (',' id)* ')' (: Type)
unique_ptr<PattAST> Parser::ParsePatt() {
	if (CurTok == tok_identifier) {
		auto Patt = ParseMutiPatt();
		unique_ptr<TypeAST> PattType = nullptr;
		if (CurTok == ':') {
			PattType = ParseType();
			if (!PattType)
				// if error or lack of type
				return nullptr;
		}
		
	}
	if (CurTok == '(') {
		auto Patt = ParseSinglePatt();
		vector<unique_ptr<PattAST>> Contents;
		unique_ptr<TypeAST> PattType = nullptr;
		
	}
	// unknown token
	return ParserPattError("unknown token when expected pattern");
}

/*-------------------------------------------
				  type
-------------------------------------------*/

/// SingleType ::= bool | int | real | char | string | unit
unique_ptr<TypeAST> Parser::ParseSingleType() {
	vector<unique_ptr<TypeAST>> Contents;
	string TypeName = getIdentifierStr();
	getNextToken(); // eat single type
	return make_unique<TypeAST>(TypeName,move(Contents));
}

/// ParenType ::= '(' Type ')'
unique_ptr<TypeAST> Parser::ParseParenType() {
	getNextToken(); // eat (
	auto Type = ParseType();
	if (!Type)
		// if error or lack of type
		return nullptr;

	if (CurTok != ')')
		return ParserTypeError("coupled parenthses without ')'");
	
	getNextToken(); // eat )
	return move(Type);
}

/// PrimaryType ::= SingleType
///				::= ParenType
unique_ptr<TypeAST> Parser::ParsePrimaryType() {
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

/// Type ::= PrimaryType ('*' PrimaryType)*
unique_ptr<TypeAST> Parser::ParseType() {
	auto PrType = ParsePrimaryType();
	if (!PrType)
		// if error or lack of primary type
		return nullptr;

	if (CurTok == '*') {
		vector<unique_ptr<TypeAST>> Contents;
		Contents.push_back(move(PrType));
		while (CurTok == '*') {
			getNextToken(); // eat *
			auto tmp = ParsePrimaryType();
			if (!tmp)
				// error or lack of lack of primary type
				return nullptr;
			Contents.push_back(move(tmp));
		}
		return make_unique<TypeAST>("tuple", move(Contents));
	}

	return move(PrType);
}

