#ifndef _MAKE_RE_LOCAL_H_
#define _MAKE_RE_LOCAL_H_

#include "types.h"

extern node *RELfunctionbody(node *arg_node, info *arg_info);
extern node *RELvardeclaration(node *arg_node, info *arg_info);

extern node *RELdoRegularExpr(node *syntaxtree);

#endif