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

#include "Analysis/HappensBeforeGraph.h"

/*
Happens before depends on each event having an increasing ID per thread:

T1  T2
==  ==
e1  e1
e2  e2
e3  e3
e4  e4

Then edges are added between sync events.
If e2 is a fork spawning t2, and e3 is the corresponding join,
edges are added between events as shown below:

T1  T2
==  ==
e1
e2->e1
    e2
    e3
e3<-e4
e4

Now call any event with a sync edge a syncEvent and store them separately.

T1Sync  T2Sync
======  ======
e2      e1
e3      e4

To check if an event A on T1 can reach an event B on T2
- find the closest sync event after A on T1 (syncAfterA)
- find the closest sync event before B on T2 (syncBeforeB)
- check if syncAfterA can reach syncBeforeB
  - normal graph traversal on the sync event edges

This shrinks the graph that needs to be searched.
*/

using namespace race;

namespace {
const ThreadTrace *getForkedThread(const ForkEvent *fork, const ProgramTrace &program) {
  for (auto const &thread : program.getThreads()) {
    if (thread->spawnSite.has_value() && thread->spawnSite.value() == fork) {
      return thread.get();
    }
  }
  assert(false && "Some forkEvent in does not correspond to a thread");
  return nullptr;
}

const ForkEvent *getForkWithHandle(const llvm::Value *handle, const ThreadTrace &thread) {
  for (auto const &fork : thread.getForkEvents()) {
    if (fork->getIRInst()->getThreadHandle() == handle) {
      return fork;
    }
  }

  return nullptr;
}

const ForkEvent *getForkWithHandle(const llvm::Value *handle, const ProgramTrace &program) {
  for (auto const &thread : program.getThreads()) {
    auto fork = getForkWithHandle(handle, *thread);
    if (fork != nullptr) {
      return fork;
    }
  }

  return nullptr;
}

const ForkEvent *getCorrespondingFork(const JoinEvent *join, const ProgramTrace &program) {
  auto const joinHandle = join->getIRInst()->getThreadHandle();

  // Check for fork on this thread with matching handle first
  if (auto fork = getForkWithHandle(joinHandle, join->getThread()); fork != nullptr) {
    return fork;
  }

  // Check for fork in whole program with matching handle
  if (auto const fork = getForkWithHandle(joinHandle, program); fork != nullptr) {
    return fork;
  }

  if (auto load = llvm::dyn_cast<llvm::LoadInst>(joinHandle)) {
    // Check if loaded value is handle
    auto loadedHandle = load->getPointerOperand();
    if (auto const fork = getForkWithHandle(loadedHandle, program); fork != nullptr) {
      return fork;
    }

    // Check if a store to the value being loaded is the handle
    for (const llvm::User *user : loadedHandle->users()) {
      if (user == joinHandle) continue;
      if (auto store = llvm::dyn_cast<llvm::StoreInst>(user)) {
        if (auto const fork = getForkWithHandle(store->getPointerOperand(), program); fork != nullptr) {
          return fork;
        }
      }
    }
  } else {
    // TODO: what is this doing?
    while (auto phi = llvm::dyn_cast<llvm::PHINode>(joinHandle)) {
      auto incoming = phi->getIncomingValue(0);
      if (llvm::isa<llvm::CallInst>(incoming)) {
        if (auto const fork = getForkWithHandle(incoming, program); fork != nullptr) {
          return fork;
        }
      }
    }
  }

  // If we still have not found the joined thread, use heuristics
  // pick the thread that was spawned by the closet fork event from this join site
  auto const forks = join->getThread().getForkEvents();
  for (auto it = forks.rbegin(), rend = forks.rend(); it != rend; ++it) {
    auto const fork = *it;
    if (fork->getID() < join->getID()) {
      llvm::errs() << "Using nearest fork heuristic to find corresponding fork!\n\tJoin: " << *join->getInst()
                   << "\n\tFork: " << *fork->getInst() << "\n";
      return fork;
    }
  }

  llvm::errs() << "Unable to find corresponding fork for join event: " << *join->getInst() << "\n";
  return nullptr;
}

const ThreadTrace *getJoinedThread(const JoinEvent *join, const ProgramTrace &program) {
  auto fork = getCorrespondingFork(join, program);
  if (fork != nullptr) {
    return getForkedThread(fork, program);
  }
  return nullptr;
}

}  // namespace

