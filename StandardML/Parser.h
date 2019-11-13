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
	map<char, int> BinopPrecedence;
	int getNextToken();
	int GetTokPrecedence();

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
	/// VariableExpr
	///   ::= Variable
	/// NOTE: function call lays below
	///   ::= Variable Expression
	///   ::= Variable '(' ')'
	///   ::= Variable '(' Expression (',' Expression)* ')' 
	unique_ptr<ExprAST> ParseVariableExpr();

	/*---------------If and Let Expression--------------*/
	/// IfExpr ::= tok_if Expression tok_then Expression tok_else Expression
	unique_ptr<ExprAST> ParseIfExpr();
	/// LetExpr ::= tok_let Declartion tok_in Expression (';' Expression)* tok_end
	unique_ptr<ExprAST> ParseLetExpr();

	/*------------Paren and Primary Expression-------------*/
	/// ParenExpr ::= '(' Expression ')'
	unique_ptr<ExprAST> ParseParenExpr();
	/// PrimaryExpr
	///   ::= VariableExpr
	///   ::= ConstExpr
	///   ::= LetExpr
	///   ::= ParenExpr
	unique_ptr<ExprAST> ParsePrimaryExpr();
	/*----------------Binary Expression----------------*/
	/// BinaryExpr
	///   ::= (Op PrimaryExpr)*	
	unique_ptr<ExprAST> ParseBinaryExpr();

	/*--------------Expression(top level)--------------*/
	/// Expression 
	///   ::= IfExpr
	///   ::= PrimaryExpr BinaryExpr
	unique_ptr<ExprAST> ParseExpression();


	/*-------------------------------------------
					declaration
	-------------------------------------------*/
	/*----------------Function----------------*/
	/// Prototype
	///   ::= id '(' ')'
	///   ::= id '(' id ( ',' id )* ')'
	///   ::= id id
	unique_ptr<PrototypeAST> ParsePrototype();
	/// Function ::= tok_fun Prototype '=' Expression
	unique_ptr<DecAST> ParseFunction();

	/*-----------------value-----------------*/
	/// Value ::= tok_val id '=' Expression
	unique_ptr<DecAST> ParseValue();
};

