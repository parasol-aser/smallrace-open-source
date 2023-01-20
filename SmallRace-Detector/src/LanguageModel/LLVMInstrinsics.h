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

#include <llvm/ADT/StringRef.h>

namespace LLVMModel {

inline bool isDebug(const llvm::StringRef &funcName) {
  return funcName.equals("llvm.dbg.declare") || funcName.equals("llvm.dbg.value");
}
inline bool isLifetime(const llvm::StringRef &funcName) { return funcName.startswith("llvm.lifetime"); }
inline bool isStackSave(const llvm::StringRef &funcName) { return funcName.equals("llvm.stacksave"); }
inline bool isStackRestore(const llvm::StringRef &funcName) { return funcName.equals("llvm.stackrestore"); }
inline bool isMemcpy(const llvm::StringRef &funcName) { return funcName.startswith("llvm.memcpy"); }

// returns true for llvm APIS that have no effect on race detection
inline bool isNoEffect(const llvm::StringRef &funcName) {
  return isDebug(funcName) || isLifetime(funcName) || isStackSave(funcName) || isStackRestore(funcName) ||
         isMemcpy(funcName);
}

}  // namespace LLVMModel