#ifndef _TYPE_CHECKING_H_
#define _TYPE_CHECKING_H_

#include "types.h"

extern node *TCbinop (node *arg_node, info *arg_info);
extern node *TCparameters (node *arg_node, info *arg_info);

extern node *TCdoTypeChecking(node *syntaxtree);


#endif