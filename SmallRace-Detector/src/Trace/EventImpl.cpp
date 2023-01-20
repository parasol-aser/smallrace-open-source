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

#include "Trace/EventImpl.h"

#include "Trace/ProgramTrace.h"
#include "Trace/ThreadTrace.h"

using namespace race;

std::vector<const pta::ObjTy *> ReadEventImpl::getAccessedMemory() const {
  std::vector<const pta::ObjTy *> pts;
  info->thread.program.pta.getPointsTo(info->context, read->getAccessedValue(), pts);
  if(pts.size()==0){
    if(llvm::isa<llvm::CallBase>(read->getAccessedValue())){
      auto CB = llvm::cast<llvm::CallBase>(read->getAccessedValue());
      auto arg0 = CB->getArgOperand(0)->stripPointerCasts();
      std::vector<const pta::ObjTy *> pts2;
      info->thread.program.pta.getPointsTo(info->context, arg0, pts2);
      if(pts2.size()!=0) return pts2;
    }
  }
  return pts;
}

std::vector<const pta::ObjTy *> WriteEventImpl::getAccessedMemory() const {
  std::vector<const pta::ObjTy *> pts;
  //llvm::outs() << "write value: " << *(write->getAccessedValue()) << "\n";
  info->thread.program.pta.getPointsTo(info->context, write->getAccessedValue(), pts);
  if(pts.size()==0){
    if(llvm::isa<llvm::CallBase>(write->getAccessedValue())){
      auto CB = llvm::cast<llvm::CallBase>(write->getAccessedValue());
      auto arg0 = CB->getArgOperand(0)->stripPointerCasts();
      std::vector<const pta::ObjTy *> pts2;
      info->thread.program.pta.getPointsTo(info->context, arg0, pts2);
      if(pts2.size()!=0) return pts2;
    }
  }
  return pts;
}

std::vector<const pta::CallGraphNodeTy *> ForkEventImpl::getThreadEntry() const {
  auto entryVal = fork->getThreadEntry();
  auto mod = info->thread.program.pta.getLLVMModule();
  llvm::Function *entryNewFun = nullptr;
  std::vector<const pta::ObjTy *> pts;
  if(llvm::isa<llvm::Argument>(entryVal)){
    info->thread.program.pta.getPointsTo(info->context, entryVal, pts);
    for(auto &o: pts){
      auto tmp = o->getValue();
      // llvm::outs() << "tmp" << *tmp << "\n";
      if(llvm::isa<llvm::CallBase>(tmp)){
        auto cb = llvm::cast<llvm::CallBase>(tmp);
        if(cb->getCalledFunction()->getName().startswith("st.model.blockParam")){
          auto anonNameVar = llvm::cast<llvm::GlobalVariable>(cb->getArgOperand(0)->stripPointerCasts());
          auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
          entryNewFun = mod->getFunction(anonNameArr->getAsString());
          break;
        }
      }
    }
  }
  else{
    auto anonNameVar = llvm::cast<llvm::GlobalVariable>(entryVal);
    auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
    entryNewFun = mod->getFunction(anonNameArr->getAsString());
  
    if(entryNewFun == nullptr){
      auto local = llvm::cast<llvm::CallBase>(llvm::cast<llvm::CallBase>(fork->getInst())->getArgOperand(0)->stripPointerCasts());
      info->thread.program.pta.getPointsTo(info->context, local, pts);
      llvm::Value* blkValue = nullptr;
      for(auto &o: pts){
        auto tmp = o->getValue();
        //llvm::outs() << *tmp << "\n";
        if(llvm::isa<llvm::CallBase>(tmp)){
          if(llvm::cast<llvm::CallBase>(tmp)->getCalledFunction()->getName().startswith("st.model.blockParam")){
            blkValue = (llvm::Value*)tmp;
            break;
          }
        }
      }
      if(blkValue == nullptr)
        return {nullptr};
      auto anonNameVar = llvm::cast<llvm::GlobalVariable>(llvm::cast<CallBase>(blkValue)->getArgOperand(0)->stripPointerCasts());
      auto anonNameArr = llvm::cast<llvm::ConstantDataArray>(anonNameVar->getInitializer());
      entryNewFun = mod->getFunction(anonNameArr->getAsString());
  }
  }
  
if(!entryNewFun)
    {
      //llvm::outs() << "null entry" << "\n";
      return {nullptr};
    }
  if (auto entryFunc = llvm::dyn_cast<llvm::Function>(entryNewFun)) {
    auto const newContext = pta::CT::contextEvolve(info->context, fork->getInst());
    auto const entryNode = info->thread.program.pta.getDirectNodeOrNull(newContext, entryFunc);
    return {entryNode};
  }

  // the entry is indirect and we need to figure out where the real function is
  auto callsite = info->thread.program.pta.getInDirectCallSite(info->context, fork->getInst());
  auto const &nodes = callsite->getResolvedNode();
  return std::vector<const pta::CallGraphNodeTy *>(nodes.begin(), nodes.end());
}
