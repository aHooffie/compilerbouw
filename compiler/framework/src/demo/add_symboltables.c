/*
 * Module: Traversing and looking for symbol table links
 * Prefix: AS
 */

#include "add_symboltables.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"

/* Found a node that should have a symbol table entry (either read or write) */

// GLOBALS
node *ASprogram(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASprog");
  
  printf("Found a Program node. This should create a ST!\n");
  PROGRAM_DECLARATIONS( arg_node) = TRAVdo( PROGRAM_DECLARATIONS( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldec");
  printf("Found a GLOBALDEC %s. Write to global ST.\n", GLOBALDEC_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *ASglobaldef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldef");
  printf("Found a GLOBALDEF %s. Write to global ST.\n", GLOBALDEF_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

// FUNCTION RELATED
node *ASfunction(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");
  FUNCTION_FUNCTIONBODY( arg_node) = TRAVdo( FUNCTION_FUNCTIONBODY( arg_node), arg_info);
  FUNCTION_PARAMETERS( arg_node) = TRAVdo( FUNCTION_PARAMETERS( arg_node), arg_info);

  printf("Found a FUNCTION %s. Write to global ST & Create own ST.\n", FUNCTION_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *ASparameters(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");
  printf("Found a PARAMETER %s. Write to function ST.\n", PARAMETERS_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *ASvardeclaration(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");
  printf("Found a VARDECLARATION %s. Write to function ST.\n", VARDECLARATION_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *ASfunctioncallstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunctioncallstmt");
  printf("Found a FUNCTIONCALLSTMT %s. Write to function ST??", FUNCTIONCALLSTMT_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *ASfunctioncallexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FUNCTIONCALLEXPR");
  printf("Found a FUNCTIONCALLEXPR %s. Write to function ST???", FUNCTIONCALLEXPR_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

// VARIABLES INSIDE FUNCTIONS
node *ASvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvar");
  printf("Found a VAR %s. Write to function ST.", VAR_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *ASvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvarlet");
  printf("Found a VARLET %s. Write to function ST.", VARLET_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

// IDS & ARRAYEXPR??
node *ASids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASids");
  printf("Found an ID! DOES THIS REQUIRE ST?%s", IDS_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

/* Traversal start function */
node *ASdoAddSymbolTables(node *syntaxtree)
{

  DBUG_ENTER("ASdoAddSymbolTables");

  TRAVpush( TR_as);
  syntaxtree = TRAVdo(syntaxtree, NULL);
  TRAVpop();

  CTInote("Traversing done...\n");

  DBUG_RETURN(syntaxtree);
}
