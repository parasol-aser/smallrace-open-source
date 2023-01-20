#include "st/MLIRGen.h"

#include <llvm/IR/IRBuilder.h>

#include <numeric>
#include <algorithm>

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ScopedHashTable.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/Verifier.h"
#include "st/ParserWrapper.h"
// #include "st/STDialect.h"
#include "st/STParserListener.h"
#include "st/ScopedHashTableX.h"
//#include "ast/AST.h"

using llvm::ArrayRef;
using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;
using llvm::makeArrayRef;
using llvm::ScopedHashTableScope;
using llvm::SmallVector;
using llvm::StringRef;
using llvm::Twine;

using namespace mlir;

const std::string ANON_FUNC_NAME = "st.anonfun.";
const std::string ANON_NAME = "anon";
int ANON_FUNC_INDEX = 0;
const std::string MAIN_FUNC_NAME = "main";
const std::string GLOBAL_VAR_NAME = "global_";
const std::string LOCAL_VAR_NAME = "local_";
const std::string GLOBAL_OP_NAME = "global_op_";

const std::string ST_BUILT_IN_NAME = "st.";  // for all the built-in st function
const std::string ST_BUILT_IN_MODEL_NAME = "st.model.";
const std::string ST_BUILT_IN_MODEL_NEW_TEMP = "st.model.newTemp";
const std::string ST_BUILT_IN_MODEL_NEW_OBJECT = "st.model.newObject";
const std::string ST_BUILT_IN_MODEL_NEW_OBJECT2 = "st.model.newObject2";
const std::string ST_BUILT_IN_MODEL_INST_VAR = "st.model.instVar";
const std::string ST_BUILT_IN_MODEL_CLASS_VAR = "st.model.classVar";
const std::string ST_BUILT_IN_MODEL_PARENT_SCOPE = "st.model.parentScope";
const std::string ST_BUILT_IN_MODEL_OPAQUE_ASSIGN = "st.model.opaqueAssign";
const std::string ST_BUILT_IN_MODEL_BINARY_OP = "st.model.binaryop";
const std::string ST_BUILT_IN_MODEL_BLOCK_PARAM = "st.model.blockParam";

const std::string ST_BUILT_IN_CLASS_NAME = "st.class.";
const std::string ST_BUILT_IN_CLASS_METADATA = "st.class.metadata";
const std::string ST_BUILT_IN_CLASS_SUPER = "st.class.super";

/// A "module" matches a Toy source file: containing a list of functions.
mlir::ModuleOp theModule;
LLVM::LLVMPointerType llvmI8PtrTy;
Type llvmVoidTy, llvmI32Ty, llvmI64Ty;

/// A mapping for the functions that have been code generated to MLIR.
llvm::StringMap<mlir::FuncOp> functionMap;

namespace st {

/// Implementation of a simple MLIR emission from the ST AST.
///
/// This will emit operations that are specific to the Toy language, preserving
/// the semantics of the language and (hopefully) allow to perform accurate
/// analysis and transformation based on these high level semantics.
class MLIRGenImpl {
 private:
  /// The builder is a helper class to create IR inside a function. The builder
  /// is stateful, in particular it keeps an "insertion point": this is where
  /// the next operations will be introduced.
  mlir::OpBuilder builder;

 public:
  MLIRGenImpl(mlir::MLIRContext &context) : builder(&context) {
    // We create an empty MLIR module and codegen functions one at a time and
    // add them to the module.
  }
  /// Public API: convert the AST for a Toy module (source file) to an MLIR
  /// Module operation.
  mlir::ModuleOp mlirGen(ModuleAST &moduleAST) {
    // Verify the module after we have finished constructing it, this will check
    // the structural properties of the IR and invoke any specific verifiers we
    // have on the Toy operations.

    theModule.dump();

    return theModule;
  }

 private:
  /// The symbol table maps a variable name to a value in the current scope.
  /// Entering a function creates a new scope, and the function arguments are
  /// added to the mapping. When the processing of a function is terminated, the
  /// scope is destroyed and the mappings created in this scope are dropped.
  llvm::ScopedHashTable<StringRef, std::pair<mlir::Value, const ExprAST *>>
      symbolTable;
  using SymbolTableScopeT =
      llvm::ScopedHashTableScope<StringRef,
                                 std::pair<mlir::Value, const ExprAST *>>;


  /// Emit a print expression. It emits specific operations for two builtins:
  /// transpose(x) and print(x).
  mlir::Value mlirGen(PrintExprAST &call) {
    //todo: implement codegen
    return nullptr;
  }
};
// The public API for codegen.
mlir::OwningModuleRef mlirGen(mlir::MLIRContext &context,
                              ModuleAST &moduleAST) {
  return MLIRGenImpl(context).mlirGen(moduleAST);
};
}  // namespace st

namespace stx {

class MLIRGenImpl {
 private:
  /// The builder is a helper class to create IR inside a function. The builder
  /// is stateful, in particular it keeps an "insertion point": this is where
  /// the next operations will be introduced.
  mlir::OpBuilder builder;

  std::map<std::string, ClassAST *> classesInfoMap;
  std::map<std::string, std::vector<std::string>> inst_varsClassMap;
  std::map<std::string, std::vector<std::string>> class_varsClassMap;

