/*---------------------------------------------------
	this file contains the common tools


---------------------------------------------------*/

#pragma once
#include<memory>
#include<string>
#include <vector>
using namespace std;

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {

	//exit
	tok_exit = -1,


	tok_error = -2,

	// primary
	tok_identifier = -4,
	tok_number = -5, 

	// keywords
	tok_val = -11,
	tok_real = -12,
	tok_int = -13,
	tok_char = -14,
	tok_string = -15,

	tok_fn = -16,
	tok_fun = -17,
	tok_functor = -55,
	tok_type = -18,
	tok_datatype = -19,
	tok_abstype = -20,
	tok_eqtype = -21,
	tok_withtype = -22,
	tok_handle = -23,
	tok_infix = -24,
	tok_nonfix = -25,
	tok_infixr = -26,
	tok_local = -27,
	tok_sharing = -28,
	tok_and = -29,
	tok_as = -30,
	tok_with = -31,
	tok_then = -32,
	tok_where = -33,
	tok_include = -34,
	tok_of = -35,
	tok_open = -36,
	tok_op = -37,
	tok_raise = -38,
	tok_rec = -39,
	tok_exception = -40,
	tok_andalso = -41,
	tok_orelse = -42,
	tok_do = -43,
	tok_while = -44,
	tok_if = -45,
	tok_else = -46,
	tok_case = -47,
	tok_let = -48,
	tok_in = -49,
	tok_end = -50,
	tok_sig = -51,
	tok_signature = -52,
	tok_struct = -53,
	tok_structure = -54,

	//Reserved special character
	/*tok_maohao = -55,
	tok_shuxian = -56,
	tok_denghao = -57,
	tok_dengHdaY = -58,
	tok_hengXdaY = -59,
	tok_jinghao = -60,
	tok_maoHdaY = -61,*/
	
	
};


// Abstract Syntax Tree (aka Parse Tree)
namespace {

	/// ExprAST - Base class for all expression nodes.
	class ExprAST {
	public:
		virtual ~ExprAST() = default;
	};

	/*----------constant----------*/

	/// NumberExprAST - Expression class for numeric literals like "1.0".
	class NumberExprAST : public ExprAST {
		double NumVal;
		
	public:
		NumberExprAST(double NumVal) : NumVal(NumVal) {}
	};

	class CharExprAST : public ExprAST {
		char CharVal;

	public:
		CharExprAST(char CharVal) : CharVal(CharVal) {}
	};

	class StringExprAST : public ExprAST {
		string StringVal;

	public:
		StringExprAST(string StringVal) : StringVal(StringVal) {}
	};

	/*-------------variable----------------*/

	/// VariableExprAST - Expression class for referencing a variable, like "a".
	class VariableExprAST : public ExprAST {
		string Name;

	public:
		VariableExprAST(const string& Name) : Name(Name) {}
	};

	/// BinaryExprAST - Expression class for a binary operator.
	class BinaryExprAST : public ExprAST {
		char Op;
		unique_ptr<ExprAST> LHS, RHS;

	public:
		BinaryExprAST(char Op, unique_ptr<ExprAST> LHS,
			unique_ptr<ExprAST> RHS)
			: Op(Op), LHS(move(LHS)), RHS(move(RHS)) {}
	};

	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		string Callee;
		vector<unique_ptr<ExprAST>> Args;

	public:
		CallExprAST(const string& Callee,
			vector<unique_ptr<ExprAST>> Args)
			: Callee(Callee), Args(move(Args)) {}
	};

	/// PrototypeAST - This class represents the "prototype" for a function,
	/// which captures its name, and its argument names (thus implicitly the number
	/// of arguments the function takes).
	class PrototypeAST {
		string Name;
		vector<string> Args;

	public:
		PrototypeAST(const string& Name, vector<string> Args)
			: Name(Name), Args(move(Args)) {}

		const string& getName() const { return Name; }
	};

	/// FunctionAST - This class represents a function definition itself.
	class FunctionAST {
		unique_ptr<PrototypeAST> Proto;
		unique_ptr<ExprAST> Body;

	public:
		FunctionAST(unique_ptr<PrototypeAST> Proto,
			unique_ptr<ExprAST> Body)
			: Proto(move(Proto)), Body(move(Body)) {}
	};



}