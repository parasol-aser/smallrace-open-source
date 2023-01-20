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

#include "Reporter.h"

#include <experimental/filesystem>
#include <fstream>
namespace fs = std::experimental::filesystem;

#include "llvm/IR/DebugInfoMetadata.h"

using namespace race;

namespace {

std::optional<SourceLoc> getSourceLoc(const llvm::Instruction *inst) {
  auto const &loc = inst->getDebugLoc();
  if (auto diloc = loc.get()) {
    return SourceLoc(diloc);
  }

  return std::nullopt;
}

std::optional<SourceLoc> getSourceLoc(const Event *e) { return getSourceLoc(e->getInst()); }

}  // namespace

bool SourceLoc::operator<(const SourceLoc &other) const {
  if (filename != other.filename) return filename < other.filename;
  if (line != other.line) return line < other.line;
  return col < other.col;
}

void race::to_json(json &j, const SourceLoc &loc) {
  j = json{{"filename", loc.filename}, {"dir", loc.directory}, {"line", loc.line}, {"col", loc.col}};
}

llvm::raw_ostream &race::operator<<(llvm::raw_ostream &os, const SourceLoc &loc) {
  os << loc.directory << ":" << loc.filename << ":" << loc.line << ":" << loc.col;
  return os;
}

CallSignature::CallSignature(const llvm::CallBase *callBase) : call(std::nullopt), loc(getSourceLoc(callBase)) {
  if (callBase->getCalledFunction()->hasName()) {
    call = callBase->getCalledFunction()->getName();
  }
}

llvm::raw_ostream &race::operator<<(llvm::raw_ostream &os, const CallSignature &call) {
  os << (call.call ? call.call.value() : "UnamedFunc") << " (";
  if (call.loc) {
    os << call.loc;
  } else {
    os << "UnknownLoc";
  }
  os << ")";
  return os;
}

namespace {
std::vector<CallSignature> computeCallstack(const Event *targetEvent) {
  std::vector<CallSignature> callstack;

  auto const &thread = targetEvent->getThread();
  auto const &events = thread.getEvents();

  // Compute parent thread callstack and add it to the final callstack
  auto const spawnSite = targetEvent->getThread().spawnSite;
  if (spawnSite) {
    callstack = computeCallstack(spawnSite.value());
  }

  for (auto const &event : events) {
    // quit when past the target event
    // if the targetEvent is a call it will also be added to the callstack
    if (event->getID() > targetEvent->getID()) break;

    if (auto const callEvent = llvm::dyn_cast<EnterCallEvent>(event.get())) {
      auto call = llvm::cast<llvm::CallBase>(callEvent->getInst());
      callstack.emplace_back(call);
    } else if (event->type == Event::Type::CallEnd) {
      callstack.pop_back();
    }
  }

  // Check if targetEvent is a fork, if so add it to the call stack
  if (auto fork = llvm::dyn_cast<ForkEvent>(targetEvent)) {
    auto call = llvm::cast<llvm::CallBase>(fork->getInst());
    callstack.emplace_back(call);
  }

  return callstack;
}

std::vector<CallSignature> computeStackTrace(const Event *targetEvent) {
  std::vector<CallSignature> stacktrace;

  auto const &thread = targetEvent->getThread();
  auto const &events = thread.getEvents();

  auto const spawnSite = targetEvent->getThread().spawnSite;
  if (spawnSite) {
    stacktrace = computeStackTrace(spawnSite.value());
  }

  for (auto const &event : events) {
    // quit when past the target event
    // if the targetEvent is a call it will also be added to the stacktrace
    if (event->getID() > targetEvent->getID()) break;
    
    if (auto const callEvent = llvm::dyn_cast<EnterCallEvent>(event.get())) {
      auto call = llvm::cast<llvm::CallBase>(callEvent->getInst());
      stacktrace.emplace_back(call);
    } 
    
  }

  // Check if targetEvent is a fork, if so add it to the call stack
  if (auto fork = llvm::dyn_cast<ForkEvent>(targetEvent)) {
    auto call = llvm::cast<llvm::CallBase>(fork->getInst());
    stacktrace.emplace_back(call);
  }

  return stacktrace;
}

}  // namespace

RaceAccess::RaceAccess(const MemAccessEvent *event)
    : location(getSourceLoc(event)), type(event->type), inst(event->getInst()), callstack(computeCallstack(event)), stacktrace(computeStackTrace(event)) {
  updateMisleadingDebugLoc();
}

void race::to_json(json &j, const RaceAccess &access) {
  if (access.location.has_value()) {
    j = access.location.value();
    j["type"] = access.type;
  } else {
    llvm_unreachable("The report we serialize to JSON should only include races with valid locations");
  }
}

bool RaceAccess::operator==(const RaceAccess &other) const { return location == other.location && inst == other.inst; }
bool RaceAccess::operator!=(const RaceAccess &other) const { return !(*this == other); }
bool RaceAccess::operator<(const RaceAccess &other) const {
  if (location != other.location) return location < other.location;
  return inst < other.inst;
}

// update the line when a "line 0" location happens
void RaceAccess::updateMisleadingDebugLoc() {
  if (location->line != 0) return;

  auto loc = inst->getDebugLoc().get();
  if (auto *F = llvm::dyn_cast<llvm::DILexicalBlock>(loc->getScope())) {
    // here stores the original loc before debug info merged
    location = SourceLoc(loc->getFilename(), F->getLine(), F->getColumn());
    return;
  }

  llvm::errs() << "Cannot correctly udpate misleading debugloc for " << *inst << "\n";
}