HappensBeforeGraph::HappensBeforeGraph(const race::ProgramTrace &program) {
  // Barriers are handled by adding two edges between each barrier event
  // e.g.
  //   T1       T2
  //   barr <-> barr
  // This enforces a happens-before ordering that models barrier behaviour

  // To do this, as we traverse over the program threads we keep a map of
  // the last event that corresponds to each barrier. If there is already
  // an event in the map (meaning a previous event has visited this barrier)
  // the dual-edge is added between the prev event and the current event,
  // and the current event is added to the map so that the next event to reach
  // this barrier will add the dual-edge with the curent event.
  std::map<const llvm::Instruction *, const BarrierEvent *> lastBarrier;
  std::vector<const Event*> queueWrites;
  std::vector<const Event*> queueReads;
  auto addBarrierEdge = [&lastBarrier, this](const BarrierEvent *event) {
    auto const barrierInst = event->getInst();

    auto it = lastBarrier.find(barrierInst);
    if (it != lastBarrier.end()) {
      // Add two way sync edge so neither event can happen before the other
      auto const prevEvent = it->second;
      addSyncEdge(event, prevEvent);
      addSyncEdge(prevEvent, event);
    }

    // Add this event as most recent to hit the barrier
    lastBarrier[barrierInst] = event;

    // FIXME: This logic will likely need to be updated for non-omp barriers
    assert(event->getIRInst()->type == IR::Type::OpenMPBarrier && "Check that non-omp barriers work correctly");
  };

  for (auto const &thread : program.getThreads()) {
    for (auto const &event : thread->getEvents()) {
      switch (event->type) {
        case Event::Type::Fork: {
          auto forkEvent = llvm::cast<ForkEvent>(event.get());
          auto forkedThread = getForkedThread(forkEvent, program);
          if (forkedThread == nullptr) {
            // TODO: log warning
            llvm::errs() << "Could not find fork!\n";
            continue;
          }
          // If thread has no events just leave it out of happens before
          if (!forkedThread->getEvents().empty()) {
            addSyncEdge(forkEvent, forkedThread->getEvents().front().get());
          }
          break;
        }
        case Event::Type::Join: {
          auto joinEvent = llvm::cast<JoinEvent>(event.get());
          auto joinedThread = getJoinedThread(joinEvent, program);
          if (joinedThread == nullptr) {
            // TODO: log warning
            llvm::errs() << "Could not find join!\n";
            continue;
          }
          // If thread has no events just leave it out of happens before
          if (!joinedThread->getEvents().empty()) {
            addSyncEdge(joinedThread->getEvents().back().get(), joinEvent);
          }
          break;
        }
        case Event::Type::Barrier: {
          auto barrierEvent = llvm::cast<BarrierEvent>(event.get());
          addBarrierEdge(barrierEvent);
          break;
        }
        case Event::Type::Write: {
          //if write on queue, add to the event vector
          auto inst = event.get()->getInst();
          //llvm::outs() << *inst << "\n";
          if(llvm::isa<llvm::CallBase>(inst)){
            if(llvm::cast<llvm::CallBase>(inst)->getCalledFunction()->getName().startswith("st.nextPut")){
              queueWrites.push_back(llvm::cast<WriteEvent>(event.get()));
            }
          }
        }
        case Event::Type::Call: {
          //if executing blocks read from queue, add to the event vector
          auto inst = event.get()->getInst();
          if(llvm::isa<llvm::CallBase>(inst)){
            auto cb = llvm::cast<llvm::CallBase>(inst);
            if(cb->getCalledFunction()->getName().str() == "st.value"){
              if(llvm::isa<EnterCallEvent>(event.get()))
              queueReads.push_back(llvm::cast<EnterCallEvent>(event.get()));
            }
              
          }
        }
        default:
          // Do Nothing
          break;
      }
    }
  }
  if(queueWrites.size()==queueReads.size()){
    for(int i=0;i<queueReads.size();i++)
      addSyncEdge(queueWrites[i], queueReads[i]);
  }
}

void HappensBeforeGraph::addSync(const Event *syncEvent) {
  auto &syncs = threadSyncs[syncEvent->getThread().id];
  EventPID syncPID(syncEvent);
  // find where to insert sync to keep list in ascending sorted order
  auto it = std::lower_bound(syncs.begin(), syncs.end(), syncPID);
  // dont insert a sync twice
  if (it != syncs.end() && *it == syncPID) {
    return;
  }
  syncs.insert(it, syncPID);
}

