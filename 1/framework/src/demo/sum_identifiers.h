#ifndef _SUM_IDENTIFIERS_H_
#define _SUM_IDENTIFIERS_H_

#include "types.h"

extern node *SIDvar(node *arg_node, info *arg_info);
extern node *SIDvarlet(node *arg_node, info *arg_info);
extern node *SIDdoSumIdentifiers(node *syntaxtree);

#endif
