/*
 * Module: Traversing and looking for symbol table links
 * Prefix: M
 */

#include "milestone5.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"

/* Found a node that should have a symbol table entry (either read or write) */
// GLOBALS
node *Mglobaldec(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a XX! %s", GLOBALDEC_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mglobaldefnode *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a XX! %s", GLOBALDEF_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

// FUNCTION RELATED
node *Mfunction(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a XX! %s", FUNCTION_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mfunctioncallstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a XX! %s", FUNCTIONCALLSTMT_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mfunctioncallexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a XX! %s", FUNCTIONCALLEXPR_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

// VARIABLES INSIDE FUNCTIONS

node *Mvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a VAR! %s", VAR_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a VARLET! %s", VARLET_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mvardeclaration(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvar");
  printf("Found a VARDECLARATION! %s", VARDECLARATION_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

// ADD: PARAMETERS, IDS FOR IF WHILE DOWHILE RETURN BOOL(?) ARRAYS LOCALFUNCTION (?)

/*
 * Traversal start function
 */

node *SIdoSumInts(node *syntaxtree)
{
  info *arg_info;

  DBUG_ENTER("SIdoSumIns");

  arg_info = MakeInfo();

  TRAVpush(TR_si);
  syntaxtree = TRAVdo(syntaxtree, arg_info);
  TRAVpop();

  CTInote("Sum of integer constants: %d", INFO_SUM(arg_info));

  arg_info = FreeInfo(arg_info);

  DBUG_RETURN(syntaxtree);
}
