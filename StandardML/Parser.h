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

	int CurTok = 0;
	map<int, int> BinopPrecedence = {
		{tok_andalso,10}, {tok_orelse,10}, {'<',20}, {'>',20}, {'<>',20}, {'<=',20},
		{'>=',20}, {'=', 20}, {'+',30}, {'-',30}, {'^',30}, {'*',40}, {tok_div,40}, {'/',40}
	};
	int getNextToken() { return CurTok = lexer.gettok(); }
	int GetTokPrecedence();
	
	// get the value of lexer member
	const string& getIdentifierStr() { return lexer.getIdentifierStr; }
	const char& getCharVal() { return lexer.getCharVal; }
	const string& getStrVal() { return lexer.getStrVal; }
	const double& getNumVal() { return lexer.getNumVal; }
	const bool& getBoolVal() { return lexer.getBoolVal; }

	/*-------------------------------------------
					expression
	-------------------------------------------*/

	/*----------------constant----------------*/
	// parse bool value
	unique_ptr<ExprAST> ParseBoolenExpr();
	// parse int and real value
	unique_ptr<ExprAST> ParseNumberExpr();
	// parse char and string value
	unique_ptr<ExprAST> ParseStringExpr();
	// top level parser of constant
	unique_ptr<ExprAST> ParseConstExpr();

	/*------------Variable and function call------------*/
	/// VariableExpr ::= Variable
	///				 ::= Variable Primary
	unique_ptr<ExprAST> ParseVariableExpr();

	/*------------Paren and Let Expression-------------*/
	/// ParenExpr ::= '(' ')' | '(' Expression (',' Expression)* ')'
	unique_ptr<ExprAST> ParseParenExpr();
	/// LetExpr ::= tok_let Declartion tok_in Expression tok_end
	unique_ptr<ExprAST> ParseLetExpr();

	
	/// PrimaryExpr
	///   ::= VariableExpr
	///   ::= ConstExpr
	///   ::= LetExpr
	///   ::= ParenExpr
	unique_ptr<ExprAST> ParsePrimaryExpr();
	
	/// BinaryExpr ::= (Op PrimaryExpr)*	
	unique_ptr<ExprAST> ParseBinaryExpr(int ExprPrec,
		std::unique_ptr<ExprAST> LHS);

	/// IfExpr ::= tok_if Expression tok_then Expression tok_else Expression
	unique_ptr<ExprAST> ParseIfExpr();

	/*--------------Expression(top level)--------------*/
	/// Expression ::= IfExpr (':' Type)
	///			   ::= PrimaryExpr (BinaryExpr)* (':' Type)
	unique_ptr<ExprAST> ParseExpression();


	/*-------------------------------------------
					declaration
	-------------------------------------------*/

	/// Function ::= tok_fun id Patt (':' Type) '=' Expression
	unique_ptr<DecAST> ParseFunction();

	/// Value ::= tok_val Patt '=' Expression
	unique_ptr<DecAST> ParseValue();

	/// Declaration ::= Function
	///				::= Value
	unique_ptr<DecAST> ParseDeclaration();

	/*-------------------------------------------
					  pattern
	-------------------------------------------*/	

	/// SinglePatt ::= id
	unique_ptr<PattAST> ParseSinglePatt();

	/// MutiPatt ::= '(' ')' | '(' Patt (',' Patt)* ')'
	unique_ptr<PattAST> ParseMutiPatt();

	/// Patt ::= singlePatt (':' Type)
	///		 ::= MutiPatt (':' Type)
	unique_ptr<PattAST> ParsePatt();

	/*-------------------------------------------
					  type
	-------------------------------------------*/

	/// SingleType ::= bool | int | real | char | string | unit
	unique_ptr<TypeAST> ParseSingleType();
		
	/// ParenType ::= '(' Type ')'
	unique_ptr<TypeAST> ParseParenType();

	/// PrimaryType ::= SingleType
	///				::= ParenType
	unique_ptr<TypeAST> ParsePrimaryType();

	/// Type ::= PrimaryType ('*' PrimaryType)*
	unique_ptr<TypeAST> ParseType();

	/*-------------------------------------------
					Error Handle
	-------------------------------------------*/
	unique_ptr<ExprAST> ParserExprError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		fprintf(stdout, "Parser Error: %s\n", info);
		return nullptr;
	}

	unique_ptr<DecAST> ParserDecError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);	
		fprintf(stdout, "Parser Error: %s\n", info);
		return nullptr;
	}

	unique_ptr<PattAST> ParserPattError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);	
		fprintf(stdout, "Parser Error: %s\n", info);
		return nullptr;
	}

	unique_ptr<TypeAST> ParserTypeError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		fprintf(stdout, "Parser Error: %s\n", info);
		return nullptr;
	}
};

