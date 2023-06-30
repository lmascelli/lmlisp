#pragma once

#include "types.hpp"
#include "runtime.hpp"
#include <vector>
#include <string>

namespace lmlisp {
  EnvironmentP init_core(std::vector<std::string> argv);
  void post_init(Runtime &r, std::string filename);
}
