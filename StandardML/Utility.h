/*---------------------------------------------------
	this file contains the common tools

	TODO: 
		1.error handle(when error, withdraw class and code genarate)
		2.same name handle(cover)
		3.type of patt or expr
	NOTE:
		1.default datatype: 
			tuple(include unit) record bool int real char string
		2.default Binary operators: 
			bool: andalso orelse = <> (notº¯Êý?)
			int: + - * div mod < > = <> >= <=
			real: + - * / < > = <> >= <=
			char: = <> <= >=
			string: ^ = <> <= >=
---------------------------------------------------*/

#pragma once
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

using namespace llvm;
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
	tok_div = -55,
	tok_mod = -57,

};

namespace {

	/*-------------------------------------------
		Abstract Syntax Tree (aka Parse Tree)
	-------------------------------------------*/

	/*----------------father class----------------*/

	///TypeAST - type of single pattern or single expression
	///		   - type of multiple pattern or paren expression			
	class TypeAST {
	protected:
		string TypeName;
		vector<unique_ptr<TypeAST>> Contents;
	public:
		TypeAST(const string& TypeName, vector<unique_ptr<TypeAST>> Contents)
			: TypeName(TypeName), Contents(Contents) {}
		virtual ~TypeAST() = default;
		bool FitType(const TypeAST* Type);
		const string& getTypeName() { return TypeName; }
	};

	/// DecAST - Base class for all declaration nodes
	class DecAST {
	public:
		virtual ~DecAST() = default;
	};

	/// ExprAST - Base class for all expression nodes.
	class ExprAST {
	protected:
		
	public:
		unique_ptr<TypeAST> ExprType = nullptr;
		virtual ~ExprAST() = default;
		virtual Value* codegen() = 0;
	};

	/// PattAST = Base class for all pattern nodes
	class PattAST {
	protected:
		
	public:
		unique_ptr<TypeAST> PattType = nullptr;
		virtual ~PattAST() = default;
	};


	/*-------------------------------------------
					expression
	-------------------------------------------*/

	/*----------------Constant Expression----------------*/
	class BoolExprAST : public ExprAST {
		bool BoolVal;
	public:
		BoolExprAST(bool BoolVal) : BoolVal(BoolVal) {}
		Value* codegen() override;
	};

	class IntExprAST : public ExprAST {
		int IntVal;
	public:
		IntExprAST(int IntVal) : IntVal(IntVal) {}
		Value* codegen() override;
	};

	class RealExprAST : public ExprAST {
		double RealVal;
	public:
		RealExprAST(double RealVal) : RealVal(RealVal) {}
		Value* codegen() override;
	};

	class CharExprAST : public ExprAST {
		char CharVal;
	public:
		CharExprAST(char CharVal) : CharVal(CharVal) {}
		Value* codegen() override;
	};

	class StringExprAST : public ExprAST {
		string StrVal;
	public:
		StringExprAST(const string& StrVal) : StrVal(StrVal) {}
		Value* codegen() override;
	};

	/*----------------variable, Paren, Call and Let----------------*/

	/// VariableExprAST - name of value 
	class VariableExprAST : public ExprAST {
		string VariName;
	public:
		VariableExprAST(const string& VariName) : VariName(VariName) {}
		Value* codegen() override;
	};

