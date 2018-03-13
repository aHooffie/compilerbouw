/* HOIHOI
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

  PROGRAM_DECLARATIONS( arg_node) = TRAVdo( PROGRAM_DECLARATIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldec");

  if (GLOBALDEC_DIMENSIONS( arg_node) != NULL)
    GLOBALDEC_DIMENSIONS( arg_node) = TRAVdo( GLOBALDEC_DIMENSIONS( arg_node), NULL);

  printf("Found a GLOBALDEC %s. Write to global ST.\n", GLOBALDEC_NAME(arg_node));
  
  DBUG_RETURN(arg_node);
}

node *ASglobaldef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldef");

  printf("Found a GLOBALDEF %s. Write to global ST.\n", GLOBALDEF_NAME(arg_node));

  if (GLOBALDEF_DIMENSIONS( arg_node) != NULL)
      GLOBALDEF_DIMENSIONS( arg_node) = TRAVdo( GLOBALDEF_DIMENSIONS( arg_node), NULL);

  if (GLOBALDEF_ASSIGN( arg_node) != NULL)
    GLOBALDEF_ASSIGN( arg_node) = TRAVdo( GLOBALDEF_ASSIGN( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

// FUNCTION RELATED
node *ASfunction(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");

  printf("Found a FUNCTION %s. Write to global ST & Create own ST.\n", FUNCTION_NAME(arg_node));

  FUNCTION_FUNCTIONBODY( arg_node) = TRAVdo( FUNCTION_FUNCTIONBODY( arg_node), NULL);
  FUNCTION_PARAMETERS( arg_node) = TRAVdo( FUNCTION_PARAMETERS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASparameters(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");

  printf("Found a PARAMETER %s. Write to function ST.\n", PARAMETERS_NAME(arg_node));


  if (PARAMETERS_NEXT( arg_node) != NULL)
    PARAMETERS_NEXT( arg_node) = TRAVdo( PARAMETERS_NEXT( arg_node), NULL);

  if (PARAMETERS_NEXT( arg_node) != NULL)
    PARAMETERS_NEXT( arg_node) = TRAVdo( PARAMETERS_NEXT( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASvardeclaration(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");

  printf("Found a VARDECLARATION %s. Write to function ST.\n", VARDECLARATION_NAME(arg_node));

  if (VARDECLARATION_DIMENSIONS( arg_node) != NULL)
    VARDECLARATION_DIMENSIONS( arg_node) = TRAVdo( VARDECLARATION_DIMENSIONS( arg_node), NULL);

  if (VARDECLARATION_INIT( arg_node) != NULL)
    VARDECLARATION_INIT( arg_node) = TRAVdo( VARDECLARATION_INIT( arg_node), NULL);

  if (VARDECLARATION_NEXT( arg_node) != NULL)
    VARDECLARATION_NEXT( arg_node) = TRAVdo( VARDECLARATION_NEXT( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASfunctioncallstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunctioncallstmt");

  printf("Found a FUNCTIONCALLSTMT %s. Write to function ST??\n", FUNCTIONCALLSTMT_NAME(arg_node));

  if (FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) != NULL)
    FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASfunctioncallexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FUNCTIONCALLEXPR");

  printf("Found a FUNCTIONCALLEXPR %s. Write to function ST???\n", FUNCTIONCALLEXPR_NAME(arg_node));

  if (FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) != NULL)
    FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

// VARIABLES INSIDE FUNCTIONS
node *ASvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvar");
  printf("Found a VAR %s. Write to function ST.\n", VAR_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *ASvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvarlet");
  printf("Found a VARLET %s. Write to function ST.\n", VARLET_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

// IDS & ARRAYEXPR??
node *ASids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASids");
  printf("Found an ID %s! DOES THIS REQUIRE ST?\n", IDS_NAME(arg_node));
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
