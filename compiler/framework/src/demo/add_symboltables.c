/*
 * Module: Traversing and looking for symbol table links
 * Prefix: AS
 */

#include "add_symboltables.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "lookup_table.h"

#include "memory.h"
#include "ctinfo.h"

/* INFO structure */
struct INFO 
{
  lut_t *symboltable;
  info *prev;
};


/* INFO macros */
#define INFO_SYMBOLTABLE(n)  ((n)->symboltable)
#define INFO_PREV(n)         ((n)->prev)

/* INFO functions */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  INFO_SYMBOLTABLE( result) = NULL;
  INFO_PREV( result) = NULL;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}

/* Found nodes that should write to a global symbol table entry  */
node *ASprogram(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASprog");

  /* Create a lut for the global scoped variables and functions. */
  printf("Found Program node. This should create a ST!\n");
  lut_t *global_lut = LUTgenerateLut();
  INFO_SYMBOLTABLE(arg_info) = global_lut;
  // PROGRAM_DECLARATION(arg_node) = INFO_SYMBOLTABLE(arg_info);
  
  /* Continue with children.*/
  PROGRAM_DECLARATIONS( arg_node) = TRAVdo( PROGRAM_DECLARATIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldec");

  /* Found globaldec. Check if there already is one with the same name. */
  printf("Found GLOBALDEC %s. Write to global ST.\n", GLOBALDEC_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_SYMBOLTABLE(arg_info), GLOBALDEC_NAME(arg_node));
  // if (data != NULL)
  //   printf("------- GLOBALDEC already exists. ABORT! \n\n");

  /* Else, insert the globaldec into the global lut. */
  // INFO_SYMBOLTABLE(arg_info) = LUTinsertIntoLutS(
    // INFO_SYMBOLTABLE(arg_info), GLOBALDEC_NAME(arg_node), GLOBALDEC_TYPE(arg_node));
  // GLOBALDEC_DECLARATION(arg_node) = INFO_SYMBOLTABLE(arg_info);

  /* Continue with children. */
  if (GLOBALDEC_DIMENSIONS( arg_node) != NULL)
    GLOBALDEC_DIMENSIONS( arg_node) = TRAVdo( GLOBALDEC_DIMENSIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASglobaldef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldef");

  /* Found globaldef. Check if there already is one with the same name. */
  printf("Found GLOBALDEF %s. Trying to write to global ST.\n", GLOBALDEF_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_SYMBOLTABLE(arg_info), GLOBALDEF_NAME(arg_node));
  // if (data != NULL)
  //   printf("------- GLOBALDEF already exists. ABORT! \n\n");

  /* Else, insert the globaldef into the global lut. */
  // INFO_SYMBOLTABLE(arg_info) = LUTinsertIntoLutS(
    // INFO_SYMBOLTABLE(arg_info), GLOBALDEF_NAME(arg_node), GLOBALDEF_TYPE(arg_node));
  // GLOBALDEF_DECLARATION(arg_node) = INFO_SYMBOLTABLE(arg_info);

  /* Continue with children. */
  if (GLOBALDEF_DIMENSIONS( arg_node) != NULL)
      GLOBALDEF_DIMENSIONS( arg_node) = TRAVdo( GLOBALDEF_DIMENSIONS( arg_node), NULL);
  if (GLOBALDEF_ASSIGN( arg_node) != NULL)
    GLOBALDEF_ASSIGN( arg_node) = TRAVdo( GLOBALDEF_ASSIGN( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

/* Found nodes that should write or read to a function symbol table entry  */
node *ASfunction(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");

  /* Found function. Check if there already is one with the same name. */
  printf("Found FUNCTION %s. Write to global ST & Create own ST.\n", FUNCTION_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_SYMBOLTABLE(arg_info), FUNCTION_NAME(arg_node));
  // if (data != NULL)
    // printf("------- FUNCTION already exists. ABORT! \n\n");

  /* Else, insert the function into the global lut
    + Create own lut. */
  // INFO_SYMBOLTABLE(arg_info) = LUTinsertIntoLutS(
    // INFO_SYMBOLTABLE(arg_info), FUNCTION_NAME(arg_node), FUNCTION_TYPE(arg_node));
  // FUNCTION_DECLARATION(arg_node) = INFO_SYMBOLTABLE(arg_info);
  
  // info *new = MakeInfo();
  // lut_t *function_lut = LUTgenerateLut();
  // INFO_SYMBOLTABLE(new) = function_lut;
  // INFO_PREV(new) = INFO_SYMBOLTABLE(arg_info);

  /* Continue with children. */
  if (FUNCTION_PARAMETERS( arg_node) != NULL)
    FUNCTION_PARAMETERS( arg_node) = TRAVdo( FUNCTION_PARAMETERS( arg_node), NULL);
  if (FUNCTION_FUNCTIONBODY( arg_node) != NULL)
    FUNCTION_FUNCTIONBODY( arg_node) = TRAVdo( FUNCTION_FUNCTIONBODY( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASparameters(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASparameters");

  /* Found parameter. Check if there already is one with the same name. */
  printf("Found PARAMETER %s. Write to function ST.\n", PARAMETERS_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_FUNCTION(arg_info), PARAMETERS_NAME(arg_node));
  // if (data != NULL)
    // printf("------- PARAMETER already exists. ABORT! \n\n");

  /* Else, insert the parameter into the function lut. */
  // INFO_FUNCTION(arg_info) = LUTinsertIntoLutS(
    //  INFO_FUNCTION(arg_info), PARAMETERS_NAME(arg_node), PARAMETERS_TYPE(arg_node));
  // PARAMETERS_DECLARATION(arg_node) = INFO_FUNCTION(arg_info);


  /* Continue with children. */
  if (PARAMETERS_NEXT( arg_node) != NULL)
    PARAMETERS_NEXT( arg_node) = TRAVdo( PARAMETERS_NEXT( arg_node), NULL);
  if (PARAMETERS_DIMENSIONS( arg_node) != NULL)
    PARAMETERS_DIMENSIONS( arg_node) = TRAVdo( PARAMETERS_DIMENSIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASvardeclaration(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvardeclaration");

  /* Found vardeclaration. Check if there already is one with the same name. */
  printf("Found VARDECLARATION %s. Write to function ST.\n", VARDECLARATION_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_FUNCTION(arg_info), VARDECLARATION_NAME(arg_node));
  // if (data != NULL)
    // printf("------- VARDECLARATION already exists. ABORT! \n\n");

  /* Else, insert the vardeclarations into the function lut. */
  // INFO_FUNCTION(arg_info) = LUTinsertIntoLutS(
  //    INFO_FUNCTION(arg_info), VARDECLARATION_NAME(arg_node), VARDECLARATION_TYPE(arg_node));
  // VARDECLARATION_DECLARATION(arg_node) = INFO_FUNCTION(arg_info);

  /* Continue with children. */
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

  /* Found functioncallstmt. Check if there already is one with the same name. */
  printf("Found FUNCTIONCALLSTMT %s. Read in function \n", FUNCTIONCALLSTMT_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_FUNCTION(arg_info), FUNCTIONCALLSTMT_NAME(arg_node));
  // if (data != NULL)
    // printf("FUNCTIONCALLSTMT already exists. THIS IS OK!! \n\n");

  // FUNCTIONCALLSTMT_SYMBOLTABLE(arg_node) = INFO_FUNCTION(arg_info);
  
  /* Continue with children. */
  if (FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) != NULL)
    FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASfunctioncallexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FUNCTIONCALLEXPR");

  /* Found functioncallexpr. Check if there already is one with the same name. */
  printf("Found FUNCTIONCALLEXPR %s. Read in function \n", FUNCTIONCALLEXPR_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_FUNCTION(arg_info), FUNCTIONCALLEXPR_NAME(arg_node));
  // if (data != NULL)
    // printf("FUNCTIONCALLEXPR already exists. THIS IS OK!!  \n\n");

  /* Else, insert the functioncallexpr into the function lut. */
  // FUNCTIONCALLEXPR_SYMBOLTABLE(arg_node) = INFO_FUNCTION(arg_info);
  
  /* Continue with children. */
  if (FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) != NULL)
    FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

// VARIABLES INSIDE FUNCTIONS
node *ASvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvar");

  /* Found var. Check if there already is one with the same name. */
  printf("Found VAR %s.\n", VAR_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_FUNCTION(arg_info), VAR_NAME(arg_node));
  // if (data != NULL)
    // printf("------- VAR %s already exists. ABORT! \n\n", VAR_NAME(arg_node));

  /* Else, insert the var into the function lut. */
  // INFO_FUNCTION(arg_info) = LUTinsertIntoLutS(
    //  INFO_FUNCTION(arg_info), VAR_NAME(arg_node), NULL);   // NOTE: WHAT TYPE IS A VAR?
  // VAR_DECLARATION(arg_node) = INFO_FUNCTION(arg_info);
  
  /* Continue with children. */
  if (VAR_INDICES( arg_node) != NULL)
      VAR_INDICES( arg_node) = TRAVdo(VAR_INDICES( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

node *ASvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvarlet");

  /* Found varlet. Check if there already is one with the same name. */
  printf("Found VARLET %s.\n", VARLET_NAME(arg_node));
  // void **data = LUTsearchInLutS(INFO_FUNCTION(arg_info), VARLET_NAME(arg_node));
  // if (data != NULL)
    // printf("------- VAR %s already exists. ABORT! \n\n", VARLET_NAME(arg_node));

  /* Else, insert the varlet into the function lut. */
  // INFO_FUNCTION(arg_info) = LUTinsertIntoLutS(
    //  INFO_FUNCTION(arg_info), VARLET_NAME(arg_node), NULL);   // NOTE: WHAT TYPE IS A VAR?
  // VARLET_DECLARATION(arg_node) = INFO_FUNCTION(arg_info);

  /* Continue with children. */
  if (VARLET_INDICES( arg_node) != NULL)
      VARLET_INDICES( arg_node) = TRAVdo(VARLET_INDICES( arg_node), NULL);
  if (VARLET_NEXT( arg_node) != NULL)
      VARLET_NEXT( arg_node) = TRAVdo(VARLET_NEXT( arg_node), NULL);

  DBUG_RETURN(arg_node);
}

// IDS & ARRAYEXPR??
node *ASids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASids");
  printf("Found ID %s! DOES THIS REQUIRE ST?\n", IDS_NAME(arg_node));
  DBUG_RETURN(arg_node);
}

/* Traversal start function */
node *ASdoAddSymbolTables(node *syntaxtree)
{
  DBUG_ENTER("ASdoAddSYMBOLTABLE");

  info *arg_info;
  arg_info = MakeInfo();

  TRAVpush( TR_as);
  syntaxtree = TRAVdo(syntaxtree, arg_info);
  TRAVpop();

  CTInote("Traversing done...\n");

  arg_info = FreeInfo(arg_info);

  DBUG_RETURN(syntaxtree);
}
