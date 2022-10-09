#pragma once
#include "runtime.hpp"
#include <functional>

namespace lmlisp {

Runtime lm_init(Runtime_external_functions externals);

}
