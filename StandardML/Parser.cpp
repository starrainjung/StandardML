/*---------------------------------------------------
	this file implements the Parser
---------------------------------------------------*/

#include "Parser.h"



int Parser::getNextToken() {
	return lexer.gettok();
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

/*-------------------------------------------
				declaration
-------------------------------------------*/
/*----------------Function----------------*/
/// Prototype
///   ::= id '(' ')'
///   ::= id '(' id ( ',' id )* ')'
///   ::= id id
unique_ptr<PrototypeAST> Parser::ParsePrototype() {

	if (CurTok != tok_identifier) {
		ParserError("Expected function name in prototype");
		return nullptr;
	}

	std::string FuncName = lexer.IdentifierStr;
	std::vector<string> Args;

	getNextToken();//eat identifier

	if (CurTok != '(') {//
		ParserError("Expected '('in prototype");
		return nullptr;
	}

	while (getNextToken() == tok_identifier)
		Args.push_back(lexer.IdentifierStr);

	if (CurTok != ')') {//
			ParserError("Expected ')'in prototype");
			return nullptr;
		}//eat )
	return std::make_unique<PrototypeAST>(FuncName, Args);
}
/// Function ::= tok_fun Prototype '=' Expression
unique_ptr<DecAST> Parser::ParseFunction() {
	getNextToken();//eat def
	auto Proto = ParsePrototype();
	if (!Proto) return nullptr;

	if (CurTok != '=') {//
		ParserError("Expected '=' in declaration");
		return nullptr;
	}
	if (auto E = ParseExpression())
		return std::make_unique<FunctionDecAST>(std::move(Proto), std::move(E));
	return nullptr;
}

/*-----------------value-----------------*/
/// Value ::= tok_val id '=' Expression
unique_ptr<DecAST> Parser::ParseValue() {
	getNextToken();//eat val
	if (CurTok != tok_identifier) {
		ParserError("Expected identifier name in expression");
		return nullptr;
	}
	getNextToken();//eat identifier

	if (CurTok != '=') {
		ParserError("Expected '=' in prototype");
		return nullptr;
	}
	getNextToken();//eat =

	auto expr = ParseParenExpr();
	return nullptr;
}



//unique_ptr<exprast> parseidentifierexpr() {
//	std::string idname = lexer.identifierstr;
//
//	getnexttoken();
//
//	if (curtok != '(') {
//		return std::make_unique<variableexprast>(idname);
//	}
//
//	getnexttoken();
//	std::vector<std::unique_ptr<exprast>> args;
//	if (curtok != ')')
//	{
//		while (1) {
//			if (auto arg = parseexpression())
//				args.push_back(std::move(arg));
//			else
//				return nullptr;
//			if (curtok == ')')
//				break;
//			if (curtok != ',') {
//				parsererror("expected ')' or ',' in argument list");
//				return tok_error;
//			}
//			getnexttoken();
//		}
//	}
//	return std::make_unique<prototypeast>(idname, std::move(args));
//
//}


