#include "lmlisp.hpp"

namespace lmlisp {

struct temp {
  Runtime type;
};

Runtime lm_init(Runtime_external_functions externals) {
  Runtime ret(externals);
  return static_cast<Runtime>(ret);
}
} // namespace lmlisp
