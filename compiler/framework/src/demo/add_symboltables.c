/*
 * Module: Traversing and looking for symbol table links
 * Prefix: M
 */

#include "add_symboltables.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"



/*
 * INFO structure
 */

struct INFO {
  // int sum;
};


/*
 * INFO macros
 */

// #define INFO_SUM(n)  ((n)->sum)



/*
 * INFO functions
 */

static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  // INFO_SUM( result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}


/* Found a node that should have a symbol table entry (either read or write) */


// PROGRAM
node *Mprog(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mprog");
  // printf("Found a GLOBALDEC! %s", GLOBALDEC_NAME(arg_node));
  DBUG_RETURN(arg_node);
}


// GLOBALS
node *Mdecls(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mdecls");
  // printf("Found a GLOBALDEC! %s", GLOBALDEC_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mglobaldec(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mglobaldec");
  printf("Found a GLOBALDEC! %s", GLOBALDEC_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mglobaldefnode( *arg_node, info *arg_info)
{
  DBUG_ENTER("Mglobaldefnode");
  printf("Found a GLOBALDEFNODE! %s", GLOBALDEF_NAME(arg_node));
  DBUG_RETURN(arg_node);
}


// FUNCTION RELATED
node *Mfunction(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mfunction");
  printf("Found a FUNCTION! %s", FUNCTION_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mfunctioncallstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mfunctioncallstmt");
  printf("Found a FUNCTIONCALLSTMT! %s", FUNCTIONCALLSTMT_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mfunctioncallexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FUNCTIONCALLEXPR");
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
  DBUG_ENTER("Mvarlet");
  printf("Found a VARLET! %s", VARLET_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

node *Mvardeclaration(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mvardeclaration");
  printf("Found a VARDECLARATION! %s", VARDECLARATION_NAME(arg_node));
  DBUG_RETURN(arg_node);
}


// PARAMETERS
node *Mparams(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mparams");
  printf("Found a PARAMETER! %s", PARAMETERS_NAME(arg_node));
  DBUG_RETURN(arg_node);
}


// IDS
node *Mids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mids");
  printf("Found an ID! %s", IDS_NAME(arg_node));
  DBUG_RETURN(arg_node);
}


// LOOP RELATED
// node *Mfor(node *arg_node, info *arg_info)
// {
//   DBUG_ENTER("Mfor");
//   // printf("Found a ...! %s", IDS_NAME(arg_node));
//   DBUG_RETURN(arg_node);
// }

// node *Mwhileloop(node *arg_node, info *arg_info)
// {
//   DBUG_ENTER("Mwhileloop");
//   // printf("Found a ...! %s", IDS_NAME(arg_node));
//   DBUG_RETURN(arg_node);
// }

// node *Mdowhileloop(node *arg_node, info *arg_info)
// {
//   DBUG_ENTER("Mdowhileloop");
//   // printf("Found a ...! %s", IDS_NAME(arg_node));
//   DBUG_RETURN(arg_node);
// }

// // IF STATEMENT
// node *Mif(node *arg_node, info *arg_info)
// {
//   DBUG_ENTER("Mif");
//   // printf("Found a ...! %s", IDS_NAME(arg_node));
//   DBUG_RETURN(arg_node);
// }

// RETURN STATEMENT
node *Mreturn(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mreturn");
  // printf("Found a ...! %s", NODE_TYPE(arg_node));
  DBUG_RETURN(arg_node);
}

// BOOLEAN
node *Mbool(node *arg_node, info *arg_info)
{
  DBUG_ENTER("Mbool");
  printf("Found a BOOL! %s", BOOL_VALUE(arg_node));
  DBUG_RETURN(arg_node);
}

// ARRAYS LOCALFUNCTION (?)!!


/*
 * Traversal start function
 */

node *MtravSymbolTable(node *syntaxtree)
{
  info *arg_info;

  DBUG_ENTER("MtravSymbolTable");

  TRAVpush(TR_m);
  syntaxtree = TRAVdo(syntaxtree, arg_info);
  TRAVpop();

  arg_info = FreeInfo( arg_info);

  CTInote("Traversing done...\n");

  DBUG_RETURN(syntaxtree);
}
