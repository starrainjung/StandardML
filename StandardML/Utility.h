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

#ifndef UTILITY_H

#define UTILITY_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "SMLJIT.h"
#include "Token.h"

using namespace llvm;
using namespace llvm::orc;
using namespace std;

class Parser;

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
		vector<TypeAST*> Contents;
	public:
		TypeAST(const string& TypeName, vector<TypeAST*> Contents)
			: TypeName(TypeName), Contents(move(Contents)) {}
		virtual ~TypeAST() = default;
		bool FitType(const TypeAST* Type);
		const string& getTypeName() { return TypeName; }
	};

	/// PattAST - Base class for all pattern nodes
	class PattAST {
		string PattName;
	public:
		bool isSingle;
		vector<unique_ptr<PattAST>> Contents;
		TypeAST* PattType = nullptr;
		PattAST(const string& PattName, bool isSingle) : PattName(PattName), isSingle(isSingle) {}
		virtual ~PattAST() = default;
		const string& getPattName() { return PattName; }
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
		bool isSingle = 1;
		TypeAST* ExprType = nullptr;
		virtual ~ExprAST() = default;
		virtual Value* codegen() = 0;
	};

	/*-------------------------------------------
					expression
	-------------------------------------------*/

	/*----------------Constant Expression----------------*/
	/// only Constant can define type in Parser
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

	/// define type in codegen
	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		string Callee;
		vector<unique_ptr<ExprAST>> Args;
	public:
		CallExprAST(const string& Callee, vector<unique_ptr<ExprAST>> Args)
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
		Value* codegen();
	};

	/*----------------function----------------*/
	// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
	class PrototypeAST : public DecAST {
		std::string FuncName;
		unique_ptr<PattAST> FuncPatt;
		vector<string> ArgsName;
		vector<string> ArgsType;
		TypeAST* RetType;
	public:
		PrototypeAST(const std::string& FuncName, unique_ptr<PattAST> FuncPatt, TypeAST* RetType)
			: FuncName(FuncName), FuncPatt(move(FuncPatt)), RetType(RetType) {}
		Function* codegen();
		const std::string& getName() const { return FuncName; }
		void setAgs(PattAST* patt);
	};

	/// FunctionDecAST - This class represents a function definition itself.
	class FunctionDecAST : public DecAST {
		unique_ptr<PrototypeAST> Proto;
		unique_ptr<ExprAST> FuncBody;
	public:
		FunctionDecAST(unique_ptr<PrototypeAST> Proto, unique_ptr<ExprAST> FuncBody)
			: Proto(move(Proto)), FuncBody(move(FuncBody)) {}
		Function* codegen();
	};

	/*-------------------------------------------
							Global
		-------------------------------------------*/

	vector<unique_ptr<TypeAST>> TypeLoop;
	LLVMContext TheContext;
	IRBuilder<> Builder(TheContext);
	unique_ptr<Module> TheModule;
	std::unique_ptr<legacy::FunctionPassManager> TheFPM;
	std::unique_ptr<SMLJIT> TheJIT;

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

	unique_ptr<FunctionDecAST> ParserFuncDecError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		
		fprintf(stdout, "Parser Error: %s\n", info);
		return nullptr;
	}

	unique_ptr<ValueDecAST> ParserValDecError(const char* info) {
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

	TypeAST* ParserTypeError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		
		fprintf(stdout, "Parser Error: %s\n", info);
		return nullptr;
	}

	Value* CodeGenError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		
		fprintf(stdout, "CodeGen Error: %s\n", info);
		return nullptr;
	}

	Value* CodeGenError(const char* info, const char* extinfo) {
		// clean buffer of keyboard
		fflush(stdin);
		
		fprintf(stdout, "CodeGen Error: %s%s\n", info, extinfo);
		return nullptr;
	}

	Function* FuncCodeGenError(const char* info) {
		// clean buffer of keyboard
		fflush(stdin);
		
		fprintf(stdout, "CodeGen Error: %s\n", info);
		return nullptr;
	}
}

#endif // !UTILITY_H