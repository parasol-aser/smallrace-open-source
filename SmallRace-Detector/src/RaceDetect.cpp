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



#include "RaceDetect.h"

#include <llvm/Analysis/ScopedNoAliasAA.h>

#include "Analysis/HappensBeforeGraph.h"
#include "Analysis/LockSet.h"
#include "Analysis/SharedMemory.h"
#include "Analysis/SimpleAlias.h"
#include "LanguageModel/RaceModel.h"
#include "Trace/ProgramTrace.h"
#include "LanguageModel/SmalltalkModel.h"

// #include <chrono>


#include <set>
#include <unordered_set>

using namespace race;

namespace timer{

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
}



static llvm::cl::opt<std::string> EntryFuncs("entries", cl::desc("Entrypoints of IR"),
                                           cl::value_desc("configuration file, in json format"));


bool filter_local_variable(const race::MemAccessEvent *write){
  auto instWrite = write->getIRInst()->getInst();

  if(llvm::isa<llvm::CallInst>(instWrite)){
    auto calli = llvm::cast<llvm::CallInst>(instWrite);
    auto callFuncName = calli->getCalledFunction()->getName();
    if (SmalltalkModel::isNewTemp(callFuncName)) {
      auto name1 = write->getIRInst()->getAccessedValue()->getName();
      if (name1.startswith("local_") ){
        return true;
      }
    }
  }
  return false;
}

Report race::detectRaces(llvm::Module *module, DetectRaceConfig config) {
  if(EntryFuncs != ""){

  }
  // preprocess2(*module, EntryFuncs);
  timer::Timer<std::chrono::milliseconds, std::chrono::steady_clock> myClock;  // Timer ticks upon construction.

  race::ProgramTrace program(module);
  
  myClock.tock();
  llvm::outs() << "+=+=+=+= ProgramTrace runs for " << myClock.duration().count() << " ms\n";
  llvm::outs() << "+=+=+=+= Size of Threads: " << program.getThreads().size() << "\n";

  std::set<const llvm::Value *> alllocks;

  auto allthreads = &program.getThreads();
  for (auto it = allthreads->begin(); it != allthreads->end(); ++it) {
      auto threads = it->get();
      for (auto const &event : threads->getEvents()) {
          
        switch (event->type) {
          case Event::Type::Lock: {
            auto lockEvent = llvm::cast<LockEvent>(event.get());
            auto LockValue = lockEvent->getIRInst()->getLockValue();
            std::vector<const pta::ObjTy *> pts;
            program.pta.getPointsTo(nullptr, LockValue, pts);
            for (auto obj : pts) {
              // llvm::outs() << "obj->getValue() = " << *obj->getValue() << " ID=" << obj->getObjectID() <<  "\n";
              LockValue = obj->getValue();
              // auto funPtrNode = consGraph->getCGNode(obj->getObjectID())->getSuperNode();
              // auto targetNameSuffix = obj->getValue()->getName().str();  obj->getValue().get
              // if(llvm::isa<CallBase>( obj->getValue())){
              //   auto callb = llvm::cast<CallBase>( obj->getValue());
              //   llvm::outs() << "Smalltalk callbase: " << callb->getArgOperand(0)->stripPointerCasts() << "\n";
              // }
            }
            alllocks.insert(LockValue);
          }
        }
      }
  }
  llvm::outs() << "+=+=+=+= Size of Locks: " << alllocks.size() << "\n";
  // llvm::outs() << "All locks: {";
  //         auto it = alllocks.begin();
  //         for (; it != alllocks.end(); it++) llvm::outs() << *it << " ";
  //         llvm::outs() << "}\n";


  // std::cout << "Resetting clock\n";


  myClock.tick();
    
  if (config.dumpPreprocessedIR.has_value()) {
    std::error_code err;
    llvm::raw_fd_ostream outfile(config.dumpPreprocessedIR.value(), err);
    if (err) {
      llvm::errs() << "Error dumping preprocessed IR!\n";
    } else {
      program.getModule().print(outfile, nullptr);
      outfile.close();
    }
  }
  race::Reporter reporter;
  // return reporter.getReport();
  race::SharedMemory sharedmem(program);
  race::HappensBeforeGraph happensbefore(program);
  race::LockSet lockset(program);
  race::SimpleAlias simpleAlias;
  // race::OpenMPAnalysis ompAnalysis(program);

  // race::Reporter reporter;
  // return reporter.getReport();
  llvm::PassBuilder PB;
  llvm::FunctionAnalysisManager FAM;
  PB.registerFunctionAnalyses(FAM);
  // contains default AA pipeline (type + scoped + global)
  // but i do not know how to register it properly now
  // FAM.registerPass([&] { return PB.buildDefaultAAPipeline(); });

  // Adds to report if race is detected between write and other
  auto checkRace = [&](const race::WriteEvent *write, const race::MemAccessEvent *other) {
    if (DEBUG_PTA) { 
      llvm::outs() << "Checking Race: " << write->getID() << " " << other->getID() << "\n";
    }

    if (filter_local_variable(write) || filter_local_variable(other)){
      return;
    }

    if (!happensbefore.areParallel(write, other) || lockset.sharesLock(write, other)) {
      return;
    }

    if (simpleAlias.mustNotAlias(write, other)) {
      return;
    }
    reporter.collect(write, other);
  };

  for (auto const sharedObj : sharedmem.getSharedObjects()) {
    auto threadedWrites = sharedmem.getThreadedWrites(sharedObj);
    auto threadedReads = sharedmem.getThreadedReads(sharedObj);

    for (auto it = threadedWrites.begin(), end = threadedWrites.end(); it != end; ++it) {
      auto const wtid = it->first;
      auto const writes = it->second;
      // check Read/Write race
      for (auto const &[rtid, reads] : threadedReads) {
        if (wtid == rtid) continue;
        for (auto write : writes) {
          for (auto read : reads) {
            checkRace(write, read);
          }
        }
      }

      // Check write/write
      for (auto wit = std::next(it, 1); wit != end; ++wit) {
        auto otherWrites = wit->second;
        for (auto write : writes) {
          for (auto otherWrite : otherWrites) {
            checkRace(write, otherWrite);
          }
        }
      }
    }
  }

  llvm::outs() << program << "\n";
  
  myClock.tock();
  llvm::outs() << "+=+=+=+= Race Detection runs for " << myClock.duration().count() << " ms\n";
  return reporter.getReport();
}
