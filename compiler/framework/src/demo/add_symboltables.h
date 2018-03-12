#ifndef _ADD_SYMBOLTABLES_H_
#define _ADD_SYMBOLTABLES_H_

// moet dit hier ook? voor node?
#include "types.h"

extern node *Mglobaldec(node *arg_node, info *arg_info);
extern node *Mglobaldefnode( *arg_node, info *arg_info);

extern node *Mfunction(node *arg_node, info *arg_info);
extern node *Mfunctioncallstmt(node *arg_node, info *arg_info);
extern node *Mfunctioncallexpr(node *arg_node, info *arg_info);

extern node *Mvar(node *arg_node, info *arg_info);
extern node *Mvarlet(node *arg_node, info *arg_info);
extern node *Mvardeclaration(node *arg_node, info *arg_info);

#endif