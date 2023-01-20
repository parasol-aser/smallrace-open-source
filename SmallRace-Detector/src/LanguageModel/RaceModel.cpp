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
#include "LanguageModel/RaceModel.h"

#include "IR/IRImpls.h"
#include "LanguageModel/OpenMP.h"
#include "LanguageModel/pthread.h"
#include "LanguageModel/SmalltalkModel.h"
using namespace pta;

RaceModel::RaceModel(llvm::Module *M, llvm::StringRef entry) : Super(M, entry) {
  originCtx::setOriginRules([&](const originCtx *context, const llvm::Instruction *I) -> bool {
    return this->isInvokingAnOrigin(context, I);
  });
}

InterceptResult RaceModel::interceptFunction(const ctx * /* callerCtx */, const ctx * /* calleeCtx */,
                                             const llvm::Function *F, const llvm::Instruction *callsite) {
  auto funcName = F->getName();
  //llvm::outs() << funcName << "\n";
  // Skip intrinsic in PTA
  if (F->isIntrinsic()) {
    return {nullptr, InterceptResult::Option::IGNORE_FUN};
  }

  if (PthreadModel::isPthreadCreate(funcName)) {
    race::PthreadCreate create(llvm::cast<CallBase>(callsite));
    auto callback = create.getThreadEntry()->stripPointerCasts();
    return {callback, InterceptResult::Option::EXPAND_BODY};
  }

  if (OpenMPModel::isFork(funcName)) {
    race::OpenMPFork fork(llvm::cast<CallBase>(callsite));
    return {fork.getThreadEntry(), InterceptResult::Option::EXPAND_BODY};
  }

//Smalltalk
  if(SmalltalkModel::isFork(funcName)){
    race::SmalltalkFork fork(llvm::cast<CallBase>(callsite));
    auto CB = llvm::cast<CallBase>(callsite);
    if(llvm::isa<llvm::Argument>(fork.getThreadEntry())){
      return {fork.getThreadEntry(), InterceptResult::Option::EXPAND_BODY};
    }
    auto anonNameVar = llvm::cast<llvm::GlobalVariable>(fork.getThreadEntry());
    auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
    auto anonFunc = this->getLLVMModule()->getFunction(anonNameArr->getAsString());
    blockTable[anonNameArr->getAsString().str()] = callsite->getFunction()->getName().str();
    if(anonFunc)
      return {anonFunc, InterceptResult::Option::EXPAND_BODY};
    else
      return {CB->getArgOperand(0), InterceptResult::Option::EXPAND_BODY};
  }
  else if(SmalltalkModel::isCritical(funcName)){
    if(llvm::isa<llvm::CallInst>(callsite)){
      auto calli = llvm::cast<llvm::CallInst>(callsite);
      
      if(!llvm::isa<llvm::CallBase>(calli->getArgOperand(1)->stripPointerCasts()))
        return {F, InterceptResult::Option::EXPAND_BODY};
      auto blk = llvm::cast<llvm::CallBase>(calli->getArgOperand(1)->stripPointerCasts());
      auto anonNameVar = llvm::cast<llvm::GlobalVariable>(blk->getArgOperand(0)->stripPointerCasts());
      auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
      auto anonFunc = this->getLLVMModule()->getFunction(anonNameArr->getAsString());
      blockTable[anonNameArr->getAsString().str()] = callsite->getFunction()->getName().str();
      return {anonFunc, InterceptResult::Option::EXPAND_BODY};
    }
  }
  else if(SmalltalkModel::isnext(funcName) || SmalltalkModel::isnextPut(funcName) || SmalltalkModel::isValue(funcName))
    return {F, InterceptResult::Option::EXPAND_BODY};
  else if(SmalltalkModel::isLibRead(funcName) || SmalltalkModel::isLibWrite(funcName))
    return {F, InterceptResult::Option::EXPAND_BODY};
  else if(funcName.startswith("st.class"))
    return {F, InterceptResult::Option::EXPAND_BODY};
  else if(!SmalltalkModel::isOpaqueAssign(funcName) && SmalltalkModel::isSmalltalk(funcName)){

    auto CB = llvm::cast<CallBase>(callsite);
    if (funcName.find('$') != funcName.npos) {
      //Indirect call that is already been resolved
     return {F, InterceptResult::Option::EXPAND_BODY};
    }
    
    if(SmalltalkModel::isRepeat(funcName)) {
      auto block = CB->getArgOperand(0)->stripPointerCasts();
      if(llvm::isa<llvm::CallBase>(block)) {
        auto b = llvm::cast<llvm::CallBase>(block);
        if(SmalltalkModel::isblockParam(b->getCalledFunction()->getName())) {
          auto anonFuncName = b->getArgOperand(0)->stripPointerCasts();
          if(llvm::isa<llvm::GlobalVariable>(anonFuncName)) {
            auto anonNameVar = llvm::cast<llvm::GlobalVariable>(anonFuncName);
            auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
            auto func = this->getLLVMModule()->getFunction(anonNameArr->getAsString());
            return {func, InterceptResult::Option::EXPAND_BODY};
          }
        }
      }
    }
    
    if(llvm::isa<GlobalVariable>(CB->getArgOperand(0)->stripPointerCasts())) {
      auto var = llvm::cast<GlobalVariable>(CB->getArgOperand(0)->stripPointerCasts());
      if(var->getName().startswith("local_self")){
        auto namePrefix = CB->getCalledFunction()->getName().str();
        auto nameS = CB->getCaller()->getName().str();
        if (nameS.find("$")!=std::string::npos){  // todo: recursively find pointers
          auto nameSuffix = nameS.substr(nameS.find("$")); // nameS might not contains '$', as in @st.anonfun.15 in t2.ll
          if(this->getLLVMModule()->getFunction(namePrefix + nameSuffix))
            return {this->getLLVMModule()->getFunction(namePrefix + nameSuffix), InterceptResult::Option::EXPAND_BODY};
        }
        else if(blockTable.count(nameS)) {
          auto parentFunc = blockTable[nameS];// to do: only one level of parent scope considered
          if (parentFunc.find("$")!=std::string::npos){
          auto nameSuffix = parentFunc.substr(parentFunc.find("$")); // nameS might not contains '$', as in @st.anonfun.15 in t2.ll
          if(this->getLLVMModule()->getFunction(namePrefix + nameSuffix))
            return {this->getLLVMModule()->getFunction(namePrefix + nameSuffix), InterceptResult::Option::EXPAND_BODY};
          else
            return {F, InterceptResult::Option::EXPAND_BODY};
          }
        }
        else
          return {F, InterceptResult::Option::EXPAND_BODY};
      }
      else{
        auto varName = "local_" + var->getName().str();
        //to do: test with non local vars
        auto curFunc = callsite->getFunction()->getName().str();
        while(blockTable.count(curFunc)) {
          curFunc = blockTable[curFunc];
          std::pair<std::string, std::string> p(curFunc, varName);
          if(blockVarTable.count(p)) {
            return {blockVarTable[p], InterceptResult::Option::EXPAND_BODY};
          }
          
        }
      }
    }

  if(funcName.startswith("st.anonfun"))
    return {F, InterceptResult::Option::EXPAND_BODY};
  //If arg0 is able to resolve, return it
  auto arg0 = CB->getArgOperand(0);
  if(llvm::isa<llvm::CallBase>(arg0)) {
    auto cb = llvm::cast<llvm::CallBase>(arg0);
    auto cbName = cb->getCalledFunction()->getName();
    if(SmalltalkModel::isNewTemp(cbName)) {
      return {arg0, InterceptResult::Option::EXPAND_BODY};
    }
  }

  //Smalltalk function calls with blocks as arguments
    for(int i=0; i<CB->getNumArgOperands(); i++) {
      auto arg = CB->getArgOperand(i);
      auto argV = arg->stripPointerCasts();
      if(llvm::isa<llvm::CallBase>(argV)) {
        auto arg0 = llvm::cast<llvm::CallBase>(argV);
        if(SmalltalkModel::isblockParam(arg0->getCalledFunction()->getName())) {
          auto blkName = llvm::cast<llvm::GlobalVariable>(arg0->getArgOperand(0)->stripPointerCasts());
          auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(blkName->getInitializer());
          auto blk = this->getLLVMModule()->getFunction(anonNameArr->getAsString());
          //llvm::outs() << "block name: " << blk->getName() << "\n";
          if(blk)
            return {blk, InterceptResult::Option::EXPAND_BODY};
        }
      }
    }
    auto t = CB->getArgOperand(0);
    return {t, InterceptResult::Option::EXPAND_BODY};
  }

  // By default always try to expand the function body
  return {F, InterceptResult::Option::EXPAND_BODY};
}

