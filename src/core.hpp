#pragma once

#include "types.hpp"
#include "runtime.hpp"

namespace lmlisp {
  EnvironmentP init_core(Runtime_external_functions &f);
}