	/// ParenExprAST - '(' expr ',' expr (',' expr)* ')'
	///				 - '(' ')'
	class ParenExprAST : public ExprAST {
		vector<unique_ptr<ExprAST>> Contents;
	public:
		ParenExprAST(vector<unique_ptr<ExprAST>> Contents)
			: Contents(move(Contents)) {}
		Value* codegen() override;
	};

	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		string Callee;
		unique_ptr<ExprAST> Args;
	public:
		CallExprAST(const string& Callee, unique_ptr<ExprAST> Args)
			: Callee(Callee), Args(move(Args)) {}
		Value* codegen() override;
	};

	/// LetExprAST - let dec in expr end
	class LetExprAST : public ExprAST {
		unique_ptr<DecAST> LetDec;
		unique_ptr<ExprAST> InExpr;
	public:
		LetExprAST(unique_ptr<DecAST> LetDec, unique_ptr<ExprAST> InExpr)
			: LetDec(move(LetDec)), InExpr(move(InExpr)) {}
		Value* codegen() override;
	};

	/*----------------Binary and If Expression----------------*/
	/// BinaryExprAST - Expression class for a binary operator.
	/// include default binary(infix) operators and user-defined binary operators
	class BinaryExprAST : public ExprAST {
		int Op;
		unique_ptr<ExprAST> LHS, RHS;
	public:
		BinaryExprAST(const int& Op, unique_ptr<ExprAST> LHS,
			unique_ptr<ExprAST> RHS)
			: Op(Op), LHS(move(LHS)), RHS(move(RHS)) {}
		Value* codegen() override;
	};

	/// IfExprAST - if expr then expr else expr
	class IfExprAST : public ExprAST {
		unique_ptr<ExprAST> IfExpr, ThenExpr, ElseExpr;
	public:
		IfExprAST(unique_ptr<ExprAST> IfExpr, unique_ptr<ExprAST> ThenExpr,
			unique_ptr<ExprAST> ElseExpr)
			: IfExpr(move(IfExpr)), ThenExpr(move(ThenExpr)), ElseExpr(move(ElseExpr)) {}
		Value* codegen() override;
	};


	/*-------------------------------------------
					declaration
	-------------------------------------------*/

	/*----------------value----------------*/
	/// ValueDecAST - name a expression value
	class ValueDecAST : public DecAST {
		unique_ptr<PattAST> ValPatt;
		unique_ptr<ExprAST> ValExpr;
	public:
		ValueDecAST(unique_ptr<PattAST> ValPatt, unique_ptr<ExprAST> ValExpr)
			: ValPatt(move(ValPatt)), ValExpr(move(ValExpr)) {}
	};


	/*----------------function----------------*/
	/// FunctionDecAST - This class represents a function definition itself.
	class FunctionDecAST : public DecAST {
		string FuncName;
		unique_ptr<PattAST> FuncPatt;
		unique_ptr<ExprAST> FuncBody;
		unique_ptr<TypeAST> RetType;
	public:
		FunctionDecAST(const string& FuncName, unique_ptr<PattAST> FuncPatt,
			unique_ptr<ExprAST> FuncBody, unique_ptr<TypeAST> RetType = nullptr)
			: FuncName(FuncName), FuncPatt(move(FuncPatt)), FuncBody(move(FuncBody)), RetType(move(RetType)) {}
		Function* codegen();
	};


	/*-------------------------------------------
					  pattern
	-------------------------------------------*/

	/// SinglePattAST - pattern of single Variable
	class SinglePattAST : public PattAST {
		string VariName;
	public:
		SinglePattAST(const string& VariName) : VariName(VariName) {}
	};

	/// MultiplePattAST - pattern of multiple variable
	class MultiplePattAST : public PattAST {
		vector<unique_ptr<PattAST>> Contents;
	public:
		MultiplePattAST(vector<unique_ptr<PattAST>> Contents)
			: Contents(move(Contents)) {}
	};


	/*-------------------------------------------
					Error Handle
	-------------------------------------------*/

	//==----------------------------------------------------==//
	//	NOTE: three other ways of clean buffer
	//	 1.fflush(stdin);
	//	 2.scanf("%*[^\n]%*c");
	//	 3.int tmpBuf;
	//	   while ((tmpBuf = getchar()) != '\n' && tmpBuf != EOF);
	//==----------------------------------------------------==//

	int LexerError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		fprintf(stdout, "Lexer Error: %s\n", info);
		return tok_error;
	}

	

	Value* CodeGenError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		fprintf(stdout, "CodeGen Error: %s\n", info);
		return nullptr;
	}

}
