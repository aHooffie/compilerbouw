#ifndef _TYPE_CHECKING_H_
#define _TYPE_CHECKING_H_

#include "types.h"

extern char *NodetypetoString(node *arg_node);
extern void typeError(info *arg_info, char *message);

extern node *TCglobaldef(node *arg_node, info *arg_info);
extern node *TCglobaldec(node *arg_node, info *arg_info);
extern node *TCfunction(node *arg_node, info *arg_info);
extern node *TCparameters(node *arg_node, info *arg_info);

extern node *TCassign(node *arg_node, info *arg_info);

extern node *TCbinop(node *arg_node, info *arg_info);
extern node *TCmonop(node *arg_node, info *arg_info);

extern node *TCvarlet(node *arg_node, info *arg_info);
extern node *TCvar(node *arg_node, info *arg_info);
extern node *TCbool(node *arg_node, info *arg_info);
extern node *TCfloat(node *arg_node, info *arg_info);
extern node *TCnum(node *arg_node, info *arg_info);

extern node *TCdoTypeChecking(node *syntaxtree);

#endif