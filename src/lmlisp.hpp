#pragma once
#include "runtime.hpp"
#include <functional>
#include <vector>
#include <string>

namespace lmlisp {
  Runtime& init(
		std::string filename = "",
		std::vector<std::string> argv = {});
}
