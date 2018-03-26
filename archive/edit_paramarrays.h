#ifndef _EDIT_PARAMARRAYS_H_
#define _EDIT_PARAMARRAYS_H_

#include "types.h"

extern void *EPdoEditParamarrays(node *syntaxtree);

extern node *EPfunction(node *arg_node, info *arg_info);
extern node *EPfunctioncallstmt(node *arg_node, info *arg_info);
extern node *EPfunctioncallexpr(node *arg_node, info *arg_info);
extern node *EPparameters(node *arg_node, info *arg_info);
extern node *EParrayexpr(node *arg_node, info *arg_info);
extern node *EPexpressions(node *arg_node, info *arg_info);
extern node *EPids(node *arg_node, info *arg_info);

#endif
