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

#include <llvm/ADT/iterator.h>
//#include <llvm/IR/CallSite.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Operator.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>

#include <queue>

namespace pta {

struct Noop {
  template <typename... Args>
  __attribute__((always_inline)) void operator()(Args &&...) {}
};

inline llvm::raw_ostream &dbg_os() {
#ifndef NDEBUG
  return llvm::dbgs();
#else
  return llvm::nulls();
#endif
}

template <typename BOOL>
bool AndBools(BOOL b) {
  return b;
}

template <typename BOOL, typename... BOOLS>
bool AndBools(BOOL cur, BOOLS... remain) {
  if (cur) {
    return AndBools(remain...);
  }
  return false;
}

template <typename T, size_t... N>
bool isTupleEquelOnIndex(const T &T1, const T &T2, std::index_sequence<N...> sequence) {
  return AndBools((std::get<N>(T1) == std::get<N>(T2))...);
}

template <typename... Args>
bool isTupleEqual(const std::tuple<Args...> &tuple1, const std::tuple<Args...> &tuple2) {
  static_assert(sizeof...(Args) > 0);

  return isTupleEquelOnIndex(tuple1, tuple2, std::index_sequence_for<Args...>{});
}

// instead of compare pointer value, compare they content of them
template <typename PtrTy, typename Comparator = std::less<PtrTy>>
class PtrContentComparator : public std::binary_function<PtrTy *, PtrTy *, bool> {
  using comp = Comparator;
  comp _M_comp;

 public:
  bool operator()(const PtrTy *__x, const PtrTy *__y) { return _M_comp(*__x, *__y); }
};

template <typename PtrTy, typename Comparator = std::less<PtrTy>>
class UniquePtrContentComparator : public std::binary_function<std::unique_ptr<PtrTy>, std::unique_ptr<PtrTy>, bool> {
  using comp = Comparator;
  comp _M_comp;

 public:
  bool operator()(const std::unique_ptr<PtrTy> &__x, const std::unique_ptr<PtrTy> &__y) const {
    return _M_comp(*(__x.get()), *(__y.get()));
  }
};

inline llvm::Type *getBoundedArrayTy(llvm::Type *elemType, size_t num) {
  if (llvm::ArrayType::isValidElementType(elemType)) {
    return llvm::ArrayType::get(elemType, num);
  }

  return nullptr;
}

inline const llvm::Type *stripArray(const llvm::Type *T) {
  while (auto AT = llvm::dyn_cast<llvm::ArrayType>(T)) {
    T = AT->getArrayElementType();  // strip array
  }
  return T;
}

inline llvm::Type *stripArray(llvm::Type *T) {
  while (auto AT = llvm::dyn_cast<llvm::ArrayType>(T)) {
    T = AT->getArrayElementType();  // strip array
  }
  return T;
}

llvm::Type *isStructWithFlexibleArray(const llvm::StructType *ST);
llvm::StructType *getConvertedFlexibleArrayType(const llvm::StructType *ST, llvm::Type *flexbleArrayTy);

inline llvm::Type *getUnboundedArrayTy(llvm::Type *elemType) {
  if (auto ST = llvm::dyn_cast<llvm::StructType>(elemType)) {
    if (llvm::Type *lastElemTy = isStructWithFlexibleArray(ST)) {
      return getConvertedFlexibleArrayType(ST, lastElemTy);
    }
  }

  return getBoundedArrayTy(elemType, std::numeric_limits<uint32_t>::max());
}

// for debugging purpose
void recordCGNode(const llvm::Value *val);
void dumpCGNodeDistribution();

std::string getSourceDir(const llvm::Value *val);

void prettyFunctionPrinter(const llvm::Function *func, llvm::raw_ostream &os);

// whether the indirect call site compatible with the target function
bool isCompatibleCall(const llvm::Instruction *indirectCall, const llvm::Function *target);

std::string getDemangledName(llvm::StringRef mangledName);

llvm::StringRef stripNumberPostFix(llvm::StringRef originalName);

const llvm::Type *getTypeAtOffset(const llvm::Type *type, size_t offset, const llvm::DataLayout &DL,
                                  bool stripArray = true);

bool isZeroOffsetTypeInRootType(const llvm::Type *rootType, const llvm::Type *elemType, const llvm::DataLayout &);

}  // namespace pta
