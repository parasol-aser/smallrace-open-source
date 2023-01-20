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

#include "ProgramTrace.h"

#include "Trace/Event.h"
#include <vector>
#include <map>

using namespace race;
// using namespace std;


#include <chrono>

template <class TimeT = std::chrono::milliseconds,
          class ClockT = std::chrono::steady_clock>
class Timer
{
    using timep_t = decltype(ClockT::now());
    
    timep_t _start = ClockT::now();
    timep_t _end = {};

public:
    void tick() { 
        _end = timep_t{};
        _start = ClockT::now(); 
    }
    
    void tock() {
        _end = ClockT::now(); 
    }
    
    template <class TT = TimeT>
    TT duration() const { 
        // Use gsl_Expects if your project supports it.
        assert(_end != timep_t{} && "Timer must toc before reading the time"); 
        return std::chrono::duration_cast<TT>(_end - _start); 
    }
};


llvm::StringRef getNamefromGlobal(llvm::Module *module, llvm::StringRef entryName){
  //llvm::outs() << "--- getNamefromGlobal:" << entryName << "\n";
  if (entryName.startswith("local_")) return entryName; // handle local variable

  auto gv = module->getNamedGlobal(entryName);
    if (gv) {
      // ConstantDataArray 
      if (llvm::ConstantDataArray  *CDA = dyn_cast<llvm::ConstantDataArray >(gv->getOperand(0))) {
        //llvm::outs() << "Starts not function3:" << CDA->getAsString() << "\n";
        return (CDA->getAsString());
        // f = module->getFunction(CDA->getAsString());
      } else {
        //llvm::outs() << "Nope. ConstantDataArray"
                     //<< "\n";
        return entryName;
      }
    } else {
      //llvm::outs() << "Nope. getNamedGlobal" << "\n";
      return entryName;
    }
    return entryName;

}

llvm::StringRef handleGetelementptr(llvm::Module *module, const llvm::User *callee1){
  if (auto gv2 = dyn_cast_or_null<llvm::Constant>(callee1)) {
      if (auto anonFuncData = dyn_cast_or_null<llvm::ConstantExpr>(gv2)) {
        if (llvm::StringRef(anonFuncData->getOpcodeName()).equals("getelementptr")) {
          auto op = anonFuncData->getOperandList()->get();
          llvm::StringRef var_name = op->getName();
          return getNamefromGlobal(module, var_name);
        }
      }
  }
}


