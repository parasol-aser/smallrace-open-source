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

#pragma once

#include <vector>
#include <map>
#include <string>

#include "LanguageModel/RaceModel.h"
#include "ThreadTrace.h"
#include "Trace/Event.h"

namespace race {

// all included states are ONLY used when building ProgramTrace/ThreadTrace
struct TraceBuildState {
  // the counter of thread id: since we are constructing ThreadTrace while building events,
  // pState.threads.size() will be updated after finishing the construction, we need such a counter
  ThreadID currentTID = 0;
};

class ProgramTrace {
  
  std::vector<std::unique_ptr<ThreadTrace> > threads;

struct instruct{
    enum {
      READ,
      WRITE,
      INIT
    }instruction; // 0-read 1-write
    std::string scope;
  };

 std::map<std::string, std::vector<instruct> > racesdata;




  friend class ThreadTrace;

 public:
 llvm::Module *module;
  pta::PTA pta;
std::map<std::string, std::vector<instruct> > getRacesdata(){
  return racesdata;
}
  
void dumpRacesdata(){
  llvm::outs() << "----------- DUMP STARTS HERE -----------\n";
  for(auto it = racesdata.cbegin(); it != racesdata.cend(); ++it)
  {
    
      llvm::outs() << "DUMP_RACE_DATA --- for variable: " << it->first << " \n";
      for (auto inst : it->second){
        if (inst.instruction ==0){ llvm::outs() << "      --- READ";}
        if (inst.instruction ==1){ llvm::outs() << "      --- WRITE";}
        if (inst.instruction ==2){ llvm::outs() << "      --- INIT";}
        llvm::outs() << " at Scope: " << inst.scope << "\n";  
      }
      
  }
  llvm::outs() << "----------- DUMP ENDS HERE -----------\n";
}
  
  [[nodiscard]] inline const std::vector<std::unique_ptr<ThreadTrace>> &getThreads() const { return threads; }

  [[nodiscard]] const Event *getEvent(ThreadID tid, EventID eid) { return threads.at(tid)->getEvent(eid); }

  // Get the module after preprocessing has been run
  [[nodiscard]] const Module &getModule() const { return *module; }


  explicit ProgramTrace(llvm::Module *module, llvm::StringRef entryName = "main");
  void ProgramTrace2(llvm::Module *module, llvm::StringRef entryName = "main", llvm::StringRef curScope = "thread0");

  ~ProgramTrace() = default;
  ProgramTrace(const ProgramTrace &) = delete;
  ProgramTrace(ProgramTrace &&) = delete;  // Need to update threads because
                                           // they contain reference to parent
  ProgramTrace &operator=(const ProgramTrace &) = delete;
  ProgramTrace &operator=(ProgramTrace &&) = delete;
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const ProgramTrace &trace);

}  // namespace race
