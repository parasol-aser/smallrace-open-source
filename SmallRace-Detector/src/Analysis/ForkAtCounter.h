
#pragma once

#include "Trace/ProgramTrace.h"

namespace race {

class LockSet {
 public:
  explicit ForkAtCounter(const ProgramTrace &program);

};
}  // namespace race