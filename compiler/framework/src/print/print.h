
/**
 * @file print.h
 *
 * Functions to print node structures
 * 
 */

#ifndef _SAC_PRT_NODE_H_
#define _SAC_PRT_NODE_H_

#include "types.h"

/*  MODULE + DECLARATIONS AKA TOP STRUCTURE BELOW */
extern node *PRTdoPrint(node *syntaxtree);
extern node *PRTprogram(node *arg_node, info *arg_info);
extern node *PRTdeclarations(node *arg_node, info *arg_info);
extern node *PRTfunction(node *arg_node, info *arg_info);
extern node *PRTparameters(node *arg_node, info *arg_info);
extern node *PRTfunctionbody(node *arg_node, info *arg_info);
extern node *PRTglobaldec(node *arg_node, info *arg_info);
extern node *PRTglobaldef(node *arg_node, info *arg_info);
extern node *PRTvardeclaration(node *arg_node, info *arg_info);

/*  STATEMENTS BELOW */
extern node *PRTstmts(node *arg_node, info *arg_info);
extern node *PRTassign(node *arg_node, info *arg_info);
extern node *PRTifelse(node *arg_node, info *arg_info);
extern node *PRTwhile(node *arg_node, info *arg_info);
extern node *PRTreturn(node *arg_node, info *arg_info);
extern node *PRTfor(node *arg_node, info *arg_info);
extern node *PRTdowhile(node *arg_node, info *arg_info);
extern node *PRTfunctioncallstmt(node *arg_node, info *arg_info);
extern node *PRTexpressions(node *arg_node, info *arg_info);
extern node *PRTlocalfunction(node *arg_node, info *arg_info);

/*  EXPRESSIONS ARE BELOW  */
extern node *PRTbinop(node *arg_node, info *arg_info);
extern node *PRTfloat(node *arg_node, info *arg_info);
extern node *PRTnum(node *arg_node, info *arg_info);
extern node *PRTbool(node *arg_node, info *arg_info);
extern node *PRTvar(node *arg_node, info *arg_info);
extern node *PRTvarlet(node *arg_node, info *arg_info);
extern node *PRTmonop(node *arg_node, info *arg_info);
extern node *PRTfunctioncallexpr(node *arg_node, info *arg_info);
extern node *PRTcast(node *arg_node, info *arg_info);
extern node *PRTarrayexpr(node *arg_node, info *arg_info);
extern node *PRTternop(node *arg_node, info *arg_info);

/* MISCELLANEOUS */
extern node *PRTinstructions(node *arg_node, info *arg_info);
extern node *PRTsymboltableentry(node *arg_node, info *arg_info);
extern node *PRTsymboltable(node *arg_node, info *arg_info);
extern node *PRTerror(node *arg_node, info *arg_info);
extern node *PRTids(node *arg_node, info *arg_info);
extern void PRTindent(info *arg_info);

#endif /* _SAC_PRT_NODE_H_ */
