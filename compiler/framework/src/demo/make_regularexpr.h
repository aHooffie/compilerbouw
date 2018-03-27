#ifndef _MAKE_REGULAREXPR_H_
#define _MAKE_REGULAREXPR_H_

#include "types.h"

extern node *REglobaldec(node *arg_node, info *arg_info);
extern node *REglobaldef(node *arg_node, info *arg_info);
extern node *REvardeclaration(node *arg_node, info *arg_info);

bool checkPrevDeclaration(node *symboltableentry, char *name);

extern node *REdoRegularExpressions(node *syntaxtree);

#endif