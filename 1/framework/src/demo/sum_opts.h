#ifndef _SUM_OPTS_H_
#define _SUM_OPTS_H_

#include "types.h"

extern node *SObinop(node *arg_node, info *arg_info);
extern node *SOmodule(node *arg_node, info *arg_info);
extern node *SOdoSumOpts(node *syntaxtree);

#endif
