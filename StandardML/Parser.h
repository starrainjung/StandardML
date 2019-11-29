/*---------------------------------------------------
	the head file of Parser	
---------------------------------------------------*/

#ifndef PARSER_H

#define PARSER_H

#include "Utility.h"
#include "Lexer.h"

class Lexer;

class Parser {

	int CurTok;
	map<int, int> BinopPrecedence;

	int GetTokPrecedence();

	// get the value of lexer member
	const string& getIdentifierStr() { return lexer.getIdentifierStr(); }
	const char& getCharVal() { return lexer.getCharVal(); }
	const string& getStrVal() { return lexer.getStrVal(); }
	const double& getNumVal() { return lexer.getNumVal(); }
	const bool& getBoolVal() { return lexer.getBoolVal(); }

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
	unique_ptr<FunctionDecAST> ParseFunction();

	/// Value ::= tok_val Patt '=' Expression
	unique_ptr<ValueDecAST> ParseValue();

	/// Declaration ::= Function
	///				::= Value
	unique_ptr<DecAST> ParseDeclaration();

	/*-------------------------------------------
					  pattern
	-------------------------------------------*/

	/// SinglePatt ::= id
	unique_ptr<PattAST> ParseSinglePatt();

	/// MutiPatt ::= '(' ')' | '(' Patt (',' Patt)* ')'
	unique_ptr<PattAST> ParseParenPatt();

	/// Patt ::= singlePatt (':' Type)
	///		 ::= MutiPatt (':' Type)
	unique_ptr<PattAST> ParsePatt();

	/*-------------------------------------------
					  type
	-------------------------------------------*/

	/// SingleType ::= bool | int | real | char | string | unit
	TypeAST* ParseSingleType();

	/// ParenType ::= '(' Type ')'
	TypeAST* ParseParenType();

	/// PrimaryType ::= SingleType
	///				::= ParenType
	TypeAST* ParsePrimaryType();

	/// Type ::= PrimaryType ('*' Type)*
	TypeAST* ParseType();

	void HandleDeclaration();
	void HandleTopLevelExpression();


public:

	Lexer lexer;
	void MainLoop();
	void resetLastChar() { lexer.resetLastChar(); }
	int getNextToken() { return CurTok = lexer.gettok(); }
	void InitializeModuleAndPassManager();
	Parser() {
		this->BinopPrecedence = {
	   {tok_andalso,10}, {tok_orelse,10}, {'<',20}, {'>',20}, {'<>',20}, {'<=',20},
	   {'>=',20}, {'=', 20}, {'+',30}, {'-',30}, {'^',30}, {'*',40}, {tok_div,40}, {'/',40}
		};
		this->CurTok = 0;
	}
};

#endif // !PARSER_H
