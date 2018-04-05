#ifndef _GEN_BYTE_CODE_H_
#define _GEN_BYTE_CODE_H_
#include "types.h"

extern node *GBCfunctioncallstmt(node *arg_node, info *arg_info);
extern node *GBCfunctioncallexpr(node *arg_node, info *arg_info);

/* Variable declarations. */
extern node *GBCglobaldec(node *arg_node, info *arg_info);
extern node *GBCglobaldef(node *arg_node, info *arg_info);
extern node *GBCfunction(node *arg_node, info *arg_info);
extern node *GBCfunctionbody(node *arg_node, info *arg_info);
extern node *GBCvardeclaration(node *arg_node, info *arg_info);
extern node *GBCvarlet(node *arg_node, info *arg_info);

/* Statements*/
extern node *GBCifelse(node *arg_node, info *arg_info);
extern node *GBCfor(node *arg_node, info *arg_info);
extern node *GBCwhile(node *arg_node, info *arg_info);
extern node *GBCdowhile(node *arg_node, info *arg_info);
extern node *GBCreturn(node *arg_node, info *arg_info);
extern node *GBCassign(node *arg_node, info *arg_info);

/* Expressions */
extern node *GBCexpressions(node *arg_node, info *arg_info);
extern node *GBCternop(node *arg_node, info *arg_info);
extern node *GBCbinop(node *arg_node, info *arg_info);
extern node *GBCmonop(node *arg_node, info *arg_info);
extern node *GBCvar(node *arg_node, info *arg_info);
extern node *GBCnum(node *arg_node, info *arg_info);
extern node *GBCfloat(node *arg_node, info *arg_info);
extern node *GBCcast(node *arg_node, info *arg_info);
extern node *GBCbool(node *arg_node, info *arg_info);

extern void printInstructions(info *arg_info);
extern char *instrToString(instr i);
extern void addNode(node *arg_node, info *arg_info);
extern node *GBCdoGenByteCode(node *syntaxtree);

#endif
