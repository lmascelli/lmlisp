#pragma once

#define TEST_DO_OR_EXC(TEST, CODE, MSG)			\
  if ((TEST)) { CODE } else { return exc(MSG)->el(); }

#define TEST_EQ_N_ARGS_OR_EXC(LIST, N, MSG, CODE)			\
  if ((LIST)->size() != (N)) { return exc((MSG))->el(); } else { CODE }

#define TEST_SPECIAL_FORM(EL, FORM)					\
  ((EL)->type == SYMBOL and (EL)->to<Symbol>()->value() == (FORM))

#define CHECK_N_TYPE_OR_EXC(LIST, N, TYPE, MSG, CODE)			\
  if ((LIST)->at((N))->type != TYPE) { return exc((MSG))->el(); } else { CODE }
