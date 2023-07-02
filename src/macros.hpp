#pragma once

#define THROW(EXC) { \
  Runtime::raised = true; \
  Runtime::exc_value = str(EXC); \
  return nil()->el(); \
}

#define CHECK_EXC \
  (Runtime::raised and not Runtime::handled)

#define TEST_DO_OR_EXC(TEST, CODE, MSG)			\
  if ((TEST)) { CODE } else {THROW(MSG); }

#define TEST_EQ_N_ARGS_OR_EXC(LIST, N, MSG, CODE)			\
  if ((LIST)->size() != (N)) { THROW(MSG); } else { CODE }

#define TEST_SPECIAL_FORM(EL, FORM)					\
  ((EL)->type == SYMBOL and (EL)->to<Symbol>()->value() == (FORM))

#define CHECK_N_TYPE_OR_EXC(LIST, N, TYPE, MSG, CODE)			\
  if ((LIST)->at((N))->type != TYPE) { THROW(MSG); } else { CODE }

