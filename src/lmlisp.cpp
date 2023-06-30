#include "lmlisp.hpp"
#include "types.hpp"

namespace lmlisp {
  Runtime& init(
		std::string filename,
		std::vector<std::string> argv) {
    if (Runtime::current == nullptr) {
      Runtime::current = new Runtime(filename, argv);
    }
    return *Runtime::current;
  }
} // namespace lmlisp
