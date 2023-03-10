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

#include "PointerAnalysis/Graph/ConstraintGraph/SCCIterator.h"
#include "SolverBase.h"

namespace pta {

template <typename LangModel>
class AndersenWave : public SolverBase<LangModel, AndersenWave<LangModel>> {
 public:
  using super = SolverBase<LangModel, AndersenWave<LangModel>>;
  using PtsTy = typename super::PtsTy;
  using PT = PTSTrait<PtsTy>;
  using ConsGraphTy = ConstraintGraph<typename super::ctx>;
  using CGNodeTy = CGNodeBase<typename super::ctx>;
  using GT = llvm::GraphTraits<ConsGraphTy>;
  using ctx = typename super::ctx;
  using LMT = typename super::LMT;

 protected:
  void runSolver(LangModel &langModel) {
    bool changed;
    ConsGraphTy &consGraph = *(super::getConsGraph());

    do {
      changed = false;

      // first do SCC detection and topo-sort
      auto copy_it = scc_begin<ctx, Constraints::copy>(consGraph);
      auto copy_ie = scc_end<ctx, Constraints::copy>(consGraph);

      for (; copy_it != copy_ie; ++copy_it) {
        const std::vector<CGNodeTy *> &scc = *copy_it;
        if (copy_it.hasLoop()) {
          // We do not merge SCC in this solver,
          // As the current implementation is much more clear, and it works
          // fine.

          // instead of collapsing SCC into super nodes,
          // simply propagate points to set within the SCC the node.
          // within the same scc, all nodes should have exactly the same pts
          for (auto nit = ++(scc.begin()), nie = scc.end(); nit != nie; nit++) {
            super::processCopy(*nit, scc.front());
          }
          // now scc.front get all pts in the pts
          for (auto nit = ++(scc.begin()), nie = scc.end(); nit != nie; nit++) {
            // propagate pts back from the front node to nodes in
            // the SCC.
            CGNodeTy *node = *nit;
            super::processCopy(scc.front(), node);

            for (auto cit = node->succ_copy_begin(), cie = node->succ_copy_end(); cit != cie; cit++) {
              super::processCopy(node, *cit);
            }
          }
        }
        // now handle the node at the front
        CGNodeTy *curNode = scc.front();
        for (auto cit = curNode->succ_copy_begin(), cie = curNode->succ_copy_end(); cit != cie; cit++) {
          super::processCopy(curNode, *cit);
        }
      }

      // then handle STORE/LOAD edges.
      size_t nodeNum = consGraph.getNodeNum();
      for (NodeID id = 0; id < nodeNum; id++) {
        CGNodeTy *curNode = consGraph.getNode(id);

        for (auto it = curNode->succ_load_begin(), ie = curNode->succ_load_end(); it != ie; it++) {
          changed |= super::processLoad(curNode, *it);
        }

        for (auto it = curNode->succ_store_begin(), ie = curNode->succ_store_end(); it != ie; it++) {
          changed |= super::processStore(curNode, *it);
        }
        for (auto it = curNode->succ_offset_begin(), ie = curNode->succ_offset_end(); it != ie; it++) {
          changed |= super::processOffset(curNode, *it);
        }
      }
    } while (changed);
  }

  friend super;
};

}  // namespace pta
