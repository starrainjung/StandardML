#include "Utility.h"

static map<string, Value*> NamedValues;
static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

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
		for (unsigned i = 0, VecSize = this->Contents.size(); i < VecSize; i++)
			if (!Contents[i]->FitType(Type->Contents[i]))
				return false;
	}
	// if fit return true
	return true;
}

Function* getFunction(std::string Name) {
	// First, see if the function has already been added to the current module.
	if (auto* F = TheModule->getFunction(Name))
		return F;

	// If not, check whether we can codegen the declaration from some existing
	// prototype.
	auto FI = FunctionProtos.find(Name);
	if (FI != FunctionProtos.end())
		return FI->second->codegen();

	// If no existing prototype exists, return null.
	return nullptr;
}


/*-------------------------------------------
		  CodeGen for expression
-------------------------------------------*/

Value* BoolExprAST::codegen(){
	vector<Type*> Contents;
	auto type = llvm::make_unique<TypeAST>("bool", move(Contents));
	this->ExprType = type.get();
	TypeLoop.push_back(move(type));
	if (BoolVal)
		return ConstantInt::getTrue(TheContext);
	else
		return ConstantInt::getFalse(TheContext);
}

Value* IntExprAST::codegen(){
	vector<Type*> Contents;
	auto type = llvm::make_unique<TypeAST>("int", move(Contents));
	this->ExprType = type.get();
	TypeLoop.push_back(move(type));
	return ConstantInt::get(TheContext, APInt(32,IntVal));
}

Value *RealExprAST::codegen(){
	vector<Type*> Contents;
	auto type = llvm::make_unique<TypeAST>("real", move(Contents));
	this->ExprType = type.get();
	TypeLoop.push_back(move(type));
	return ConstantFP::get(TheContext, APFloat(RealVal));
}

Value* CharExprAST::codegen(){
	vector<Type*> Contents;
	auto type = llvm::make_unique<TypeAST>("char", move(Contents));
	this->ExprType = type.get();
	TypeLoop.push_back(move(type));
	return ConstantInt::get(TheContext, APInt(8,CharVal));
}

Value *StringExprAST::codegen() {
	vector<Type*> Contents;
	auto type = llvm::make_unique<TypeAST>("string", move(Contents));
	this->ExprType = type.get();
	TypeLoop.push_back(move(type));
	return Builder.CreateGlobalStringPtr(StringRef(StrVal),"str");
}

Value* VariableExprAST::codegen() {
	// Look this variable up in the function.
	if (NamedValues.count(VariName))
		return NamedValues[VariName];
	
	GlobalVariable* Val = TheModule->getGlobalVariable(VariName);
	if (Val)
		return Val;

	return CodeGenError("Unknown variable name");
}

Value* ParenExprAST::codegen() {
	if (Contents.size() == 0)
		return nullptr;
	

}

