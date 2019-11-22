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


//BinopPrecedence['<'] = 10;

/*-------------------------------------------
				expression
-------------------------------------------*/


///constant
unique_ptr<ExprAST> Parser::ParseBoolenExpr()
{
	//bool
	auto Result = std::unique_ptr<BoolExprAST>();
	getNextToken();
	return Result;
}

unique_ptr<ExprAST> Parser::ParseNumberExpr()
{

	if (CurTok == tok_int) {
		auto Result = std::unique_ptr<IntExprAST>();
		getNextToken();
		return Result;
	}
	if (CurTok == tok_real) {
		auto Result = std::unique_ptr<RealExprAST>();
		getNextToken();
		return Result;
	}
}

unique_ptr<ExprAST> Parser::ParseStringExpr()
{
	
	if (CurTok == tok_char) {
		auto Result = std::unique_ptr<CharExprAST>();
		getNextToken();
		return Result;
	}
	if (CurTok == tok_string) {
		auto Result = std::unique_ptr<StringExprAST>();
		getNextToken();
		return Result;
	}
}

unique_ptr<ExprAST> Parser::ParseConstExpr()
{
	switch (CurTok)
	{
	case tok_bool:
		return ParseBoolenExpr();
	case tok_int:
		return ParseNumberExpr();
	case tok_real:
		return ParseNumberExpr();
	case tok_char:
		return ParseStringExpr();
	case tok_string:
		return ParseStringExpr();
	}
}


///()
unique_ptr<ExprAST> Parser::ParseParenExpr()
{
	getNextToken();//eat(

	auto V = ParseExpression();
	if (!V)
		return nullptr;
	if (CurTok != ')')
	{
		ParserError("expected ')'");
		return nullptr;
	}
	getNextToken();//eat)

}


///identifier    ??????????????????
unique_ptr<ExprAST> Parser::ParseVariableExpr()
{
	string IdName = IdentifierStr;

	getNextToken();  // eat identifier.

	if (CurTok != '(') // Simple variable ref
		return std::make_unique<VariableExprAST>(IdName);

	// Call.
	getNextToken();  // eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (1) {
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;

			if (CurTok != ',')
			{
				ParserError("Expected ')' or ',' in argument list");
				return nullptr;
			}
			getNextToken();
		}
	}

	// Eat the ')'.
	getNextToken();

	return std::make_unique<CallExprAST>(IdName, std::move(Args));
}


/// IfExpr ::= tok_if Expression tok_then Expression tok_else Expression
unique_ptr<ExprAST> Parser::ParseIfExpr()
{

#pragma region if

	getNextToken();//eat if

	if (CurTok != '(') 
		return ParseExpression();

	getNextToken();  //eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (1) {
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;
			getNextToken();
		}
	}
	getNextToken(); //eat )

#pragma endregion
	if (CurTok != tok_then) {
		ParserError("Expected 'then' ");
		return nullptr;
	}
#pragma region then
	getNextToken();//eat then

	if (CurTok != '(') 
		return ParseExpression();

	getNextToken();  //eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (1) {
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;
			getNextToken();
		}
	}
	getNextToken(); //eat )
#pragma endregion

	if (CurTok != tok_else) {
		ParserError("Expected 'else' ");
		return nullptr;
	}
#pragma region else
	getNextToken();//eat else

	if (CurTok != '(') 
		return ParseExpression();

	getNextToken();  //eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (1) {
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;
			getNextToken();
		}
	}
	getNextToken(); //eat )
#pragma endregion

}


/// LetExpr ::= tok_let Declartion tok_in Expression (';' Expression)* tok_end
unique_ptr<ExprAST> Parser::ParseLetExpr()
{
#pragma region let

	getNextToken();  // eat let

	if (CurTok != '(') 
		unique_ptr<DecAST>();

	// Call.
	getNextToken();  // eat (
	std::vector<std::unique_ptr<DecAST>> Args;
	if (CurTok != ')') {
		while (1) {
			if (auto Arg = ParseFunction())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;
			getNextToken();
		}
	}

	// Eat the ')'.
	getNextToken();
#pragma endregion

	if (CurTok != tok_in) {
		ParserError("Expected 'in' ");
		return nullptr;
	}
#pragma region in

	getNextToken();  // eat in

	if (CurTok != '(') // Simple variable ref
		return ParseExpression();

	// Call.
	getNextToken();  // eat (
	std::vector<std::unique_ptr<ExprAST>> Args;
	if (CurTok != ')') {
		while (1) {
			if (auto Arg = ParseExpression())
				Args.push_back(std::move(Arg));
			else
				return nullptr;

			if (CurTok == ')')
				break;

			if (CurTok != ';')
			{
				ParserError("Expected ')' or ',' in argument list");
				return nullptr;
			}
			getNextToken();
		}
	}

	// Eat the ')'.
	getNextToken();

#pragma endregion

	if (CurTok != tok_end) {
		ParserError("Expected 'end' ");
		return nullptr;
	}
}

///variable const let paren
unique_ptr<ExprAST> Parser::ParsePrimaryExpr()
{
	switch (CurTok) {
	default:
		ParserError("unknown token when expecting an expression");
		return nullptr;
	case tok_identifier:
		return ParseVariableExpr();
	case tok_bool:case tok_int:case tok_real:case tok_char:case tok_string:
		return ParseConstExpr();
	case tok_let:
		return ParseLetExpr();
	case '(':
		return ParseParenExpr();
	}
}


///   ::= (Op PrimaryExpr)*	
unique_ptr<ExprAST> Parser::ParseBinaryExpr(int ExprPrec,
	std::unique_ptr<ExprAST> LHS)
{
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40;
	BinopPrecedence['/'] = 40;

	while (true) {
		int TokPrec = GetTokPrecedence();

		if (TokPrec < ExprPrec)
			return LHS;

		int BinOp = CurTok;
		getNextToken(); // eat binop


		auto RHS = ParsePrimaryExpr();
		if (!RHS)
			return nullptr;

		int NextPrec = GetTokPrecedence();
		if (TokPrec < NextPrec) {
			RHS = ParseBinaryExpr(TokPrec + 1, std::move(RHS));
			if (!RHS)
				return nullptr;
		}

		LHS = make_unique<BinaryExprAST>(BinOp, std::move(LHS),
			std::move(RHS));
	}
}


///   ::= IfExpr
///   ::= PrimaryExpr BinaryExpr
unique_ptr<ExprAST> Parser::ParseExpression()
{
	if (CurTok == tok_if) {
		return ParseIfExpr();
	}
	else {
		auto LHS = ParsePrimaryExpr();
		if (!LHS)
			return nullptr;

		return ParseBinaryExpr(0, std::move(LHS));
	}
}





/*-------------------------------------------
					declaration
	-------------------------------------------*/
unique_ptr<DecAST> Parser::ParseFunction()
{
	return unique_ptr<DecAST>();
}