bool RaceModel::interceptCallSite(const CtxFunction<ctx> *caller, const CtxFunction<ctx> *callee,
                                  const llvm::Function *originalTarget, const llvm::Instruction *callsite) {
  assert(originalTarget != nullptr);
  assert(CT::contextEvolve(caller->getContext(), callsite) == callee->getContext());

  auto const call = llvm::dyn_cast<llvm::CallBase>(callsite);
  //llvm::outs() << "traversing callsite: " << *call << ", caller: " << call->getCaller()->getName() << "\n";
  if (!call || !call->getCalledFunction() || !call->getCalledFunction()->hasName()) return false;

  auto const funcName = call->getCalledFunction()->getName();

  if (PthreadModel::isPthreadCreate(funcName)) {
    // pthread_create passes a single void* arg
    //  pthread_create(null, foo, null, arg)
    //  foo(void *arg)
    // link 4th arg of caller to 1st arg of callee
    PtrNode *formal = this->getPtrNode(callee->getContext(), &*callee->getFunction()->arg_begin());
    PtrNode *actual = this->getPtrNode(caller->getContext(), call->getArgOperand(3));
    this->consGraph->addConstraints(actual, formal, Constraints::copy);
    return true;
  }

  if (OpenMPModel::isFork(funcName)) {
    // omp fork spawns thread that executes outline:
    //     omp_fork_call(a, b, outlined, n, n+1, n+2, ...)
    //     outlined(x, y, m, m+1, m+2, ...)
    // non global shared args are passed as pointers n, n+1, n+2, ...
    // and received by outlined func as m, m+1, m+2, ...

    // Need to link 4th arg of caller (n) to 3rd arg of callee (m)
    // and 4th arg of caller (n+1) to 4th arg of callee (m+1)
    // and so on

    auto calleeArg = callee->getFunction()->arg_begin();
    auto callerArg = call->arg_begin();
    std::advance(calleeArg, 2);
    std::advance(callerArg, 3);
    for (auto const end = callee->getFunction()->arg_end(); calleeArg != end; ++calleeArg, ++callerArg) {
      // Only link args with pointer type
      if (calleeArg->getType()->isPointerTy()) {
        PtrNode *formal = this->getPtrNode(callee->getContext(), calleeArg);
        PtrNode *actual = this->getPtrNode(caller->getContext(), *callerArg);
        this->consGraph->addConstraints(actual, formal, Constraints::copy);
      }
    }

    return true;
  }

  if(SmalltalkModel::isNewTemp(funcName)) {
    auto varName = llvm::cast<llvm::GlobalVariable>(call->getArgOperand(0)->stripPointerCasts())->getName().str();
    std::pair<std::string, std::string> p(call->getCaller()->getName().str(), varName);
    blockVarTable[p] = call;
  }

  if(SmalltalkModel::isInstVar(funcName)) {
    PtrNode *def = this->getPtrNode(caller->getContext(), call->getArgOperand(0)->stripPointerCasts());
    PtrNode *ref = this->getOrCreatePtrNode(caller->getContext(), call);
    this->consGraph->addConstraints(def, ref, Constraints::copy);
    return true;
  }

  if(SmalltalkModel::isblockParam(funcName)){
    if(funcName.startswith("st.model.blockParam1"))
      return true;
    auto annofunc = call->getArgOperand(0)->stripPointerCasts();
    
    auto anonNameVar = llvm::cast<llvm::GlobalVariable>(annofunc);
    auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
    auto anonFunc = this->getLLVMModule()->getFunction(anonNameArr->getAsString());
    if(!anonFunc) return true;

    auto calleeArg = anonFunc->arg_begin();
    auto callerArg = call->arg_begin();
    std::advance(calleeArg, 1);
    std::advance(callerArg, 1);
    for (auto const end = anonFunc->arg_end(); calleeArg != end; ++calleeArg, ++callerArg) {
      // Only link args with pointer type
      if (calleeArg->getType()->isPointerTy()) {
        // llvm::outs() << "callee: " << *calleeArg << "\n";
        // llvm::outs() << "caller: " << **callerArg << "\n";
        PtrNode *formal = this->getOrCreatePtrNode(callee->getContext(), calleeArg);
        PtrNode *actual = this->getOrCreatePtrNode(caller->getContext(), *callerArg);
        this->consGraph->addConstraints(actual, formal, Constraints::copy);
      }
    }

    ObjNode *blkObj = this->allocHeapObj(caller->getContext(), call, nullptr);
    PtrNode *blkPtr = this->getOrCreatePtrNode(caller->getContext(), call);
    this->consGraph->addConstraints(blkObj, blkPtr, Constraints::addr_of);

    return true;
  }


  if(SmalltalkModel::isnextPut(funcName)){
    if(llvm::isa<llvm::GlobalVariable>(call->getArgOperand(1)->stripPointerCasts())){
      return true;
    }
    if(llvm::isa<llvm::CallBase>(call->getArgOperand(1)->stripPointerCasts())){
      auto arg1 = llvm::cast<llvm::CallBase>(call->getArgOperand(1)->stripPointerCasts());
      if(!SmalltalkModel::isblockParam(arg1->getCalledFunction()->getName()))
        return true;
    }
    PtrNode *dst = this->getOrCreatePtrNode(caller->getContext(), call->getArgOperand(0)->stripPointerCasts());
    PtrNode *src = this->getOrCreatePtrNode(caller->getContext(), call->getArgOperand(1)->stripPointerCasts());
    //llvm::outs() << "caller: " << call->getCaller()->getName() << "\n";
    PtrNode *actual = this->getOrCreatePtrNode(callee->getContext(), call->getCaller()->getArg(1));
    this->consGraph->addConstraints(src, dst, Constraints::store);
    this->consGraph->addConstraints(actual, src, Constraints::copy);
    return true;
  }

  if(SmalltalkModel::isFork(funcName)) {  // to do: consider fork at priority here.
     race::SmalltalkFork fork(llvm::cast<CallBase>(callsite));

     auto glob = fork.getThreadEntry();
     if(llvm::isa<llvm::Argument>(glob)) return true;
     auto anonNameVar = llvm::cast<llvm::GlobalVariable>(fork.getThreadEntry());
     auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
    blockTable[anonNameArr->getAsString().str()] = call->getCaller()->getName().str();
    return true;
  }

  //adding support for outerscope
    if(call->getCaller()->getName().startswith("st.anonfun")) {
      auto lhs = call->arg_begin();
      auto rhs = call->arg_begin();

      std::advance(rhs, 1);
      //llvm::outs() << "caller: " << call->getCaller()->getName() << " " << ((**lhs)) << "\n";
      llvm::Value* g = &(**lhs);
      //llvm::outs() << "*g: " << *g << "\n";
      if(llvm::isa<llvm::GlobalVariable>(g->stripPointerCasts())) {
        auto GB = llvm::dyn_cast<llvm::GlobalVariable>(g->stripPointerCasts());
        //auto G = llvm::cast<llvm::GlobalVariable>(CB->getArgOperand(0)->stripPointerCasts());
        auto GName = GB->getName();
      
        auto anon = call->getCaller()->getName().str();
        while(blockTable.count(anon)) {
          anon = blockTable[anon];
          std::pair<std::string, std::string> p(anon, "local_" + GName.str());
          if(blockVarTable.count(p)) {
            auto func = this->getLLVMModule()->getFunction(anon);
            auto v = blockVarTable[p];
            if(llvm::isa<llvm::CallBase>(blockVarTable[p])) {
              auto def = llvm::cast<llvm::CallBase>(blockVarTable[p]);
              if(def->getCalledFunction()->getName().startswith("st.model.newTemp")) {
                PtrNode *lNode = this->getOrCreatePtrNode(caller->getContext(), (llvm::Value*)blockVarTable[p]);
                PtrNode *rNode = this->getPtrNode(caller->getContext(), *rhs);

                this->consGraph->addConstraints(rNode, lNode, Constraints::copy);
                return true;
              }
            }
          }
        }
      }
      
    }

  if(SmalltalkModel::isOpaqueAssign(funcName)) {
    auto lhs = call->arg_begin();
    auto rhs = call->arg_begin();

    std::advance(rhs, 1);

    // llvm::outs() << "lhs: " << **lhs << "\n";
    // llvm::outs() << "caller: " << call->getCaller()->getName() << "\n";
    if(llvm::isa<llvm::CallBase>(*rhs)){
      auto rhb = llvm::cast<llvm::CallBase>(*rhs);
      if(SmalltalkModel::isnext(rhb->getCalledFunction()->getName())){
        PtrNode *src = this->getOrCreatePtrNode(caller->getContext(), rhb->getArgOperand(0)->stripPointerCasts());
        PtrNode *dst = this->getOrCreatePtrNode(caller->getContext(), call->getArgOperand(0)->stripPointerCasts());
        this->consGraph->addConstraints(src, dst, Constraints::load);
        return true;
      }
      if(SmalltalkModel::isblockParam(rhb->getCalledFunction()->getName())){
        if(llvm::isa<llvm::CallBase>(*lhs)){
          auto lhb = llvm::cast<llvm::CallBase>(*lhs);
          if(SmalltalkModel::isNewTemp(rhb->getCalledFunction()->getName())){
            PtrNode *src = this->getOrCreatePtrNode(caller->getContext(), rhb->getArgOperand(0)->stripPointerCasts());
            PtrNode *dst = this->getOrCreatePtrNode(caller->getContext(), call->getArgOperand(0)->stripPointerCasts());
            this->consGraph->addConstraints(src, dst, Constraints::load);
            return true;
          }
        }
      }
    }
    

    if(llvm::isa<llvm::CallBase>(*lhs) && llvm::isa<llvm::CallBase>(*rhs)) {
      auto lhb = llvm::cast<llvm::CallBase>(*lhs);
      auto rhb = llvm::cast<llvm::CallBase>(*rhs);
      if(SmalltalkModel::isInstVar(lhb->getCalledFunction()->getName()) && (SmalltalkModel::isNewObject(rhb->getCalledFunction()->getName()))) {
        PtrNode *lNode = this->getPtrNode(caller->getContext(), lhb->getArgOperand(0)->stripPointerCasts());
        PtrNode *rNode = this->getPtrNode(caller->getContext(), *rhs);
        this->consGraph->addConstraints(rNode, lNode, Constraints::copy);
        return true;
      }
    }
    

    PtrNode *lNode = this->getPtrNode(caller->getContext(), *lhs);
    PtrNode *rNode = this->getPtrNode(caller->getContext(), *rhs);

    this->consGraph->addConstraints(rNode, lNode, Constraints::copy);
    return true;
  }

  if(SmalltalkModel::isSmalltalk(funcName)) {
      if(llvm::isa<llvm::GlobalVariable>(call->getArgOperand(0)->stripPointerCasts())) {
        // calls other than forks which use blocks as arguments
        auto arg0 = llvm::cast<llvm::GlobalVariable>(call->getArgOperand(0)->stripPointerCasts());
        if(arg0->getName().startswith("st.anonfun")) {
          auto anonNameVar = llvm::cast<llvm::GlobalVariable>(arg0);
          auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
          auto func = this->getLLVMModule()->getFunction(anonNameArr->getAsString());
          PtrNode *gNode = this->getPtrNode(caller->getContext(), arg0);
          PtrNode *fNode = this->getPtrNode(caller->getContext(), func);

          this->consGraph->addConstraints(gNode, fNode, Constraints::copy);
          return true;
        }
      }
    }


  return false;
}

