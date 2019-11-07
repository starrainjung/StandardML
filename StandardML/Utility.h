/*---------------------------------------------------
	this file contains the common tools

	TODO：1.出错处理（错误归类；tok_error直接清空键盘缓冲区）

	NOTE: 1.default Binary oprerators: div mod andalso orelse
		  2.default datatype: bool int real real char string(can be defined as variable)

---------------------------------------------------*/

#pragma once
#include<memory>
#include<string>
#include<vector>
#include<map>
using namespace std;

// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {

	//exit
	tok_exit = -1,
	//error
	tok_error = -2,

	// identify
	tok_identifier = -4,

	// datatype of constant
	tok_bool = -10,
	tok_int = -11,
	tok_real = -12,
	tok_char = -13,
	tok_string = -14,

	// keywords
	tok_val = -15,
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

};



namespace {

	// error handle
	void LexerError(const char* info) {
		// clean buffs of keyboard

		fprintf(stdout, "Lexer Error: %s\n", info);
	}

	unique_ptr<ExprAST> ParserError(const char* info) {
		// clean buffs of keyboard

		fprintf(stdout, "Lexer Error: %s\n", info);
		return nullptr;
	}

	 //Abstract Syntax Tree (aka Parse Tree)

	/*----------------father class----------------*/
	/// DecAST - Base class for all declaration
	class DecAST {
	public:
		virtual ~DecAST() = default;
	};

	/// ExprAST - Base class for all expression nodes.
	class ExprAST {
	public:
		virtual ~ExprAST() = default;
	};

	///TypeAST - Base class for all type
	/*class TypeAST {
		string TypeName;
	public:
		TypeAST(const string& TypeName) : TypeName(TypeName) {}
		virtual ~TypeAST() = default;
	};*/

	/*-------------------------------------------
					expression
	-------------------------------------------*/

	/*----------------constant----------------*/
	class BoolExprAST : public ExprAST {
		bool BoolVal;
	public:
		BoolExprAST(bool BoolVal) : BoolVal(BoolVal) {}
	};
	class IntExprAST : public ExprAST {
		int IntVal;
	public:
		IntExprAST(int IntVal) : IntVal(IntVal) {}
	};
	class RealExprAST : public ExprAST {
		double RealVal;
	public:
		RealExprAST(double RealVal) : RealVal(RealVal) {}
	};
	class CharExprAST : public ExprAST {
		char CharVal;
	public:
		CharExprAST(char CharVal) : CharVal(CharVal) {}
	};
	class StringExprAST : public ExprAST {
		string StrVal;
	public:
		StringExprAST(const string &StrVal) : StrVal(StrVal) {}
	};

	/*----------------variable and call----------------*/
	/// VariableExprAST - name of value 
	class VariableExprAST : public ExprAST {
		string VariName;
	public:
		VariableExprAST(const string &VariName) : VariName(VariName) {}
	};
	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		string Callee;
		vector<unique_ptr<ExprAST>> Args;

	public:
		CallExprAST(const string& Callee, vector<unique_ptr<ExprAST>> Args)
			: Callee(Callee), Args(move(Args)) {}
	};

	/*----------------If and Let Expression----------------*/
	/// IfExprAST - if expr then expr else expr
	class IfExprAST : public ExprAST {
		unique_ptr<ExprAST> IfExpr, ThenExpr, ElseExpr;
	public:
		IfExprAST(unique_ptr<ExprAST> IfExpr, unique_ptr<ExprAST> ThenExpr,
			unique_ptr<ExprAST> ElseExpr)
			: IfExpr(move(IfExpr)), ThenExpr(move(ThenExpr)), ElseExpr(move(ElseExpr)) {}

	};
	/// LetExprAST - let dec in expr(;expr)* end
	class LetExprAST : public ExprAST {
		unique_ptr<DecAST> LetDec;
		vector<unique_ptr<ExprAST>> InExprs;
	public:
		LetExprAST(unique_ptr<DecAST> LetDec, vector<unique_ptr<ExprAST>> InExprs)
			: LetDec(move(LetDec)), InExprs(move(InExprs)) {}
	};

	/// BinaryExprAST - Expression class for a binary operator.
	/// include default binary(infix) operators and user-defined binary operators
	class BinaryExprAST : public ExprAST {
		string Op;
		unique_ptr<ExprAST> LHS, RHS;
	public:
		BinaryExprAST(const string &Op, unique_ptr<ExprAST> LHS,
			unique_ptr<ExprAST> RHS)
			: Op(Op), LHS(move(LHS)), RHS(move(RHS)) {}
	};

	/*-------------------------------------------
					declaration
	-------------------------------------------*/

	/*----------------function----------------*/
	/// PrototypeAST - This class represents the "prototype" for a function,
	/// which captures its name, and its argument names (thus implicitly the number
	/// of arguments the function takes).
	class PrototypeAST {
		string FuncName;
		vector<string> Args;

	public:
		PrototypeAST(const string& Name, vector<string> Args)
			: FuncName(Name), Args(move(Args)) {}

		const string& getName() const { return FuncName; }
	};

	/// FunctionDecAST - This class represents a function definition itself.
	class FunctionDecAST : public DecAST {
		unique_ptr<PrototypeAST> Proto;
		unique_ptr<ExprAST> Body;

	public:
		FunctionDecAST(unique_ptr<PrototypeAST> Proto,
			unique_ptr<ExprAST> Body)
			: Proto(move(Proto)), Body(move(Body)) {}
	};

	/*----------------value----------------*/
	/// ValueDecAST - name a expression value
	class ValueDecAST : public DecAST {
		string ValName;
		unique_ptr<ExprAST> ValExpr;
	public:
		ValueDecAST(const string& ValName, unique_ptr<ExprAST> ValExpr)
			: ValName(ValName), ValExpr(move(ValExpr)) {}
	};

}