void HappensBeforeGraph::addSyncEdge(const Event *src, const Event *dst) {
  EventPID srcID(src);
  EventPID dstID(dst);

  auto &edges = syncEdges[srcID];
  edges.insert(dstID);

  addSync(src);
  addSync(dst);
}

bool HappensBeforeGraph::canReach(const Event *src, const Event *dst) const {
  auto srcSync = findNextSync(src);
  if (!srcSync.has_value()) {
    return false;
  }

  auto dstSync = findPrevSync(dst);
  if (!dstSync.has_value()) {
    return false;
  }

  if (hasEdge(srcSync.value(), dstSync.value())) {
    return true;
  }

  return syncEdgesDFS(srcSync.value(), dstSync.value());
}

bool HappensBeforeGraph::syncEdgesDFS(HappensBeforeGraph::EventPID src, HappensBeforeGraph::EventPID dst) const {
  std::vector<EventPID> worklist;
  std::set<EventPID> visited;

  auto const addToWorklist = [&worklist, &visited](EventPID node) {
    // Only add to worklist if not already visited
    if (visited.find(node) != visited.end()) {
      return;
    }
    worklist.push_back(node);
  };

  addToWorklist(src);
  while (!worklist.empty()) {
    auto const node = worklist.back();
    visited.insert(node);
    worklist.pop_back();

    if (node == dst || hasEdge(src, dst)) {
      return true;
    }

    // Add next nodes from sync edges
    if (auto it = syncEdges.find(node); it != syncEdges.end()) {
      for (auto const next : it->second) {
        addToWorklist(next);
      }
    }

    // Add next sync event after this one on same thread
    if (auto opt = findNextSyncAfter(node); opt.has_value()) {
      addToWorklist(opt.value());
    }
  }

  return false;
}

bool HappensBeforeGraph::hasEdge(HappensBeforeGraph::EventPID src, HappensBeforeGraph::EventPID dst) const {
  if (src == dst) return true;

  auto it = syncEdges.find(src);
  if (it == syncEdges.end()) {
    return false;
  }
  auto const &destinations = it->second;
  return destinations.find(dst) != destinations.end();
}

std::optional<HappensBeforeGraph::EventPID> HappensBeforeGraph::findNextSyncAfter(EventPID node) const {
  auto it = threadSyncs.find(node.tid);
  if (it == threadSyncs.end()) {
    return std::nullopt;
  }

  auto const &syncs = it->second;
  auto syncIt = std::upper_bound(syncs.begin(), syncs.end(), node);
  if (syncIt == syncs.end()) {
    return std::nullopt;
  }

  return *syncIt;
}

std::optional<HappensBeforeGraph::EventPID> HappensBeforeGraph::findNextSync(HappensBeforeGraph::EventPID node) const {
  auto it = threadSyncs.find(node.tid);
  if (it == threadSyncs.end()) {
    return std::nullopt;
  }

  auto const &syncs = it->second;
  auto syncIt = std::lower_bound(syncs.begin(), syncs.end(), node);
  if (syncIt == syncs.end()) {
    return std::nullopt;
  }
  return *syncIt;
}
std::optional<HappensBeforeGraph::EventPID> HappensBeforeGraph::findNextSync(const Event *e) const {
  return findNextSync(EventPID(e));
}

std::optional<HappensBeforeGraph::EventPID> HappensBeforeGraph::findPrevSync(const Event *e) const {
  EventPID node(e);

  auto it = threadSyncs.find(node.tid);
  if (it == threadSyncs.end()) {
    return std::nullopt;
  }

  // TODO: check if there is a std algorithm that can do this faster
  // look for sync with lower id, starting from back of list
  auto const &syncs = it->second;
  for (auto syncIt = syncs.rbegin(), rend = syncs.rend(); syncIt != rend; ++syncIt) {
    auto const sync = *syncIt;
    if (sync <= node) {
      return sync;
    }
  }

  return std::nullopt;
}

void HappensBeforeGraph::debugDump(llvm::raw_ostream &os) const {
  for (auto const &[tid, syncs] : threadSyncs) {
    os << "T" << tid << " Syncs";
    for (auto const &sync : syncs) {
      os << "\n\t" << sync.tid << ":" << sync.eid;
    }
    os << "\n";
  }

  os << "==== Edges ====\n";
  for (auto const &[src, dsts] : syncEdges) {
    os << src.tid << ":" << src.eid << " ->";
    for (auto const &dst : dsts) {
      os << "\n\t" << dst.tid << ":" << src.tid << "\n";
    }
  }
  os << "\n";
}
