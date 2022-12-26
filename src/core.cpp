#include "core.hpp"

namespace lmlisp {

EnvironmentP init_core(Runtime_external_functions &f) {
  EnvironmentP core = std::make_shared<Environment>();
  
  core->set("+", func([](ElementP args) {
              if (args->type == LIST) {
                ListP args_l = args->to<List>();
#ifdef _LM_WITH_FLOAT
                float tot = 0;
#else
                int tot = 0;
#endif
                if (args_l->size() > 0) {
                  for (int i = 0; i < args_l->size(); i++) {
                    if (args_l->at(i)->type == NUMBER) {
                      tot += args_l->at(i)->to<Number>()->value();
                    } else
                      exit(1);
                  }
                  return num(tot);
                } else
                  return num(0);
              } else
                return nil();
            }));

  core->set("-", func([](ElementP args) {
              if (args->type == LIST) {
                ListP args_l = args->to<List>();
#ifdef _LM_WITH_FLOAT
                float tot = 0;
#else
                int tot = 0;
#endif
                if (args_l->size() > 0) {
                  if (args_l->at(0)->type == NUMBER) {
                    tot += args_l->at(0)->to<Number>()->value();
                    if (args_l->size() > 1) {
                      for (int i = 1; i < args_l->size(); i++) {
                        if (args_l->at(i)->type == NUMBER) {
                          tot -= args_l->at(i)->to<Number>()->value();
                        } else
                          exit(1);
                      }
                    }
                    return num(tot);
                  } else {
                    return nil();
                  }
                } else
                  return num(0);
              } else
                return nil();
            }));

  core->set("*", func([](ElementP args) {
              if (args->type == LIST) {
                ListP args_l = args->to<List>();
#ifdef _LM_WITH_FLOAT
                float tot = 1;
#else
                int tot = 1;
#endif
                if (args_l->size() > 0) {
                  for (int i = 0; i < args_l->size(); i++) {
                    if (args_l->at(i)->type == NUMBER) {
                      tot *= args_l->at(i)->to<Number>()->value();
                    } else
                      exit(1);
                  }
                  return num(tot);
                } else
                  return num(0);
              } else
                return nil();
            }));

  core->set("/", func([](ElementP args) {
              if (args->type == LIST) {
                ListP args_l = args->to<List>();
#ifdef _LM_WITH_FLOAT
                float tot = 1;
#else
                int tot = 1;
#endif
                if (args_l->size() > 0) {
                  if (args_l->at(0)->type == NUMBER) {
                    tot *= args_l->at(0)->to<Number>()->value();
                    if (args_l->size() > 1) {
                      for (int i = 1; i < args_l->size(); i++) {
                        if (args_l->at(i)->type == NUMBER) {
                          if (args_l->at(i)->to<Number>()->value() != 0)
                            tot /= args_l->at(i)->to<Number>()->value();
                          else {
                            // DIVISION BY ZERO
                            exit(1);
                          }
                        } else
                          exit(1);
                      }
                    }
                    return num(tot);
                  } else {
                    return nil();
                  }
                } else
                  return num(0);
              } else
                return nil();
            }));

  return core;
}
} // namespace lmlisp