  std::vector<std::map<std::string, std::pair<llvm::StringRef, mlir::Value>> *>
      symbolTableStack;
  std::vector<FunctionAST*> astStack;
  Block *curBlock;

  llvm::ScopedHashTableX<StringRef, std::pair<StringRef, mlir::Value>>
      symbolTable2;
  using SymbolTableScopeT2 =
      llvm::ScopedHashTableXScope<StringRef, std::pair<StringRef, mlir::Value>>;
  std::vector<StringRef> functionStack;
  void pushNewFunction(StringRef funcName) {
    functionStack.push_back(funcName);
    auto map =
        new std::map<std::string, std::pair<llvm::StringRef, mlir::Value>>();
    symbolTableStack.push_back(map);
  }
  void popLastFunction(StringRef funcName) {
    if (functionStack.size() > 0 && functionStack.back().equals(funcName)) {
      functionStack.pop_back();

      auto map = symbolTableStack.back();
      symbolTableStack.pop_back();
      delete map;

    } else {
      llvm::outs() << "-----popLastFunction error------func: " << funcName
                   << "\n";
      // if functionStack is empty now, then clear all symbolTable
      // TODO: free memory
      // symbolTable2.clear();
      symbolTableStack.clear();
    }
  }
  llvm::StringRef findSymbolInParentScopes(std::string varName) {
    for (auto rit = symbolTableStack.rbegin() + 1;
         rit != symbolTableStack.rend(); ++rit) {
      std::map<std::string, std::pair<llvm::StringRef, mlir::Value>> &map =
          **rit;
      if (map.find(varName) != map.end()) {
        return map.at(varName).first;
      }
    }
    return "";
  }
  StringRef getCurrentFunctionName() {
    if (functionStack.size() > 0)
      return functionStack.back();
    else {
      llvm::outs()
          << "-----getCurrentFunctionName error------empty functionStack!!\n";
      return "st.error.stub";
    }
  }
  bool matchCurrentScopeFunction(StringRef funcName) {
    // make sure
    if (functionStack.size() == 0) {
      llvm::outs() << "-----matchCurrentScopeFunction error------\n";
      return false;
    }
    return functionStack.back().equals(funcName);
  }

  /// Helper conversion for a Toy AST location to an MLIR location.
  mlir::Location loc(Location loc) {
    auto fileName = loc.file;
    if (fileName.empty())
      llvm::outs() << "-----empty file name------"
                   << "\n";
    // else
    //   llvm::outs() << "-----loc file name: " << loc.file << "------"
    //                << "\n";

    return builder.getFileLineColLoc(builder.getIdentifier(fileName), loc.line,
                                     loc.col);
  }
  /// Helper conversion for a Toy AST location to an MLIR location.
  mlir::Location loc(mlir::OpBuilder &builder, Location loc) {
    if (loc.file.empty())
      llvm::outs() << "-----empty file name------"
                   << "\n";
    // else
    //   llvm::outs() << "-----loc file name: " << loc.file << "------"
    //                << "\n";

    return builder.getFileLineColLoc(builder.getIdentifier(loc.file), loc.line,
                                     loc.col);
  }
  Value getOrCreateGlobalStringX(mlir::Location loc, std::string name,
                                 StringRef value) {
    // Create the global at the entry of the module.
    LLVM::GlobalOp global;
    if (!(global = theModule.lookupSymbol<LLVM::GlobalOp>(name))) {
      //llvm::outs() << "-----adding GlobalString ------" << name << "\n";

      OpBuilder::InsertionGuard insertGuard(builder);
      builder.setInsertionPointToStart(theModule.getBody());
      //auto type = LLVM::LLVMType::getArrayTy(llvmI8PtrTy, value.size());
      auto type = LLVM::LLVMArrayType::get(llvmI8PtrTy, value.size());
      global = builder.create<LLVM::GlobalOp>(loc, type, /*isConstant=*/true,
                                              LLVM::Linkage::Internal, name,
                                              builder.getStringAttr(value));
    }

    // Get the pointer to the first character in the global string.
    Value globalPtr = builder.create<LLVM::AddressOfOp>(loc, global);
    // return globalPtr;

    Value cst0 = builder.create<LLVM::ConstantOp>(
        loc, llvmI64Ty,
        builder.getIntegerAttr(builder.getIndexType(), 0));
    return builder.create<LLVM::GEPOp>(
        loc, llvmI8PtrTy, globalPtr,
        ArrayRef<Value>({cst0, cst0}));
  }
  FlatSymbolRefAttr getOrInsertUserDefinedFunctionX(
      mlir::Location mlocation, llvm::StringRef name,
      SmallVector<mlir::Value, 4> operands) {
    // special call: st.new
    if (name.str() == "st.new") {
      name = ST_BUILT_IN_MODEL_NEW_OBJECT;
    } else if (name.str() == "st.new:") {
      name = ST_BUILT_IN_MODEL_NEW_OBJECT2;
    }

    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    // Create a function declaration for opaqueObject, the signature is:
    //   * `i32 (i8*, ...)`
    std::vector<mlir::Type> inputTypes;
    for (auto operand : operands) inputTypes.push_back(operand.getType());

    auto mlirFnType = mlir::FunctionType::get(context, inputTypes, llvmI8PtrTy);
    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto func = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);
    // declare only

    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertSuperClassFunctionX() {
    llvm::StringRef name = ST_BUILT_IN_CLASS_SUPER;
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    // Create a function declaration, the signature is:
    //   * `void (i8*,i8*)`
    auto mlirFnType = mlir::FunctionType::get(context, {llvmI8PtrTy, llvmI8PtrTy},
                                              llvmVoidTy);
    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto func = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);
    
    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertAssignFunctionX(llvm::StringRef name) {
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    // Create a function declaration, the signature is:
    //   * `void (i8*,i8*)`
    auto mlirFnType = mlir::FunctionType::get(context, {llvmI8PtrTy, llvmI8PtrTy},
                                              llvmVoidTy);
    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto func = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);

    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertForkAtFunctionX(llvm::StringRef name) {
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    // Create a function declaration for opaqueObject, the signature is:
    //   * `void (i8*, i32...)`
    auto mlirFnType =
        mlir::FunctionType::get(context, {llvmI8PtrTy, llvmI64Ty}, llvmI8PtrTy);
    if (name.endswith("named:"))
      mlirFnType = mlir::FunctionType::get(context, 
          {llvmI8PtrTy, llvmI64Ty, llvmI8PtrTy}, llvmI8PtrTy);

    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto func = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);

    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertBuiltInParentScopeX() {
    auto name = ST_BUILT_IN_MODEL_PARENT_SCOPE;
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    auto mlirFnType = mlir::FunctionType::get(context, {llvmI8PtrTy, llvmI8PtrTy},
                                              llvmI8PtrTy);

    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto newFunc = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);

    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertBuiltInNewFunctionX() {
    auto name = ST_BUILT_IN_MODEL_NEW_TEMP;
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    auto mlirFnType =
        mlir::FunctionType::get(context, {llvmI8PtrTy}, llvmI8PtrTy);

    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto newFunc = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);

    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertBuiltInInstVarFunctionX() {
    auto name = ST_BUILT_IN_MODEL_INST_VAR;
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    auto mlirFnType =
        mlir::FunctionType::get(context, {llvmI8PtrTy}, llvmI8PtrTy);

    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto newFunc = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);