llvm::raw_ostream &race::operator<<(llvm::raw_ostream &os, const std::optional<SourceLoc> &location) {
  if (location.has_value()) {
    os << location.value();
  } else {
    os << "UnknownLocation";
  }
  return os;
}

llvm::raw_ostream &race::operator<<(llvm::raw_ostream &os, const RaceAccess &acc) {
  os << acc.location << "\t";
  if (acc.type == Event::Type::Read){
    os << "Event: Read ";
  }else if (acc.type == Event::Type::Write){
    os << "Event: Write ";
  }
  if (acc.inst) {
    os << *acc.inst;
  } else {
    os << "missing inst";
  }

  return os;
}

void race::to_json(json &j, const Race &race) { j = json{{"access1", race.first}, {"access2", race.second}}; }

Report::Report(const std::vector<std::pair<const WriteEvent *, const MemAccessEvent *>> &racepairs) {
  size_t skipped = 0;
  for (auto const &racepair : racepairs) {
    Race race(racepair.first, racepair.second);
    if (race.missingLocation()) {// <- "Check if either access is missing source location"
      skipped++;
      continue;
    }

    races.insert(race);
  }
  if (skipped > 0) {
    llvm::errs() << "skipped " << skipped << " races with unknown location\n";
  }
}

void Report::dumpReport(const std::string &path) const {
  std::ofstream output(path, std::ofstream::out);
  json reportJSON(races);
  output << reportJSON;
  output.close();
}

void Reporter::collect(const WriteEvent *e1, const MemAccessEvent *e2) {
  racepairs.emplace_back(std::make_pair(e1, e2));
}

Report Reporter::getReport() const { return Report(racepairs); }

namespace {
std::optional<std::string> tryReadSourceLine(const std::optional<SourceLoc> &loc) {
  if (!loc) return std::nullopt;

  auto const dir = loc.value().directory.str();
  auto const filename = loc.value().filename.str();
  auto const line = loc.value().line;

  auto const sourcePath = fs::path(dir) / fs::path(filename);
  std::ifstream sourceFile(sourcePath, std::ios::binary);
  if (!sourceFile) {
    return std::nullopt;
  }

  std::string sourceLine;

  // read file up to the target line
  int count = 0;
  while (count < line - 1) {
    std::getline(sourceFile, sourceLine);
    count++;
  }

  // read target line
  std::getline(sourceFile, sourceLine);
  sourceFile.close();

  return sourceLine;
}
}  // namespace

llvm::raw_ostream &race::operator<<(llvm::raw_ostream &os, const Race &race) {
  std::string firstLine = "(-)";
  if (auto const source = tryReadSourceLine(race.first.location)) {
    firstLine = source.value();
  }

  std::string secondLine = "(-)";
  if (auto const source = tryReadSourceLine(race.second.location)) {
    secondLine = source.value();
  }

  // os << race.first.location << " " << race.second.location << "\n\t" << *race.first.inst << "\n\t\t" << firstLine
  //    << "\n\t" << *race.second.inst << "\n\t\t" << secondLine;

  // Print Location of race
  os << race.first.location << " " << race.second.location;

  // Try to print code snippets
  os << "\n\t---Source Code---";
  auto source1 = tryReadSourceLine(race.first.location);
  auto source2 = tryReadSourceLine(race.second.location);
  if (source1 && source2) {
    os << "\n\t" << race.first.location.value().line << "| " << source1.value();
    os << "\n\t" << race.second.location.value().line << "| " << source2.value();
  } else {
    os << "\n\t(Unable to read source file)";
  }

  os << "\n";
  // Print IR
  os << "\n\t---LLVM IR---";
  os << "\n\t";
  if (race.first.type == Event::Type::Read){
    os << "First Event: Read： ";
  }else if (race.first.type == Event::Type::Write){
    os << "First Event: Write： ";
  }

  os << "\n\t" << *race.first.inst;
  os << "\n\t" ;
  if (race.second.type == Event::Type::Read){
    os << "Second Event: Read： ";
  }else if (race.second.type == Event::Type::Write){
    os << "Second Event: Write： ";
  }

  os << "\n\t" << *race.second.inst;

  os << "\n";
  int count = 0;
  os << "\n\t---Callstacks---";
  for (auto const &call : race.first.callstack) {
    os << "\n\t> ";
    for(int i = 0; i < count; ++i) os << "  ";
    os << call;
    count++;
  }
  os << "\n\t====";
  count = 0;
  for (auto const &call : race.second.callstack) {
    os << "\n\t> ";
    for(int i = 0; i < count; ++i) os << "  ";
    os << call;
    count++;
  }

  os << "\n";

  // int count = 0;
  // os << "\n\t---Stacktrace---\n";
  // for (auto const &call : race.first.stacktrace) {
  //   os << "\t> ";
  //   for(int i = 0; i < count; ++i) os << "   ";

  //   os << call << "\n";
  //   count++;
  // }
  // count = 0;
  // os << "\t====\n";
  // for (auto const &call : race.second.stacktrace) {
  //   os << "\t> ";
  //   for(int i = 0; i < count; ++i) os << "   ";
    
  //   os << call << "\n";
  //   count++;
  // }
  return os;
}