Value* CallExprAST::codegen() {
	Function* CalleeF = getFunction(Callee);
	if (!CalleeF)
		return CodeGenError("Unknown function referenced");

	if (CalleeF->arg_size() != Args.size())
		return CodeGenError("Incorrect arguments passed");

	vector<Value*> ArgsV;
	for (unsigned i = 0, ArgSize = Args.size(); i != ArgSize; ++i) {
		ArgsV.push_back(Args[i]->codegen());
		if (!ArgsV.back())
			return nullptr;
	}

	return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

Value* LetExprAST::codegen() {

}


Value* BinaryExprAST::codegen() {
	Value* L = LHS->codegen();
	Value* R = RHS->codegen();
	if (!L || !R)
		return nullptr;

	if (!LHS->ExprType->FitType(RHS->ExprType))
		return CodeGenError("type not fetch in binary expression");

	string TypeName = LHS->ExprType->getTypeName();
	
	vector<Type*> Contents;
	auto type = llvm::make_unique<TypeAST>(TypeName, move(Contents));
	this->ExprType = type.get();
	TypeLoop.push_back(move(type));

	switch (Op) {
	case '+':
		if (TypeName == "int")
			return Builder.CreateAdd(L, R, "addINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFAdd(L, R, "addFLOATtmp");
		else
			return CodeGenError("undefined operater(+) for type: ", TypeName.c_str());
	case '-':
		if (TypeName == "int")
			return Builder.CreateSub(L, R, "subINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFSub(L, R, "subFLOATtmp");
		else
			return CodeGenError("undefined operater(-) for type: ", TypeName.c_str());
	case '*':
		if (TypeName == "int")
			return Builder.CreateMul(L, R, "mulINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFMul(L, R, "mulFLOATtmp");
		else
			return CodeGenError("undefined operater(*) for type: ", TypeName.c_str());
	case tok_div:
		if (TypeName == "int")
			return Builder.CreateSDiv(L, R, "divINTtmp");
		else
			return CodeGenError("undefined operater(div) for type: ", TypeName.c_str());
	case '/':
		if (TypeName == "real")
			return Builder.CreateFDiv(L, R, "divFLOATtmp");
		else
			return CodeGenError("undefined operater(/) for type: ", TypeName.c_str());
	case '=':
		if (TypeName == "int")
			return Builder.CreateICmpEQ(L, R, "cmpINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFCmpUEQ(L, R, "cmpFLOATtmp");
		else
			return CodeGenError("undefined operater(=) for type: ", TypeName.c_str());
	case '<>':
		if (TypeName == "int")
			return Builder.CreateICmpNE(L, R, "cmpINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFCmpUNE(L, R, "cmpFLOATtmp");
		else
			return CodeGenError("undefined operater(<>) for type: ", TypeName.c_str());
	case '>':
		if (TypeName == "int")
			return Builder.CreateICmpSGT(L, R, "cmpINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFCmpUGT(L, R, "cmpFLOATtmp");
		else
			return CodeGenError("undefined operater(>) for type: ", TypeName.c_str());
	case '<':
		if (TypeName == "int")
			return Builder.CreateICmpSLT(L, R, "cmpINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFCmpULT(L, R, "cmpFLOATtmp");
		else
			return CodeGenError("undefined operater(<) for type: ", TypeName.c_str());
	case '>=':
		if (TypeName == "int")
			return Builder.CreateICmpSGE(L, R, "cmpINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFCmpUGE(L, R, "cmpFLOATtmp");
		else
			return CodeGenError("undefined operater(>=) for type: ", TypeName.c_str());
	case '<=':
		if (TypeName == "int")
			return Builder.CreateICmpSLE(L, R, "cmpINTtmp");
		else if (TypeName == "real")
			return Builder.CreateFCmpULE(L, R, "cmpFLOATtmp");
		else
			return CodeGenError("undefined operater(<=) for type: ", TypeName.c_str());
	case tok_andalso:
		if (TypeName == "bool")
			return Builder.CreateAnd(L, R, "andBOOLtmp");
		else
			return CodeGenError("undefined operater(andalso) for type: ", TypeName.c_str());
	case tok_orelse:
		if (TypeName == "bool")
			return Builder.CreateOr(L, R, "orBOOLtmp");
		else
			return CodeGenError("undefined operater(orelse) for type: ", TypeName.c_str());
	default:
		return CodeGenError("invalid binary operator");
	}
}

/// QEST : what if this is top level expression that not in a function?
/// TODO : handle this whichc istop level expression
Value* IfExprAST::codegen() {
	Value* CondV = IfExpr->codegen();
	if (!CondV)
		return nullptr;
	
	if (IfExpr->ExprType->getTypeName() != "bool")
		return CodeGenError("not bool value for if condition");

	Function* TheFunction = Builder.GetInsertBlock()->getParent();

	BasicBlock* ThenBB = BasicBlock::Create(TheContext, "then", TheFunction);
	BasicBlock* ElseBB = BasicBlock::Create(TheContext, "else");
	BasicBlock* MergeBB = BasicBlock::Create(TheContext, "ifcont");

	Builder.CreateCondBr(CondV, ThenBB, ElseBB);

	// Emit then value.
	Builder.SetInsertPoint(ThenBB);

	Value* ThenV = ThenExpr->codegen();
	if (!ThenV)
		return nullptr;

	Builder.CreateBr(MergeBB);
	// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
	ThenBB = Builder.GetInsertBlock();

	// Emit else block.
	TheFunction->getBasicBlockList().push_back(ElseBB);
	Builder.SetInsertPoint(ElseBB);

	Value* ElseV = ElseExpr->codegen();
	if (!ElseV)
		return nullptr;

	Builder.CreateBr(MergeBB);
	// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
	ElseBB = Builder.GetInsertBlock();

	// Emit merge block.
	TheFunction->getBasicBlockList().push_back(MergeBB);
	Builder.SetInsertPoint(MergeBB);
	PHINode* PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");

	PN->addIncoming(ThenV, ThenBB);
	PN->addIncoming(ElseV, ElseBB);
	return PN;
}


/*-------------------------------------------
		  CodeGen for declaration
-------------------------------------------*/

void PrototypeAST::setAgs(PattAST* patt) {
	if (patt->isSingle) {
		ArgsName.push_back(FuncPatt->getPattName());
		ArgsType.push_back(FuncPatt->PattType->getTypeName());
		return;
	}
	for (unsigned i = 0, VecSize = patt->Contents.size(); i < VecSize; i++)
		setAgs(patt->Contents[i].get());
}


Function* PrototypeAST::codegen() {
	std::vector<Type*> AgrsType;
	setAgs(FuncPatt.get());
	for (int i = 0, VecSize = ArgsType.size(); i < VecSize; i++) {
		if (ArgsType[i] == "unit")
			AgrsType.push_back(Type::getVoidTy(TheContext));
		else if (ArgsType[i] == "bool")
			AgrsType.push_back(Type::getInt1Ty(TheContext));
		else if (ArgsType[i] == "int")
			AgrsType.push_back(Type::getInt32Ty(TheContext));
		else if (ArgsType[i] == "real")
			AgrsType.push_back(Type::getDoubleTy(TheContext));
		else if (ArgsType[i] == "char")
			AgrsType.push_back(Type::getInt8Ty(TheContext));
		else if (ArgsType[i] == "string")
			return FuncCodeGenError("unsupported argument type(string) in function");
		else
			return FuncCodeGenError("unknown argument type in function");
	}
	
	Type* retType = nullptr;
	if (RetType->getTypeName() == "unit")
		retType = Type::getVoidTy(TheContext);
	else if (RetType->getTypeName() == "bool")
		retType = Type::getInt1Ty(TheContext);
	else if (RetType->getTypeName() == "int")
		retType = Type::getInt32Ty(TheContext);
	else if (RetType->getTypeName() == "real")
		retType = Type::getDoubleTy(TheContext);
	else if (RetType->getTypeName() == "char")
		retType = Type::getInt8Ty(TheContext);
	else if (RetType->getTypeName() == "string")
		return FuncCodeGenError("unsupported return type(string) in function");
	else
		return FuncCodeGenError("unknown return type in function");

	FunctionType* FT = FunctionType::get(retType, AgrsType, false);

	Function* F = Function::Create(FT, Function::ExternalLinkage, FuncName, TheModule.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto& Arg : F->args())
		Arg.setName(ArgsName[Idx++]);

	return F;
}

Function* FunctionDecAST::codegen() {
	auto& P = *Proto;
	FunctionProtos[Proto->getName()] = std::move(Proto);
	Function* TheFunction = getFunction(P.getName());
	if (!TheFunction)
		return nullptr;

	// Create a new basic block to start insertion into.
	BasicBlock* BB = BasicBlock::Create(TheContext, "entry", TheFunction);
	Builder.SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	NamedValues.clear();
	for (auto& Arg : TheFunction->args())
		NamedValues[Arg.getName()] = &Arg;

	if (Value* RetVal = FuncBody->codegen()) {
		// Finish off the function.
		Builder.CreateRet(RetVal);

		// Validate the generated code, checking for consistency.
		verifyFunction(*TheFunction);

		// Run the optimizer on the function.
		TheFPM->run(*TheFunction);

		return TheFunction;
	}

	// Error reading body, remove function.
	TheFunction->eraseFromParent();
	return nullptr;
}

Value* ValueDecAST::codegen() {
	if (!ValExpr)
		return CodeGenError("lack of value for val declaration");

	Value* Val = ValExpr->codegen();
	if (!Val)
		return nullptr;

	Value* var = Builder.CreateAlloca(Val->getType());
	Value* GlobalVari = new GlobalVariable((*TheModule), Val->getType(),
		false, GlobalValue::ExternalLinkage,Constant::getNullValue(Val->getType()),ValPatt->getPattName());
	Builder.CreateStore(Val, GlobalVari);
	return GlobalVari;
}