#ifndef _ADD_SYMBOLTABLES_H_
#define _ADD_SYMBOLTABLES_H_

#include "types.h"

extern void stackPush(node *symboltable, info *arg_info);
extern void stackPop(info *arg_info);
extern bool stackEmpty(info *arg_info);

extern bool checkDuplicates(node *symboltableentry, char *name);
extern char *TypetoString(type Type);
extern node *travList(node *symboltableentry);
extern node *findOriginal(node *symboltableentry, char *name);
void printSymboltable(node *symboltableentry);

extern node *ASprogram(node *arg_node, info *arg_info);
extern node *ASglobaldec(node *arg_node, info *arg_info);
extern node *ASglobaldef(node *arg_node, info *arg_info);

extern node *ASfunction(node *arg_node, info *arg_info);
extern node *ASfunctioncallstmt(node *arg_node, info *arg_info);
extern node *ASfunctioncallexpr(node *arg_node, info *arg_info);
extern node *ASparameters(node *arg_node, info *arg_info);
extern node *ASreturn(node *arg_node, info *arg_info);

extern node *ASvar(node *arg_node, info *arg_info);
extern node *ASvarlet(node *arg_node, info *arg_info);
extern node *ASvardeclaration(node *arg_node, info *arg_info);

extern node *ASids(node *arg_node, info *arg_info);
extern node *ASdoAddSymbolTables(node *syntaxtree);

extern void stError(info *arg_info, node *arg_node, char *message, char *name);

#endif