llvm::StringRef getCallee(llvm::Module *module, const llvm::Instruction *I, int argPosi = 0){
  //llvm::outs() << "--- getCallee:" << *I << "\n";
  pta::CallSite CS(I);
  llvm::StringRef callName = CS.getCalledFunction()->getName();
  //llvm::outs() << "--- CS.getArgOperand(argPosi): " << *CS.getArgOperand(argPosi) << "\n";
  auto arg_val = CS.getArgOperand(argPosi);
  if (auto user = dyn_cast<llvm::User>(arg_val)) {
    if (auto gv = dyn_cast_or_null<llvm::Constant>(user)) {
      //llvm::outs() << "global variable: " << *gv << "\n";
      if (auto anonFuncData = dyn_cast_or_null<llvm::ConstantExpr>(gv)) {
        //llvm::outs() << "anonFuncData:" << *anonFuncData << "\n";
        if (llvm::StringRef(anonFuncData->getOpcodeName()).equals("getelementptr")) {
          // //llvm::outs() << "OK.\n";
          auto op = anonFuncData->getOperandList()->get();
          return getNamefromGlobal(module, op->getName());
        }
        else {
          return getNamefromGlobal(module, anonFuncData->getOperandList()->get()->getName());
        }

      }
    } else if (auto gv = dyn_cast_or_null<llvm::Instruction>(user)) {
        if (gv->getOpcodeName() == llvm::StringRef("call")){
        if (isa<CallBase>(gv)){
          pta::CallSite CS(gv);
          llvm::StringRef callName = CS.getCalledFunction()->getName();
          if (callName == llvm::StringRef("st.model.binaryop")) {
            auto callee1 = dyn_cast<llvm::User>(CS.getArgOperand(0));
            auto callee2 = dyn_cast<llvm::User>(CS.getArgOperand(1));
            auto callee3 = dyn_cast<llvm::User>(CS.getArgOperand(2));

            if (auto c1 = dyn_cast_or_null<llvm::Instruction>(callee2)) {
              return getCallee(module,c1);

            }
            if (auto c1 = dyn_cast_or_null<llvm::Instruction>(callee3)) {
              return getCallee(module, c1);
            }


          }else if (callName == llvm::StringRef("st.model.opaqueAssign")) {
            auto callee1 = dyn_cast<llvm::User>(CS.getArgOperand(0));
            auto callee2 = dyn_cast<llvm::User>(CS.getArgOperand(1));
          
          }else if (callName == llvm::StringRef("st.model.newTemp") || callName.startswith(StringRef("st.model.newObject")) || callName == llvm::StringRef("st.model.instVar")) {
            auto callee1 = dyn_cast<llvm::User>(CS.getArgOperand(0));
            return handleGetelementptr(module, callee1);
          }
        }
      }

      
    }else if (auto gv = dyn_cast_or_null<llvm::Operator>(user)){

      //llvm::outs() << "dyn_cast_or_null<llvm::Operator>" << "\n";

    }
    
  } else if (auto arg = dyn_cast<llvm::Argument>(arg_val)) {
    // TODO. Pointer Analysis?
    

    // //llvm::outs() << "*** arg->getParent():" << arg->getContext() << "\n";
  } else if (auto user = dyn_cast<llvm::BasicBlock>(arg_val)) {
    //llvm::outs() << "getCallee dyn_cast<llvm::BasicBlock>"
                    //<< "\n";
  } else if (auto user = dyn_cast<llvm::InlineAsm>(arg_val)) {
    //llvm::outs() << "getCallee dyn_cast<llvm::InlineAsm>"
                    //<< "\n";

  }  else {
    //llvm::outs() << "getCallee Failed. Not dyn_cast<llvm::User>"
                    //<< "\n";
  }
  
      return "";
}

std::map<std::string,std::vector<std::string> > funcClass;

void processClassNames(llvm::Module *module){
  for (auto it=module->begin(); it!=module->end();it++){
    // //llvm::outs() << it->getName() << "\n";
    std::string name = it->getName().str();
    if (name.find("$")!=std::string::npos){
      // //llvm::outs() << "---" << name.substr(0,name.find("$")) << "\n";
      funcClass[name.substr(0,name.find("$"))].push_back(name);
    }
  }


  // dump map:
  //llvm::outs() << " ---- DUMP STARTS ---- \n";
  for(auto it = funcClass.cbegin(); it != funcClass.cend(); ++it)
  {
    
    //llvm::outs() << "*** Dumping: " << it->first << " \n";
    for(auto t: it->second){
      //llvm::outs() << t << "\n";
    }
  }
  //llvm::outs() << " ---- DUMP ENDS ---- \n";
}

bool init = true;
bool DEBUG = true;
ProgramTrace::ProgramTrace(llvm::Module *module, llvm::StringRef entryName) : module(module) {
  Timer clock;  // Timer ticks upon construction.
  // Run pointer analysis
  pta.analyze(module, entryName);

  clock.tock();
  llvm::outs() << "+=+=+=+= PTA runs for " << clock.duration().count() << " ms\n";

  clock.tick();

  TraceBuildState state;

  // build all threads starting from this main func
  auto const mainEntry = pta::GT::getEntryNode(pta.getCallGraph());
  auto mainThread = std::make_unique<ThreadTrace>(*this, mainEntry, state);
  // insert at front because main thread is always first
  threads.insert(threads.begin(), std::move(mainThread));
  clock.tock();
  llvm::outs() << "+=+=+=+= Build ThreadTrace runs for " << clock.duration().count() << " ms\n";

}

llvm::raw_ostream &race::operator<<(llvm::raw_ostream &os, const ProgramTrace &trace) {
  os << "===== Program Trace =====\n";

  // the order is a little reversed for parallel omp forks after changing the traversal order
  auto const &threads = trace.getThreads();
  for (auto const &thread : threads) {
    os << *thread;
  }

  os << "========================\n";
  return os;
}