    return SymbolRefAttr::get(name, context);
  }
  FlatSymbolRefAttr getOrInsertBuiltInClassVarFunctionX() {
    auto name = ST_BUILT_IN_MODEL_CLASS_VAR;
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);

    auto mlirFnType =
        mlir::FunctionType::get(context, {llvmI8PtrTy}, llvmI8PtrTy);

    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto newFunc = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);

    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertBuiltInBinaryOpFunctionX(
      SmallVector<mlir::Value, 3> &operands) {
    auto name = ST_BUILT_IN_MODEL_BINARY_OP;
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);
    std::vector<mlir::Type> inputTypes;
    for (auto operand : operands) inputTypes.push_back(operand.getType());
    auto mlirFnType = mlir::FunctionType::get(context, inputTypes, llvmI8PtrTy);

    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto newFunc = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);

    return SymbolRefAttr::get(name, context);
  }

  FlatSymbolRefAttr getOrInsertBuiltInBlockParamFunctionX(vector<mlir::Value> &operands){
    auto name = ST_BUILT_IN_MODEL_BLOCK_PARAM + to_string(operands.size());
    auto *context = builder.getContext();
    if (theModule.lookupSymbol<FuncOp>(name))
      return SymbolRefAttr::get(name, context);
    std::vector<mlir::Type> inputTypes;
    for(auto op:operands) inputTypes.push_back(op.getType());
    auto mlirFnType = mlir::FunctionType::get(context, inputTypes, llvmI8PtrTy);

    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(theModule.getBody());
    auto newFunc = builder.create<FuncOp>(theModule.getLoc(), name, mlirFnType);
    return SymbolRefAttr::get(name, context);
  }

 public:
  MLIRGenImpl(mlir::MLIRContext &context) : builder(&context) {
    // We create an empty MLIR module and codegen functions one at a time and
    // add them to the module.
    theModule = mlir::ModuleOp::create(builder.getUnknownLoc());
    builder.getContext()->loadDialect<LLVM::LLVMDialect>();
    llvmVoidTy = LLVM::LLVMVoidType::get(builder.getContext());
    llvmI8PtrTy = LLVM::LLVMPointerType::get(IntegerType::get(builder.getContext(), 8));
    llvmI32Ty = IntegerType::get(builder.getContext(), 32);
    llvmI64Ty = IntegerType::get(builder.getContext(), 64);
  }

  mlir::ModuleOp mlirGen(stx::ModuleAST &moduleAST) {
    // all classes
    for (auto classAST : moduleAST.getClasses()) {
      classesInfoMap[classAST->getName()] = classAST;
      for (auto value : classAST->inst_vars) {
        inst_varsClassMap[value].push_back(classAST->getName());
      }
      for (auto value : classAST->class_inst_vars) {
        class_varsClassMap[value].push_back(classAST->getName());
      }

      auto loc = builder.getFileLineColLoc(
          builder.getIdentifier(classAST->fileName), classAST->line, 0);
    }

    // main entry point
    if (moduleAST.entry_point) {
      auto func = mlirGen(*moduleAST.entry_point);
      if (func) {
        theModule.push_back(func);
        functionMap.insert({func.getName(), func});
      }
    }
    for (auto funcAST : moduleAST.getFunctions()) {
      auto func = mlirGen(*funcAST);
      if (func) {
        theModule.push_back(func);
        functionMap.insert({func.getName(), func});
      }
    }
    llvm::outs() << "\n-----MLIR-GEN-DONE------\n\n";

    return theModule;
  }

  mlir::LogicalResult declareX2(std::string varName, mlir::Value value,
                                StringRef funcName) {
    if (symbolTableStack.size() == 0) {
      llvm::outs() << "-----empty symbolTableStack---\n";
      return mlir::failure();
    }
    if (symbolTableStack.back()->count(varName)) {
      //llvm::outs() << "-----symbolTable------key exists already: " << varName
      //             << " func: " << funcName << " value: " << value << "\n";
      // TODO: erase it
      return mlir::failure();
    }

    (*symbolTableStack.back())[varName] = std::make_pair(funcName, value);

    //llvm::outs() << "-----symbolTable------key: " << varName
    //             << " func: " << funcName << " value: " << value << "\n";

    return mlir::success();
  }
  
  map<BinaryFunctionCallAST *, mlir::Value> arg0_cascadeMap;
  mlir::Value mlirGen(CascadeFunctionCallAST &call) {
    arg0_cascadeMap.clear();
    // multiple calls
    mlir::Value arg = nullptr;
    mlir::Value arg0_cascade = nullptr;
    switch (call.subCall->getKind()) {
      case ExprAST::Expr_FuncCall_Key: {
        auto kcall = cast<KeywordFunctionCallAST>(*call.subCall);
        arg = mlirGen(kcall);
        arg0_cascade = arg0_cascadeMap[kcall.getBinarySend()];
      } break;
      case ExprAST::Expr_FuncCall_Bin: {
        auto bcall = cast<BinaryFunctionCallAST>(*call.subCall);
        arg = mlirGen(bcall);
        arg0_cascade = arg0_cascadeMap[&bcall];
      } break;
      default:
        break;
    }
    for (auto kwmsg : call.getKeywordMessagesStack()) {
      SmallVector<mlir::Value, 4> operands;
      // get the first argument arg0_cascade
      if (arg0_cascade)
        operands.push_back(arg0_cascade);
      else
        operands.push_back(arg);
      std::string callee = kwmsg->getName();
      if(!kwmsg->getKeywordMessages().empty()) {
        for (auto msg : kwmsg->getKeywordMessages()) {
          auto arg2 = mlirGen(*msg->getParams());
          operands.push_back(arg2);
        }
      }
      callee = ST_BUILT_IN_NAME + callee;
      auto mlocation = loc(kwmsg->loc());
      auto calleeRef =
          getOrInsertUserDefinedFunctionX(mlocation, callee, operands);
      auto callop = builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy, calleeRef,
                                          operands);
      arg = callop.getResult(0);
    }

    for (auto umsg : call.getUnaryMessagesStack()) {
      SmallVector<mlir::Value, 4> operands;
      operands.push_back(arg);
      std::string callee = umsg;
      callee = ST_BUILT_IN_NAME + callee;
      auto mlocation = loc(call.loc());
      auto calleeRef =
          getOrInsertUserDefinedFunctionX(mlocation, callee, operands);
      auto callop = builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy, calleeRef,
                                          operands);
      arg = callop.getResult(0);
    }

    return arg;
  }
  mlir::Value mlirGen(KeywordFunctionCallAST &call) {
    auto mlocation = loc(call.loc());
    auto callee = call.getName();
    SmallVector<mlir::Value, 4> operands;
    auto arg = mlirGen(*call.getBinarySend());

    operands.push_back(arg);
    
    for (auto msg : call.getKeywordMessages()) {
      auto arg2 = mlirGen(*msg->getParams());
      operands.push_back(arg2);
    }
    auto calleeRef = getOrInsertUserDefinedFunctionX(
        mlocation, ST_BUILT_IN_NAME + callee, operands);
    auto callOp = builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy,
                                               calleeRef, operands);
    return callOp.getResult(0);
  }
  mlir::Value mlirGen(BinaryFunctionCallAST &call) {
    auto mlocation = loc(call.loc());

    auto expr = call.umessage->expr;  // self
    auto arg = mlirGen(*expr);        // args can be complicated!
    if(expr->getKind() == ExprAST::Expr_Block) {
      vector<mlir::Value> blockOperands;
      blockOperands.push_back(arg);
      auto b = cast<BlockExprAST>(expr);
      for(auto oa:b->params) {
          auto o = oa.getName();
          if(symbolTableStack.back()->count(o)) {
          //if(theModule.lookupSymbol<LLVM::GlobalOp>("local_"+o)) {
            //cout << "found local var0 to outerscope: " << o << endl;
            auto def = (*symbolTableStack.back())[o].second;
            blockOperands.push_back(def);
          }
          else if(inst_varsClassMap.find(o) != inst_varsClassMap.end()) {
           // llvm::outs() << "found inst var: " << o << "\n";
            for (auto className : inst_varsClassMap.at(o)) {
            //llvm::outs() << "possible className: " << className << "\n";
            }
            auto instVarRef = getOrInsertBuiltInInstVarFunctionX();
            Value v = getOrCreateGlobalStringX(mlocation, o, o);
            auto callOp =
              builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy, instVarRef, v);
            auto value = callOp.getResult(0);
            blockOperands.push_back(value);
          }
          else {
            blockOperands.push_back(getOrCreateGlobalStringX(mlocation, o, o));
          }
      }
      auto blockRef = getOrInsertBuiltInBlockParamFunctionX(blockOperands);
      auto callop = builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy, blockRef, blockOperands);
      arg = callop.getResult(0);
    }
    arg0_cascadeMap[&call] = arg;  // hack
    
    for (auto callee : call.umessage->unaryTails) {

      SmallVector<mlir::Value, 4> operands;
      operands.push_back(arg);
      callee = ST_BUILT_IN_NAME + callee;
      auto calleeRef =
          getOrInsertUserDefinedFunctionX(mlocation, callee, operands);
      auto callop = builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy, calleeRef,
                                          operands);
      arg = callop.getResult(0);
    }
    // TODO: handle binaryTails
    auto bmsg = call.bmessages;
    while (bmsg) {
      UnaryMessageAST *umsg = bmsg->operand;
      if (umsg) {
        {
          auto v = mlirGen(*umsg);
          SmallVector<mlir::Value, 3> operands;
          std::string name = GLOBAL_OP_NAME + bmsg->opr;
          Value v0 = getOrCreateGlobalStringX(mlocation, name, bmsg->opr);
          operands.push_back(v0);
          operands.push_back(arg);
          operands.push_back(v);
          // call st.model.     // ST_BUILT_IN_MODEL_BINARY_OP
          auto binOpRef = getOrInsertBuiltInBinaryOpFunctionX(operands);
          auto callop = builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy, binOpRef,
                                              operands);
          arg = callop.getResult(0);
        }
      }
      bmsg = bmsg->next;
    }

    return arg;
  }
  mlir::Value mlirGen(UnaryMessageAST &call) {
    auto mlocation = loc(call.loc());

    auto expr = call.expr;      // self
    
    auto arg = mlirGen(*expr);  // args can be complicated!
    for (auto callee : call.unaryTails) {
      // call callee on expr

      SmallVector<mlir::Value, 4> operands;
      operands.push_back(arg);
      callee = ST_BUILT_IN_NAME + callee;
      auto calleeRef =
          getOrInsertUserDefinedFunctionX(mlocation, callee, operands);
      auto callop = builder.create<LLVM::CallOp>(mlocation, llvmI8PtrTy, calleeRef,
                                          operands);
      arg = callop.getResult(0);
    }
    return arg;
  }

  /// This is a reference to a variable in an expression. The variable is
  /// expected to have been declared and so should have a value in the symbol
  /// table, otherwise emit an error and return nullptr.
  mlir::Value mlirGen(VariableExprAST &expr) {
    std::string varName = expr.getName();

    if (symbolTableStack.size() == 0) {
      llvm::outs() << "-----error: empty symbolTable------\n";
    }
    if (symbolTableStack.back()->count(expr.getName())) {
      auto result = symbolTableStack.back()->at(expr.getName());
      auto funcName = result.first;
      auto value = result.second;

      // TODO: if this is in anonymous func, create op for var

      // llvm::outs() << "-----found in symbolTable------func: " << funcName
      //              << " value: " << value << "\n";

      // make sure the function name match
      if (matchCurrentScopeFunction(funcName))
        return value;
      else {
        // llvm::outs() << "-----did not match function scope------\n";
        // call st.model.parentScope
        auto newScopeRef = getOrInsertBuiltInParentScopeX();
        std::string name = LOCAL_VAR_NAME + varName;
        auto varLoc = loc(expr.loc());
        Value v2 = getOrCreateGlobalStringX(varLoc, name, varName);

        Value v1 = getOrCreateGlobalStringX(
            varLoc, GLOBAL_VAR_NAME + funcName.str(), funcName.str());

        auto callOp = builder.create<LLVM::CallOp>(
            varLoc, llvmI8PtrTy, newScopeRef, ArrayRef<Value>({v1, v2}));
        auto value = callOp.getResult(0);
        declareX2(name, value, funcName);
        return value;
      }
    } else {
      auto varLoc = loc(expr.loc());

      auto input = varName;
      // check classesInfoMap if input is an instance var or class var
      if (inst_varsClassMap.find(input) != inst_varsClassMap.end()) {
        auto instVarRef = getOrInsertBuiltInInstVarFunctionX();
        Value v = getOrCreateGlobalStringX(varLoc, varName, varName);
        auto callOp =
            builder.create<LLVM::CallOp>(varLoc, llvmI8PtrTy, instVarRef, v);
        auto value = callOp.getResult(0);
        return value;

      } else if (class_varsClassMap.find(input) != class_varsClassMap.end()) {
        auto classVarRef = getOrInsertBuiltInClassVarFunctionX();
        Value v = getOrCreateGlobalStringX(varLoc, varName, varName);
        auto callOp =
            builder.create<LLVM::CallOp>(varLoc, llvmI8PtrTy, classVarRef, v);
        auto value = callOp.getResult(0);
        return value;
      } else if (classesInfoMap.find(input) != classesInfoMap.end()) {
        llvm::outs() << "found className: " << input << "\n";
      } else {
        auto functionName = findSymbolInParentScopes(input);
        if (!functionName.empty())
          llvm::outs() << "found symbol in parent function: " << functionName
                       << "\n";
        else
          llvm::outs() << "unknown class or library? " << input << "\n";
      }
    }
    return getOrCreateGlobalStringX(loc(expr.loc()), varName, varName);
  }
  // JEFF: invariant - should never return NULL
  mlir::Value mlirGen(BlockExprAST &block) {
    string suffix = functionStack.size()>0?functionStack.back().str():"";
    if(suffix != "")
      replace(suffix.begin(), suffix.end(), '$', '*');
    std::string funcName = ANON_FUNC_NAME + std::to_string(++ANON_FUNC_INDEX) + "*" + suffix;
    auto locInfo = loc(block.loc());
    {
      std::string anonName = funcName;
      pushNewFunction(funcName);
      auto retBlock = getOrCreateGlobalStringX(locInfo, funcName, funcName);

      if (!block.func) {
        // should not get there in normal
        popLastFunction(funcName);
        return retBlock;
      }
      PrototypeAST protoAST(block.loc(), funcName, block.getBlockParams());

      // Create an MLIR function for the given prototype.
      auto blockFunc = mlirGen(protoAST);
      if (blockFunc) {
        theModule.push_back(blockFunc);
        functionMap.insert({blockFunc.getName(), blockFunc});

      } else {
        llvm::outs() << "-----failed to create BlockExprAST ------ name: "
                     << block.getName() << "\n";
        return retBlock;
      }

      auto &entryBlock = *blockFunc.addEntryBlock();
      auto protoArgs = protoAST.getArgs();

      unsigned int argIt = 1;
      for (auto p:protoArgs) {
        declareX2(p.getName(), entryBlock.getArgument(argIt++), funcName);
      }

      // Set the insertion point in the builder to the beginning of the function
      // body, it will be used throughout the codegen to create operations in
      // this function.
      OpBuilder::InsertionGuard insertGuard(builder);
      builder.setInsertionPointToStart(&entryBlock);
      auto retLoc = locInfo;
      //mlir::Value retValue = entryBlock.getArguments().back();
      mlir::Value retValue = entryBlock.getArguments().front();
      // temp variables
      for (auto vardecl : block.func->getLocals()) {
        declareLocalVariable(vardecl, funcName);
      }
      auto astlist = block.func->getBody();
      if (astlist->size() > 0) {
        mlirGen(*astlist);
      }

      builder.create<mlir::LLVM::ReturnOp>(retLoc, llvm::makeArrayRef(retValue));

      popLastFunction(funcName);
      return retBlock;
    }

    // TODO: insert call to this anonymous func, and return its ret
    // parent->this
    auto blockRef = SymbolRefAttr::get(funcName, builder.getContext());
    auto parentFuncName = getCurrentFunctionName();
    auto parent =
        getOrCreateGlobalStringX(locInfo, parentFuncName.str(), parentFuncName);
    auto callOp =
        builder.create<LLVM::CallOp>(locInfo, llvmI8PtrTy, blockRef, parent);
    auto value = callOp.getResult(0);

    return value;
  }

  mlir::Value mlirGen(AssignExprAST &assign) {
    // visit vhs => v1
    mlir::Value v1 = mlirGen(*assign.getRHS());
    // visit lhs => v2
    mlir::Value v2 = mlirGen(*assign.getLHS());
    // create assign expression v2 = v1
    auto assignCallRef =
        getOrInsertAssignFunctionX(ST_BUILT_IN_MODEL_OPAQUE_ASSIGN);
    builder.create<LLVM::CallOp>(loc(assign.loc()), llvmVoidTy, assignCallRef,
                                 ArrayRef<Value>({v2, v1}));
    return v2;
  }
  // Codegen a list of expression, return failure if one of them hit an error.
  // return the last value
  void mlirGen(ExprASTList &exprList) {
    auto size = exprList.size();
    for (int k = 0; k < size; k++) {
      auto expr = exprList[k];
      if (auto *ret = dyn_cast<ReturnExprAST>(expr)) {
        //todo: finish lowering of this kind of ExprAST
        //llvm::outs() << "-----mlirGen ExprASTList expr------ReturnExprAST"
        //             << "\n";
        continue;
      }
      if (auto *print = dyn_cast<PrintExprAST>(expr)) {
        //todo: finish lowering of this kind of ExprAST
        //llvm::outs() << "-----mlirGen ExprASTList expr------PrintExprAST"
        //             << "\n";
        continue;
      }
      if (auto *assign = dyn_cast<AssignExprAST>(expr)) {
        //llvm::outs() << "-----mlirGen ExprASTList expr------AssignExprAST"
        //             << "\n";
        // to be processed later
        mlirGen(*assign);
        continue;
      }
        mlirGen(*expr);
    }
  }

  mlir::Value mlirGen(LiteralExprAST &lit) {
    auto input = lit.getName();
    // constant string
    std::string name = LOCAL_VAR_NAME + input;
    Value str = getOrCreateGlobalStringX(loc(lit.loc()), name, input);
    return str;  // important
  }
  mlir::Value mlirGen(StringExprAST &str) {
    auto input = str.getName();
    // todo: cast to i*8 to follow type of predefined funcs
    auto inputS = input;
    std::string name = LOCAL_VAR_NAME + inputS;
    Value v = getOrCreateGlobalStringX(loc(str.loc()), name, inputS);
    return v;
  }

  mlir::Value mlirGen(SymbolExprAST &sym) {
    auto input = sym.getSymbol();
    // todo: cast to i*8 to follow type of predefined funcs
    auto inputS = input;
    std::string name = LOCAL_VAR_NAME + inputS;
    Value str = getOrCreateGlobalStringX(loc(sym.loc()), name, inputS);
    return str;
  }
  mlir::Value mlirGen(ReservedKeywordExprAST &reserved) {
    auto input = reserved.getName();
    // todo: cast to i*8 to follow type of predefined funcs
    // TODO: handle self

    auto inputS = input;
    std::string name = LOCAL_VAR_NAME + inputS;
    Value str = getOrCreateGlobalStringX(loc(reserved.loc()), name, inputS);
    return str;
  }

  mlir::Value mlirGen(NumberExprAST &num) {
    auto input = num.getValue();
    // todo: cast to i*8 to follow type of predefined funcs
    auto inputS = std::to_string(input);
    Value str = getOrCreateGlobalStringX(loc(num.loc()),
                                         LOCAL_VAR_NAME + inputS, inputS);
    return str;
  }

  /// Dispatch codegen for the right expression subclass using RTTI.
  mlir::Value mlirGen(ExprAST &expr) {
    switch (expr.getKind()) {
      case ExprAST::Expr_Var:
        return mlirGen(cast<VariableExprAST>(expr));
      case ExprAST::Expr_Num:
        return mlirGen(cast<NumberExprAST>(expr));
      case ExprAST::Expr_String:
        return mlirGen(cast<StringExprAST>(expr));
      case ExprAST::Expr_Symbol:
        return mlirGen(cast<SymbolExprAST>(expr));
      case ExprAST::Expr_Reserved:
        return mlirGen(cast<ReservedKeywordExprAST>(expr));
      case ExprAST::Expr_Literal:
        return mlirGen(cast<LiteralExprAST>(expr));
      case ExprAST::Expr_Block:
        return mlirGen(cast<BlockExprAST>(expr));
      case ExprAST::Expr_Assign:
        return mlirGen(cast<AssignExprAST>(expr));
      case ExprAST::Expr_FuncCall_Bin:
        return mlirGen(cast<BinaryFunctionCallAST>(expr));
      case ExprAST::Expr_FuncCall_Key: {
        return mlirGen(cast<KeywordFunctionCallAST>(expr));
      }
      case ExprAST::Expr_FuncCall_Cascade: {
        return mlirGen(cast<CascadeFunctionCallAST>(expr));
      }
      case ExprAST::Expr_DynamicDictionary: {
        auto dictExprAST = cast<DynamicDictionaryExprAST>(expr);
        mlir::Value v = nullptr;
        for (auto dictExpr : dictExprAST.getExpressions())
          v = mlirGen(*dictExpr);
        return v;
      }
      case ExprAST::Expr_DynamicArray: {
        auto arrExprAST = cast<DynamicArrayExprAST>(expr);
        mlir::Value v = nullptr;
        for (auto arrExpr : arrExprAST.getExpressions()) v = mlirGen(*arrExpr);
        return v;
      }

      default:
        emitError(loc(expr.loc()))
            << "MLIR codegen encountered an unhandled expr kind '"
            << Twine(expr.getKind()) << "'";
        return nullptr;
    }
  }

  void declareLocalVariable(VarDeclExprAST &vardecl, llvm::StringRef funcName) {
    auto varName = vardecl.getName();
    auto varLoc = loc(vardecl.loc());

    auto newTempRef = getOrInsertBuiltInNewFunctionX();
    std::string name = LOCAL_VAR_NAME + varName;
    Value v = getOrCreateGlobalStringX(varLoc, name, varName);
    auto callOp =
        builder.create<LLVM::CallOp>(varLoc, llvmI8PtrTy, newTempRef, v);
    auto value = callOp.getResult(0);
    declareX2(vardecl.getName(), value,
              funcName);  // ok this assume value is unique?
  }

  /// Create the prototype for an MLIR function with as many arguments as the
  /// provided Toy AST prototype.
  // return type: for main, void
  // for block and others, i8ptr

  mlir::FuncOp mlirGen(PrototypeAST &proto) {
    auto location = loc(proto.loc());

    // This is a generic function, the return type will be inferred later.
    llvm::SmallVector<mlir::Type, 4> argTypes;
    // by default, add *i8 to func's arg to represent "this"
    //"this" is either class object or class instance object
    if (proto.getName() != MAIN_FUNC_NAME) {
      argTypes.reserve(proto.getArgs().size() + 1);
      argTypes.push_back(llvmI8PtrTy);
    } 

    for (auto &arg : proto.getArgs()) {
      argTypes.push_back(llvmI8PtrTy);
    }

    auto returnType = llvmVoidTy;
    if (proto.getName() == MAIN_FUNC_NAME)
      returnType = llvmI64Ty;
    else
      returnType = llvmI8PtrTy;
    auto func_type = builder.getFunctionType(argTypes, returnType);
    return mlir::FuncOp::create(location, proto.getName(), func_type);
  }

  mlir::FuncOp mlirGenClassesMetadata(stx::ModuleAST &moduleAST) {
    auto location = builder.getFileLineColLoc(
        builder.getIdentifier(moduleAST.workspace), 0, 0);
    auto func_type = builder.getFunctionType(llvmI8PtrTy, llvmI64Ty);
    auto function =
        mlir::FuncOp::create(location, ST_BUILT_IN_CLASS_METADATA, func_type);

    auto &entryBlock = *function.addEntryBlock();
    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(&entryBlock);

    // all classes
    for (auto classAST : moduleAST.getClasses()) {
      auto mlocation = builder.getFileLineColLoc(
          builder.getIdentifier(classAST->fileName), classAST->line, 0);
      SmallVector<mlir::Value, 2> operands;
      // classAST->getName()
      // classAST->getSuperClassName()
      Value v1 = getOrCreateGlobalStringX(mlocation, classAST->getName(),
                                          classAST->getName());
      Value v2 =
          getOrCreateGlobalStringX(mlocation, classAST->getSuperClassName(),
                                   classAST->getSuperClassName());
      operands.push_back(v1);
      operands.push_back(v2);
      auto calleeRef = getOrInsertSuperClassFunctionX();
      builder.create<LLVM::CallOp>(mlocation, llvmVoidTy, calleeRef, operands);
    }

    mlir::Value value = builder.create<LLVM::ConstantOp>(
        location, llvmI64Ty, builder.getIntegerAttr(builder.getIndexType(), 0));
    builder.create<mlir::LLVM::ReturnOp>(location, llvm::makeArrayRef(value));

    return function;
  }
  /// Emit a new function and add it to the MLIR module.
  mlir::FuncOp mlirGen(FunctionAST &funcAST) {
    // Create a scope in the symbol table to hold variable declarations.

    // SymbolTableScopeT2 var_scope(symbolTable2);
    auto funcName = funcAST.getName();
    astStack.push_back(&funcAST);
    pushNewFunction(funcName);
    llvm::outs() << "NEW FunctionAST------name: " << funcName << "\n";

    // Create an MLIR function for the given prototype.
    auto function = mlirGen(*funcAST.getProto());
    // Let's start the body of the function now!
    // In MLIR the entry block of the function is special: it must have the
    // same argument list as the function itself.
    auto entryBlockPtr = function.addEntryBlock();
    curBlock = entryBlockPtr;
    auto &entryBlock = *entryBlockPtr;
    auto protoArgs = funcAST.getProto()->getArgs();

    unsigned int argIt = 1;
      for (auto p:protoArgs) {
        declareX2(p.getName(), entryBlock.getArgument(argIt++), funcName);
      }

    // Set the insertion point in the builder to the beginning of the function
    // body, it will be used throughout the codegen to create operations in
    // this function.
    OpBuilder::InsertionGuard insertGuard(builder);
    builder.setInsertionPointToStart(&entryBlock);

    // temp variables
    for (auto vardecl : funcAST.getLocals()) {
      declareLocalVariable(vardecl, funcName);
    }
    // symbolic execution?? let's do type inference in CR!
    // or let's do type inferece in MLIRGen?
    // if (false)
    { mlirGen(*funcAST.getBody()); }
    // if (false)
    {
      auto locProto = loc(funcAST.getProto()->loc());  // loc(return_val->loc()
      if (funcName != MAIN_FUNC_NAME) {
        //mlir::Value retValue = entryBlock.getArguments().back();
        mlir::Value retValue = entryBlock.getArguments().front();
        if (funcAST.getProto()->getRet()) {
          // TODO: return the correct value
          // return_val can be complicated
          retValue = mlirGen(*funcAST.return_val);
        }
        builder.create<mlir::LLVM::ReturnOp>(locProto, llvm::makeArrayRef(retValue));
      } else {
        mlir::Value value = builder.create<LLVM::ConstantOp>(
            locProto, llvmI64Ty,
            builder.getIntegerAttr(builder.getIndexType(), 0));
        builder.create<mlir::LLVM::ReturnOp>(locProto, llvm::makeArrayRef(value));
      }
    }
    popLastFunction(funcName);
    astStack.pop_back();
    return function;
  }
};  // namespace stx
mlir::OwningModuleRef mlirGenFull(mlir::MLIRContext &context,
                                  stx::ModuleAST &moduleAST) {
  return MLIRGenImpl(context).mlirGen(moduleAST);
};

}  // namespace stx
