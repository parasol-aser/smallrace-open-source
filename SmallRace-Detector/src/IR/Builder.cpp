/* Copyright 2021 Coderrect Inc. All Rights Reserved.
Licensed under the GNU Affero General Public License, version 3 or later (“AGPL”), as published by the Free Software
Foundation. You may not use this file except in compliance with the License. You may obtain a copy of the License at
https://www.gnu.org/licenses/agpl-3.0.en.html
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an “AS IS” BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "IR/Builder.h"

#include <llvm/Analysis/PostDominators.h>
#include <llvm/Analysis/ScopedNoAliasAA.h>
#include <llvm/Support/CommandLine.h>

#include "IR/IRImpls.h"
#include "LanguageModel/LLVMInstrinsics.h"
#include "LanguageModel/OpenMP.h"
#include "LanguageModel/pthread.h"
#include "LanguageModel/SmalltalkModel.h"


#include "PointerAnalysis/Graph/ConstraintGraph/ConstraintGraph.h"

using namespace race;
using namespace pta;
using ConsGraphTy = ConstraintGraph<ctx>;

  using CallGraphTy = CallGraph<ctx>;
  using CGNodeTy = CGNodeBase<ctx>;
  using CallNodeTy = CallGraphNode<ctx>;

  using CT = CtxTrait<ctx>;
  using PT = PTSTrait<PtsTy>;
  using MMT = MemModelTrait<MemModel>;
  using Canonicalizer = typename MMT::Canonicalizer;
  using ObjT = typename MMT::ObjectTy;
// std::unique_ptr<ConsGraphTy> consGraph;  // owner of the constraint callgraph

extern llvm::cl::opt<bool> DEBUG_PTA;

namespace {

bool hasThreadLocalOperand(const llvm::Instruction *inst) {
  auto ptr = getPointerOperand(inst);
  assert(ptr);
  if (auto global = llvm::dyn_cast<llvm::GlobalVariable>(ptr)) {
    return global->isThreadLocal();
  }
  return false;
}

// Assuming ompForkCall points to a OpenMP fork call, the next inst should be a duplicate omp fork call
// this returns that omp fork or null if the next inst is not a omp fork call
std::shared_ptr<OpenMPFork> getTwinOmpFork(const llvm::CallBase *ompForkCall) {
  auto next = ompForkCall->getNextNode();
  if (!next) return nullptr;

  auto twinOmpForkInst = llvm::dyn_cast<llvm::Instruction>(next);
  if (!twinOmpForkInst) return nullptr;

  auto twinCallInst = llvm::dyn_cast<llvm::CallBase>(twinOmpForkInst);
  if (!twinCallInst) return nullptr;
  if (!OpenMPModel::isFork(twinCallInst)) return nullptr;

  return std::make_shared<OpenMPFork>(twinCallInst);
}

// TODO: need different system for storing and organizing these "recognizers"
bool isPrintf(const llvm::StringRef &funcName) { return funcName.equals("printf"); }
}  // namespace
std::map<llvm::Value*, llvm::Value*> FtoATable;
FunctionSummary race::generateFunctionSummary(const llvm::Function *func, const pta::CallGraphNodeTy *node, const pta::PTA &pta) {
  assert(func != nullptr);
  
  return generateFunctionSummary(*func, node, pta);
}  
const llvm::Instruction* getVariableDef(std::string var_name, const pta::CallGraphNodeTy *node, const llvm::Module *module){
// std::string curScope = "";
                // while(curScope != "main"){
                  for(auto it = node->pred_begin(); it!=node->pred_end();it++){
                    auto func = (*it)->getTargetFun()->getFunction();
                    // traverse all instructions
                    // //llvm::outs() << "=== Current function:\n";
                    // func->dump();
                    // //llvm::outs() << "=== end Current function\n";

                    for (auto const &basicblock : func->getBasicBlockList()) {
                      for (auto it = basicblock.begin(), end = basicblock.end(); it != end; ++it) {
                        auto inst = llvm::cast<llvm::Instruction>(it);
                        if (llvm::isa<CallInst>(inst)){ 
                          auto calli = llvm::cast<llvm::CallInst>(inst); // calli->getCalledFunction()->getName()
                          auto callFuncName = calli->getCalledFunction()->getName();
                          if (SmalltalkModel::isNewTemp(callFuncName)) {
                            auto temp_var_name = inst->getOperand(0)->stripPointerCasts()->getName().str();
                            if (temp_var_name.find("local_", 0) == 0){
                              temp_var_name = temp_var_name.substr(6);
                            }
                            if (temp_var_name == var_name){
                              return inst;
                            }

                          }
                        }
                      }
                    }

                    // still not found, go to outer scope
                    // auto parent = (*it)->pred_begin();  // parent.I->second->target->fun->function->dump()

                    //  it.I->second->target->fun->function->dump() == parent
                    ////llvm::outs() << "Current Scope:" << func->getName().str()  << "\n";
                    if (func->getName().str() == "main"){
                      return nullptr;
                    }else{
                      auto res = getVariableDef(var_name, (*it), module);
                      if(res) return res;

                    }
                  } 
                // }
                return nullptr;


}

FunctionSummary race::generateFunctionSummary(const llvm::Function &func, const pta::CallGraphNodeTy *node, const pta::PTA &pta) {
  FunctionSummary instructions;
  for (auto const &basicblock : func.getBasicBlockList()) {
    if (DEBUG_PTA) {
      //llvm::outs() << "bb: " << basicblock.getName() << "\n";
    }
    for (auto it = basicblock.begin(), end = basicblock.end(); it != end; ++it) {
      auto inst = llvm::cast<llvm::Instruction>(it);
      if (DEBUG_PTA) {
        inst->print(llvm::outs(), false);
        //llvm::outs() << "\n";
      }
      // TODO: try switch on inst->getOpCode instead
      if (auto loadInst = llvm::dyn_cast<llvm::LoadInst>(inst)) {
        if (DEBUG_PTA) {
          loadInst->print(llvm::outs(), false);
        }
        if (loadInst->isAtomic() || loadInst->isVolatile() || hasThreadLocalOperand(loadInst)) {
          continue;
        }
        instructions.push_back(std::make_shared<race::Load>(loadInst));
      } else if (auto storeInst = llvm::dyn_cast<llvm::StoreInst>(inst)) {
        if (DEBUG_PTA) {
          storeInst->print(llvm::outs(), false);
        }
        if (storeInst->isAtomic() || storeInst->isVolatile() || hasThreadLocalOperand(storeInst)) {
          continue;
        }
        instructions.push_back(std::make_shared<race::Store>(storeInst));
      } else if (auto callInst = llvm::dyn_cast<llvm::CallBase>(inst)) {
        if (DEBUG_PTA) {
          callInst->print(llvm::outs(), false);
        }
        if (callInst->isIndirectCall()) {
          // let trace deal with indirect calls
          instructions.push_back(std::make_shared<race::CallIR>(callInst));
          continue;
        }

        auto calledFunc = callInst->getCalledFunction();
        if (calledFunc == nullptr || !calledFunc->hasName()) {
          // TODO: Log warning
          // llvm::errs() << "could not find called func: " << *callInst << "\n";
          continue;
        }

        // TODO: System for users to register new function recognizers here
        auto funcName = calledFunc->getName();
        ////llvm::outs() << "\n -- Function Name = " << funcName << "\n";
        // if (funcName.find("st.") == 0){
        //   instructions.push_back(std::make_shared<SmalltalkFork>(callInst));
        // }else
        if (LLVMModel::isNoEffect(funcName)) {
          /* Do nothing */
        } else if (PthreadModel::isPthreadCreate(funcName)) {
          instructions.push_back(std::make_shared<PthreadCreate>(callInst));
        } else if (PthreadModel::isPthreadJoin(funcName)) {
          instructions.push_back(std::make_shared<PthreadJoin>(callInst));
        } else if (PthreadModel::isPthreadMutexLock(funcName)) {
          instructions.push_back(std::make_shared<PthreadMutexLock>(callInst));
        } else if (PthreadModel::isPthreadMutexUnlock(funcName)) {
          instructions.push_back(std::make_shared<PthreadMutexUnlock>(callInst));
        } else if (PthreadModel::isPthreadSpinLock(funcName)) {
          instructions.push_back(std::make_shared<PthreadSpinLock>(callInst));
        } else if (PthreadModel::isPthreadSpinUnlock(funcName)) {
          instructions.push_back(std::make_shared<PthreadSpinLock>(callInst));
        } else if (OpenMPModel::isForStaticInit(funcName)) {
          instructions.push_back(std::make_shared<OmpForInit>(callInst));
        } else if (OpenMPModel::isForStaticFini(funcName)) {
          instructions.push_back(std::make_shared<OmpForFini>(callInst));
        } else if (OpenMPModel::isForDispatchInit(funcName)) {
          instructions.push_back(std::make_shared<OmpDispatchInit>(callInst));
        } else if (OpenMPModel::isForDispatchNext(funcName)) {
          instructions.push_back(std::make_shared<OmpDispatchNext>(callInst));
        } else if (OpenMPModel::isForDispatchFini(funcName)) {
          instructions.push_back(std::make_shared<OmpDispatchFini>(callInst));
        } else if (OpenMPModel::isSingleStart(funcName)) {
          instructions.push_back(std::make_shared<OpenMPSingleStart>(callInst));
        } else if (OpenMPModel::isSingleEnd(funcName)) {
          instructions.push_back(std::make_shared<OpenMPSingleEnd>(callInst));
        } else if (OpenMPModel::isMasterStart(funcName)) {
          instructions.push_back(std::make_shared<OpenMPMasterStart>(callInst));
        } else if (OpenMPModel::isMasterEnd(funcName)) {
          instructions.push_back(std::make_shared<OpenMPMasterEnd>(callInst));
        } else if (OpenMPModel::isBarrier(funcName)) {
          instructions.push_back(std::make_shared<OpenMPBarrier>(callInst));
        } else if (OpenMPModel::isReduceStart(funcName)) {
          instructions.push_back(std::make_shared<OpenMPReduce>(callInst));
        } else if (OpenMPModel::isReduceNowaitStart(funcName)) {
          instructions.push_back(std::make_shared<OpenMPReduce>(callInst));
        } else if (OpenMPModel::isCriticalStart(funcName)) {
          instructions.push_back(std::make_shared<OpenMPCriticalStart>(callInst));
        } else if (OpenMPModel::isCriticalEnd(funcName)) {
          instructions.push_back(std::make_shared<OpenMPCriticalEnd>(callInst));
        } else if (OpenMPModel::isSetLock(funcName)) {
          instructions.push_back(std::make_shared<OpenMPSetLock>(callInst));
        } else if (OpenMPModel::isUnsetLock(funcName)) {
          instructions.push_back(std::make_shared<OpenMPUnsetLock>(callInst));
        } else if (OpenMPModel::isSetNestLock(funcName)) {
          instructions.push_back(std::make_shared<OpenMPSetLock>(callInst));
        } else if (OpenMPModel::isUnsetNestLock(funcName)) {
          instructions.push_back(std::make_shared<OpenMPUnsetLock>(callInst));
        } else if (OpenMPModel::isGetThreadNum(funcName)) {
          instructions.push_back(std::make_shared<OpenMPGetThreadNum>(callInst));
        } else if (OpenMPModel::isOrderedStart(funcName)) {
          instructions.push_back(std::make_shared<OpenMPOrderedStart>(callInst));
        } else if (OpenMPModel::isOrderedEnd(funcName)) {
          instructions.push_back(std::make_shared<OpenMPOrderedEnd>(callInst));
        } else if (OpenMPModel::isFork(funcName)) {
          // duplicate omp preprocessing should duplicate all omp fork calls
          auto ompFork = std::make_shared<OpenMPFork>(callInst);
          auto twinOmpFork = getTwinOmpFork(callInst);
          if (!twinOmpFork) {
            // without duplicated fork we cannot detect any races in omp region so just skip it
            llvm::errs() << "Encountered non-duplicated omp fork instruction: " << *callInst << "\n";
            llvm::errs() << "Next Inst was: " << *callInst->getNextNode() << "\n";
            llvm::errs() << "Skipping entire OpenMP region\n";
            continue;
          }
          // We matched the next inst as twin omp fork
          ++it;

          // push the two forks and joins such tha the two threads created for the parallel region are in parallel
          instructions.push_back(ompFork);
          instructions.push_back(twinOmpFork);

          // omp fork has implicit join, so immediately join both threads
          instructions.push_back(std::make_shared<OpenMPJoin>(ompFork));
          instructions.push_back(std::make_shared<OpenMPJoin>(twinOmpFork));
        }else if (SmalltalkModel::isFork(funcName)){  //Smalltalk
          auto stFork = std::make_shared<SmalltalkFork>(callInst);
          ////llvm::outs() << "Captured Smalltalk Thread:" << *stFork << " --- END of Smalltalk Thread --- \n";
          // auto twinOmpFork = getTwinOmpFork(callInst);
          // if (!twinOmpFork) {
          //   // without duplicated fork we cannot detect any races in omp region so just skip it
          //   llvm::errs() << "Encountered non-duplicated omp fork instruction: " << *callInst << "\n";
          //   llvm::errs() << "Next Inst was: " << *callInst->getNextNode() << "\n";
          //   llvm::errs() << "Skipping entire OpenMP region\n";
          //   continue;
          // }
          // // We matched the next inst as twin omp fork
          // ++it;

          // push the two forks and joins such tha the two threads created for the parallel region are in parallel
          instructions.push_back(stFork);
          // instructions.push_back(twinOmpFork);

          // omp fork has implicit join, so immediately join both threads

          // Try not to add join for a correct happens before graph
          // instructions.push_back(std::make_shared<SmalltalkJoin>(stFork));
          
          // instructions.push_back(std::make_shared<OpenMPJoin>(twinOmpFork));
        }
        
        else if (SmalltalkModel::isSt(funcName)){
          // process everything but fork here.

          // Smalltalk models first:
          if(SmalltalkModel::isStModel(funcName)){
            // st.model.newTemp or newObject 
            if(SmalltalkModel::isNewObject(funcName)){// || SmalltalkModel::isNewTemp(funcName)){  
              // find out that there might not a good idea, since it needs a LLVM Load Instruction here.
              auto callInstruction = std::make_shared<race::Load>(callInst);
              // LoadInst::Create()
              // llvm::LoadInst()
              instructions.push_back(callInstruction);

            }

            // st.model.opaqueAssign
            if(SmalltalkModel::isOpaqueAssign(funcName)){
              auto lhs = callInst->arg_begin();
              auto rhs = callInst->arg_begin();

              

              std::advance(rhs, 1);
              if(llvm::isa<llvm::CallInst>(lhs)){
                auto lhsVal = llvm::cast<llvm::CallInst>(lhs);
                auto callInstruction = std::make_shared<race::Store>(lhsVal);
                instructions.push_back(callInstruction);
                
              }
              else if(llvm::isa<llvm::Argument>(lhs)){
                // auto parent = llvm::cast<llvm::Argument>(lhs)->getParent()->getName();
                // //llvm::outs() << "lhs: " << **lhs << " rhs: " << **rhs << "\n";
                // if(!parent.startswith("st.next")){
                //   bool ans = false;
                //   auto key = lhs->get();
                //   while(FtoATable.count(key)){
                //     key = FtoATable[key];
                //     if(llvm::dyn_cast<llvm::Instruction>(key)){
                //       ans = true;
                //       break;
                //     }
                //   }
                //   //llvm::outs() << "final key: " << *key << "\n";
                //   if(ans){
                //     auto gep = llvm::cast<llvm::Instruction>(key);
                //     auto callInstruction = std::make_shared<race::Load>(gep);
                //     instructions.push_back(callInstruction);
                //   }
                //   else{
                //     auto gep = llvm::cast<llvm::Instruction>(callInst);
                //     auto callInstruction = std::make_shared<race::Load>(gep);
                //     instructions.push_back(callInstruction);
                //   }
                // }
                
              }
              else {
                // //llvm::outs() << "variable is not defined in the current scope\n";
                // //llvm::outs() << llvm::isa<GlobalVariable>(lhs) << "\n";
                // auto var_name = (*lhs)->stripPointerCasts()->getName().str();

                // auto newVarDef = getVariableDef(var_name, node, pta.getLLVMModule());
                // if (!newVarDef){
                //   //llvm::outs() << "=== Still unable to find var definition in outer scope. The LLVM IR is invalid.\n";
                //   assert(false); 
                // }
                // auto lhsVal = llvm::cast<llvm::CallInst>(newVarDef);
                // auto callInstruction = std::make_shared<race::Store>(lhsVal);
                auto callInstruction = std::make_shared<race::Store>(callInst);
                instructions.push_back(callInstruction);
              }
              // else if(llvm::isa<llvm::GlobalVariable>(lhs)){ // to do: check the difference -- Done.
              // DEBUG
              // call void @st.model.opaqueAssign(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @test, i64 0, i64 0), i8* %1), !dbg !9
              // auto test1 = llvm::isa<llvm::GlobalVariable>(lhs);
              // lhs.Val: i8* getelementptr inbounds ([4 x i8], [4 x i8]* @test, i64 0, i64 0)
              // auto test2 = llvm::isa<llvm::GlobalVariable>((*lhs)->stripPointerCasts());
              // (*lhs)->stripPointerCasts(): @test = internal constant [4 x i8] c"test"
              

              // else if(llvm::isa<llvm::GlobalVariable>((*lhs)->stripPointerCasts())){
              //   // auto anonNameVar = llvm::cast<llvm::GlobalVariable>(arg0);
              //   // auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer())->getAsString();
              //   auto callInstruction = std::make_shared<race::Store>(callInst);
              //   instructions.push_back(callInstruction);
              // }
              

              if(llvm::isa<llvm::CallInst>(rhs)){
                auto rhsVal = llvm::cast<llvm::CallInst>(rhs);
                auto callInstruction = std::make_shared<race::Load>(rhsVal);
                instructions.push_back(callInstruction);
              }
            }


            // st.model.binaryop
            if(SmalltalkModel::isBinaryOp(funcName)){
              auto op = callInst->arg_begin();
              auto op1 = callInst->arg_begin();
              auto op2 = callInst->arg_begin();

              std::advance(op1, 1);
              std::advance(op2, 2);


              if(llvm::isa<llvm::CallInst>(op1)){
                auto lhsVal = llvm::cast<llvm::CallInst>(op1);
                auto callInstruction = std::make_shared<race::Load>(lhsVal);
                instructions.push_back(callInstruction);
              }
              if(llvm::isa<llvm::CallInst>(op2)){
                auto rhsVal = llvm::cast<llvm::CallInst>(op2);
                auto callInstruction = std::make_shared<race::Load>(rhsVal);
                instructions.push_back(callInstruction);
              }
              // Do we need to add a Store here? -- No, the model is in @st.model.opaqueAssign


            }

            if(SmalltalkModel::isblockParam(funcName) && !funcName.startswith("st.model.blockParam1")){
              auto blkName = llvm::cast<llvm::GlobalVariable>(callInst->getArgOperand(0)->stripPointerCasts());
              auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(blkName->getInitializer());
              auto blk = pta.getLLVMModule()->getFunction(anonNameArr->getAsString());
              if(!blk) continue;
              auto calleeArg = blk->arg_begin();
              auto callerArg = callInst->arg_begin();
              std::advance(callerArg, 1);
              std::advance(calleeArg, 1);
              for (auto const end = blk->arg_end(); calleeArg != end; ++calleeArg, ++callerArg) {
                FtoATable[calleeArg] = callerArg->get();
              }
            }
            continue;

          }else if(SmalltalkModel::isLibWrite(funcName)) {
                if(SmalltalkModel::isnextPut(funcName)) {
                  auto inst = llvm::cast<llvm::Instruction>(callInst);
                  auto callInstruction = std::make_shared<race::Store>(inst);
                  instructions.push_back(callInstruction);
                  continue;
                }
                auto write = callInst->arg_begin();
                ////llvm::outs() << "write: " << **write << "\n";
                if(auto cnt = SmalltalkModel::getLibWritePosi(funcName))
                  std::advance(write, cnt);
                ////llvm::outs() << "write: " << **write << "\n";

                if (llvm::isa<llvm::Instruction>(write)){
                  auto inst = llvm::cast<llvm::Instruction>(write);
                  auto callInstruction = std::make_shared<race::Store>(inst);
                  instructions.push_back(callInstruction);
                }else{
                  // auto newInst = read->stripPointerCasts();
                  if(llvm::isa<llvm::Argument>(write)){
                    bool ans = false;
                    auto key = write->get();
                    while(FtoATable.count(key)){
                      key = FtoATable[key];
                      if(llvm::dyn_cast<llvm::Instruction>(key)){
                        ans = true;
                        break;
                      }
                    }
                    ////llvm::outs() << "final key: " << *key << "\n";
                    if(ans){
                      auto gep = llvm::cast<llvm::Instruction>(key);
                      auto callInstruction = std::make_shared<race::Store>(gep);
                      instructions.push_back(callInstruction);
                    }
                    else{
                      auto gep = llvm::cast<llvm::Instruction>(callInst);
                      auto callInstruction = std::make_shared<race::Store>(gep);
                      instructions.push_back(callInstruction);
                    }
                  }
                  else {
                    auto gep = llvm::cast<llvm::Instruction>(callInst);
                    auto callInstruction = std::make_shared<race::Store>(gep);
                    instructions.push_back(callInstruction); 
                  }
                }

                // auto callInstruction = std::make_shared<race::Store>(llvm::cast<llvm::Instruction>(write));
                // instructions.push_back(callInstruction);

                
                // process potential code block in instruction
                for(int i=0; i<callInst->getNumArgOperands(); i++) {
                  auto arg = callInst->getArgOperand(i);
                  auto argV = arg->stripPointerCasts();
                  if(llvm::isa<llvm::GlobalVariable>(argV)) {
                    auto arg0 = llvm::cast<llvm::GlobalVariable>(argV);
                    if(arg0->getName().startswith("st.anonfun")) {
                      // this will be handled by ThreadTrace to process annofun.
                      instructions.push_back(std::make_shared<CallIR>(callInst));
                      break;
                    }

                  }
                }

                continue;
          }else if(SmalltalkModel::isLibRead(funcName)) {

                auto read = callInst->arg_begin();

                ////llvm::outs() << "read: " << **read << "\n";
                if(auto cnt = SmalltalkModel::getLibReadPosi(funcName)){
                  std::advance(read, cnt);
                }
                ////llvm::outs() << "read: " << **read << "\n";
                
                
                if (llvm::isa<llvm::Instruction>(read)){
                  auto inst = llvm::cast<llvm::Instruction>(read);
                  auto callInstruction = std::make_shared<race::Load>(inst);
                  instructions.push_back(callInstruction);
                }else{

                  if(llvm::isa<llvm::Argument>(read)){
                    
                    // ConsGraphTy *consgraph = pta.getConsGraph();
                    // size_t total_cnt = consgraph->getNodeNum();
                    // for(int i = 0;i<total_cnt;i++){
                    //     auto funPtrNode = consGraph->getCGNode(id)->getSuperNode();

                    // }

                    // consgraph->


                    // function argument, need to get points to set
                    // std::vector<const pta::ObjTy *> pts;
                    
                    // pta.getPointsTo(node->getContext(), *read, pts);
                    // for (auto obj : pts) {
                    //   //llvm::outs() << "obj->getValue() = " << *obj->getValue() << " ID=" << obj->getObjectID() <<  "\n";
                    //   auto node = consgraph->getObjectNode(obj);
                    //   if (auto objNode = llvm::cast<pta::CGNodeKind::ObjNode>(node)) {
                    //     auto object = objNode->getObject();
                    //     //llvm::outs() << "object value: " << *(object->getValue()) << "\n";
                    //     if(llvm::isa<CallBase>(object->getValue())) {
                    //       auto objV = llvm::cast<CallBase>(object->getValue());
                    //       auto calleeName = objV->getCalledFunction()->getName();
                    //       //llvm::outs() << "smalltalk callee name: " << calleeName << "\n";
                    //     }
                    //   }
                    //   // auto funPtrNode = consGraph->getCGNode(obj->getObjectID())->getSuperNode();
                    //   // auto targetNameSuffix = obj->getValue()->getName().str();  obj->getValue().get
                    //   if(llvm::isa<CallBase>( obj->getValue())){
                    //     auto target =  llvm::cast<llvm::CallBase>( obj->getValue());
                    //     int a =  1;
                    //   }
                    // }
                    bool ans = false;
                    auto key = read->get();
                    while(FtoATable.count(key)){
                      key = FtoATable[key];
                      if(llvm::dyn_cast<llvm::Instruction>(key)){
                        ans = true;
                        break;
                      }
                    }
                    ////llvm::outs() << "final key: " << *key << "\n";
                    if(ans){
                      auto gep = llvm::cast<llvm::Instruction>(key);
                      auto callInstruction = std::make_shared<race::Load>(gep);
                      instructions.push_back(callInstruction);
                    }
                    else{
                      auto gep = llvm::cast<llvm::Instruction>(callInst);
                      auto callInstruction = std::make_shared<race::Load>(gep);
                      instructions.push_back(callInstruction);
                    }
                  }
                  else{
                    // auto newInst = read->stripPointerCasts();
                    auto gep = llvm::cast<llvm::Instruction>(callInst);
                    auto callInstruction = std::make_shared<race::Load>(gep);
                    instructions.push_back(callInstruction);

                  }
                }
                

                // process potential code block in instruction
                for(int i=0; i<callInst->getNumArgOperands(); i++) {
                  auto arg = callInst->getArgOperand(i);
                  auto argV = arg->stripPointerCasts();
                  if(llvm::isa<llvm::GlobalVariable>(argV)) {
                    auto arg0 = llvm::cast<llvm::GlobalVariable>(argV);
                    if(arg0->getName().startswith("st.anonfun")) {
                      // this will be handled by ThreadTrace to process annofun.
                      instructions.push_back(std::make_shared<CallIR>(callInst));
                      break;
                    }

                  }
                }


                continue;
          }

          if (SmalltalkModel::isCritical(funcName)){
            instructions.push_back(std::make_shared<SmalltalkCriticalStart>(callInst));
            // to do: check if the CriticalEnd is needed
            instructions.push_back(std::make_shared<SmalltalkCriticalEnd>(callInst));
            continue;
          }

          if(SmalltalkModel::isValue(funcName)){
            auto value = callInst->getArgOperand(0);
            std::vector<const pta::ObjTy *> pts;
            pta.getPointsTo(node->getContext(), value, pts);
            for (auto obj : pts) {
              ////llvm::outs() << "obj->getValue() = " << *obj->getValue() << " ID=" << obj->getObjectID() <<  "\n";
              if(llvm::isa<llvm::CallBase>(*obj->getValue())) {
                auto blkName = llvm::cast<llvm::CallBase>(*obj->getValue()).getArgOperand(0)->stripPointerCasts();
                auto anonNameVar = llvm::cast<llvm::GlobalVariable>(blkName);
                auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
                auto blkFunc = pta.getLLVMModule()->getFunction(anonNameArr->getAsString());
                ////llvm::outs() << *blkFunc << "\n";
              }
            }
            instructions.push_back(std::make_shared<CallIR>(callInst));
            continue;
          }

          // todo: need to model repeat?

          // handle the rest of the functions (not built in)
          //assert((!OpenMPModel::isOpenMP(funcName) || OpenMPModel::isNoEffect(funcName)) && "Unhandled OpenMP Call!");

          instructions.push_back(std::make_shared<CallIR>(callInst));

        }

        

         else if (isPrintf(funcName)) {
          // TODO: model as read?
        } else {
          // Used to make sure we are not implicitly ignoring any OpenMP features
          // We should instead make sure we take the correct action for any OpenMP call
          assert((!OpenMPModel::isOpenMP(funcName) || OpenMPModel::isNoEffect(funcName)) && "Unhandled OpenMP Call!");

          instructions.push_back(std::make_shared<CallIR>(callInst));
        }
      }
    }
  }

  return instructions;
}