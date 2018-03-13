#ifndef _ADD_SYMBOLTABLES_H_
#define _ADD_SYMBOLTABLES_H_

// moet dit hier ook? voor node?
#include "types.h"

extern node *ASprogram(node *arg_node, info *arg_info);
extern node *ASglobaldec(node *arg_node, info *arg_info);
extern node *ASglobaldef(node *arg_node, info *arg_info);

extern node *ASfunction(node *arg_node, info *arg_info);
extern node *ASfunctioncallstmt(node *arg_node, info *arg_info);
extern node *ASfunctioncallexpr(node *arg_node, info *arg_info);
extern node *ASparameters(node *arg_node, info *arg_info);

extern node *ASvar(node *arg_node, info *arg_info);
extern node *ASvarlet(node *arg_node, info *arg_info);
extern node *ASvardeclaration(node *arg_node, info *arg_info);

extern node *ASids(node *arg_node, info *arg_info);
extern node *ASdoAddSymbolTables(node *syntaxtree);

#endif