bool RaceModel::isCompatible(const llvm::Instruction * /* callsite */, const llvm::Function * /* target */) {
  llvm_unreachable("unrecognizable function");
}

void RaceModel::interceptHeapAllocSite(const CtxFunction<ctx> *caller, const CtxFunction<ctx> *callee,
                                       const llvm::Instruction *callsite) {
  if (heapModel.isHeapAllocFun(callee->getFunction())) {  // handled by default heap model
    Type *type = heapModel.inferHeapAllocType(callee->getFunction(), callsite);
    // add points-to constraints
    PtrNode *ptr = this->getPtrNode(caller->getContext(), callsite);
    ObjNode *obj = this->allocHeapObj(caller->getContext(), callsite, type);
    this->consGraph->addConstraints(obj, ptr, Constraints::addr_of);
  } else if (OpenMPModel::isTaskAlloc(callee->getName())) {  // handled by openmp-specific model
    // the type will be something like %struct.kmp_task_t_with_privates
    Type *type = heapModel.inferHeapAllocTypeForOpenMP(callee->getFunction(), callsite);

    if (type == nullptr) {
      // llvm::errs() << "cannot infer type for omp task alloc? callsite=" << callsite << "\n";
      return;
    }

    // we are going to model the points-to constraints like this:
    //  taskObj = &sharedObj -> { sharedObj } ∈ pts(taskobj)
    //  ptr = &taskObj       -> { taskObj } ∈ pts(ptr)
    // where sharedObj, taskObj and ptr are:
    // e.g. /OpenRace/tests/data/integration/dataracebench/DRB027-taskdependmissing-orig-yes.c
    //    int i=0; //sharedObj
    //    #pragma omp parallel
    //    #pragma omp single
    //    {
    //    #pragma omp task //taskObj (available in IR only: __kmpc_omp_task_alloc)
    //      i = 1; //ptr
    //    ...
    //    }
    ObjNode *sharedObj = MMT::template allocateAnonObj<PT>(
        this->getMemModel(), caller->getContext(), this->getLLVMModule()->getDataLayout(),
        type == nullptr ? nullptr : type->getPointerElementType(), nullptr,
        false);  // do not initialized its element
    ObjNode *taskObj = allocHeapObj(caller->getContext(), callsite, type);
    PtrNode *ptr = this->getPtrNode(caller->getContext(), callsite);

    this->consGraph->addConstraints(sharedObj, taskObj, Constraints::addr_of);
    this->consGraph->addConstraints(taskObj, ptr, Constraints::addr_of);

  } else {
    // if st.model.objectNew
    //PtrNode *ptr = this->getPtrNode(caller->getContext(), callsite);
    //ObjNode *obj = this->allocHeapObj(caller->getContext(), callsite, nullptr);
    //this->consGraph->addConstraints(obj, ptr, Constraints::addr_of);
    auto funcName = llvm::cast<CallBase>(callsite)->getCalledFunction()->getName();
    if (SmalltalkModel::isNewObject(funcName)) {
      PtrNode *ptr = this->getPtrNode(caller->getContext(), callsite);
      ObjNode *obj = this->allocHeapObj(caller->getContext(), callsite, nullptr);
      this->consGraph->addConstraints(obj, ptr, Constraints::addr_of);
    }


    // llvm::errs() << "implement for alloc: allocsite=" << callsite << "\n";
  }
}

bool RaceModel::isHeapAllocAPI(const llvm::Function *F, const llvm::Instruction * /* callsite */) {
  if (!F->hasName()) return false;
  auto const name = F->getName();
  return name.equals("malloc") || name.equals("calloc") || name.equals("_Zname") || name.equals("_Znwm") || name.startswith("st.model.newObject");  // name.equals("st.model.newTemp") ||
}

namespace {
// TODO: better way of handling these
const std::set<llvm::StringRef> origins{"pthread_create", "__kmpc_fork_call"};
}  // namespace

bool RaceModel::isInvokingAnOrigin(const originCtx * /* prevCtx */, const llvm::Instruction *I) {
  auto call = llvm::dyn_cast<CallBase>(I);
  if (!call || !call->getCalledFunction() || !call->getCalledFunction()->hasName()) return false;

  auto const name = call->getCalledFunction()->getName();
  return origins.find(name) != origins.end();
}
