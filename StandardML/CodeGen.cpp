#include "Utility.h"

static LLVMContext TheContext;
static IRBuilder<> Builder(TheContext);
static unique_ptr<Module> TheModule;
static map<string, Value*> NamedValues;

bool TypeAST::FitType(const TypeAST* Type) {
	//type name is not equal
	if (this->TypeName != Type->TypeName)
		return false;
	//check contents in tuple type
	if (this->TypeName == "tuple") {
		//check contents size
		if (this->Contents.size() != Type->Contents.size())
			return false;
		//check every contents
		for (unsigned i = 0; i < this->Contents.size(); i++)
			if (!Contents[i]->FitType(Type))
				return false;
	}
	// if fit return true
	return true;
}

/*-------------------------------------------
		  CodeGen for expression
-------------------------------------------*/

Value* BoolExprAST::codegen(){
	if (BoolVal)
		return ConstantInt::getTrue(TheContext);
	else
		return ConstantInt::getFalse(TheContext);
}

Value* IntExprAST::codegen(){
	return ConstantInt::get(TheContext, APInt(32,IntVal));
}

Value *RealExprAST::codegen(){
  return ConstantFP::get(TheContext, APFloat(RealVal));
}

Value* CharExprAST::codegen(){
	return ConstantInt::get(TheContext, APInt(8,CharVal));
}

Value *StringExprAST::codegen() {
	return;
}

Value* VariableExprAST::codegen() {
	// Look this variable up in the function.
	Value* V = NamedValues[VariName];
	if (!V)
		return CodeGenError("Unknown variable name");
	return V;
}

Value* BinaryExprAST::codegen() {
	if (!LHS->ExprType->FitType(RHS->ExprType.get()))
		return CodeGenError("type not fetch in binary expression");
	
	string TypeName = LHS->ExprType->getTypeName();
	Value* L = LHS->codegen();
	Value* R = RHS->codegen();
	if (!L || !R)
		return nullptr;

	switch (Op) {
	case:
		
	}

	

}

Value* CallExprAST::codegen() {
	Function* CalleeF = TheModule->getFunction(Callee);
	if (!CalleeF)
		return CodeGenError("Unknown function referenced");

	if (CalleeF->arg_size() != Args.size())
		return CodeGenError("Incorrect arguments passed");

	vector<Value*> ArgsV;
	for (unsigned i = 0, ArgSize = Args.size(); i != ArgSize; ++i) {
		ArgsV.push_back(Args[i]->codegen());
		if (!ArgsV.back())
			// if no args pass None
			return Builder.CreateCall(CalleeF, None, "calltmep");
	}

	return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}


/*-------------------------------------------
		  CodeGen for declaration
-------------------------------------------*/



Function* FunctionDecAST::codegen() {
	if (Function* exitFunction = TheModule->getFunction(FuncName)) {
		TheModule->
	}



}