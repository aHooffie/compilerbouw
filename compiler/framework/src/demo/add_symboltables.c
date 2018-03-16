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
  node *stack;  
  int size; // current scope level
  // = stack van LUTS (scopes): push, pop, isempty();
};

/* struct macros */
#define INFO_STACK(n)       ((n)->stack)
#define INFO_SIZE(n)       ((n)->size)


/* INFO functions */
static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));
  INFO_STACK( result) = NULL;
  INFO_SIZE( result) = 0;

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
  printf("Found program node. \n");
  node *globals = TBmakeSymboltable(NULL, NULL);
  INFO_STACK(arg_info) = globals;
  INFO_SIZE(arg_info) += 1;
  printf("nieuwe st in program: stack size: %i\n", INFO_SIZE(arg_info));

  if (INFO_STACK(arg_info) == NULL) 
    CTInote("Something went wrong with the symboltable in the PROGRAM node. \n");
  
  PROGRAM_SYMBOLTABLE(arg_node) = globals;
  
  /* Continue with traversing in child nodes. */
  PROGRAM_DECLARATIONS( arg_node) = TRAVdo( PROGRAM_DECLARATIONS( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldec");

  /* Found globaldec. Check if there already is one with the same name. */
  printf("Found a globaldec node: %s. \n", GLOBALDEC_NAME(arg_node));

  if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), GLOBALDEC_NAME(arg_node)) == FALSE)
  {
    printf("globaldec %s is already declared in the symboltable!\n", GLOBALDEC_NAME(arg_node));
    return NULL;
  }

  /* Else, insert the globaldec into the symbol table linked list at the end. */
  node *newEntry = TBmakeSymboltableentry(GLOBALDEC_NAME(arg_node), GLOBALDEC_TYPE(arg_node), NULL);
  node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
  if (last == NULL) {
    SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
  } else {
    SYMBOLTABLEENTRY_NEXT(last) = newEntry;
  }
  GLOBALDEC_SYMBOLTABLEENTRY(arg_node) = newEntry;

  /* Continue with traversing in child nodes. */
  if (GLOBALDEC_DIMENSIONS( arg_node) != NULL)
    GLOBALDEC_DIMENSIONS( arg_node) = TRAVdo( GLOBALDEC_DIMENSIONS( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASglobaldef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASglobaldef");

  /* Found globaldef. Check if there already is one with the same name. */
  printf("Found GLOBALDEF %s. Trying to write to global ST.\n", GLOBALDEF_NAME(arg_node));
  
  if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), GLOBALDEF_NAME(arg_node)) == FALSE)
  {
    printf("globaldef %s is already declared in the symboltable!\n", GLOBALDEF_NAME(arg_node));
    return NULL;
  }

  /* Else, insert the globaldef into the symbol table linked list at the end. */
  node *newEntry = TBmakeSymboltableentry(GLOBALDEF_NAME(arg_node), GLOBALDEF_TYPE(arg_node), NULL);
  node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
  if (last == NULL) {
    SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
  } else {
    SYMBOLTABLEENTRY_NEXT(last) = newEntry;
  }

  GLOBALDEF_SYMBOLTABLEENTRY(arg_node) = newEntry;
  
  /* Continue with traversing in child nodes. */
  if (GLOBALDEF_DIMENSIONS( arg_node) != NULL)
      GLOBALDEF_DIMENSIONS( arg_node) = TRAVdo( GLOBALDEF_DIMENSIONS( arg_node), arg_info);
  if (GLOBALDEF_ASSIGN( arg_node) != NULL)
    GLOBALDEF_ASSIGN( arg_node) = TRAVdo( GLOBALDEF_ASSIGN( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/* Found nodes that should write or read to a function symbol table entry  */
node *ASfunction(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunction");

  /* Found function. Check if there already is one with the same name in global ST. */
  printf("Found function %s.\n", FUNCTION_NAME(arg_node));

  if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), FUNCTION_NAME(arg_node)) == FALSE)
  {
    printf("function %s is already declared in the symboltable!\n", FUNCTION_NAME(arg_node));
    return NULL;
  }

  /* Else, insert the function into the symbol table linked list at the end. */
  node *newEntry = TBmakeSymboltableentry(FUNCTION_NAME(arg_node), FUNCTION_TYPE(arg_node), NULL);
  node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
  if (last == NULL) {
    SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
  } else {
    SYMBOLTABLEENTRY_NEXT(last) = newEntry;
  }

  /* Create new symbol table and add it to the stack. */
  node *functionSymboltable = TBmakeSymboltable(NULL, INFO_STACK(arg_info));
  INFO_STACK(arg_info) = functionSymboltable;
  INFO_SIZE(arg_info) += 1;
  printf("nieuwe st in function gemaakt: stack size: %i\n", INFO_SIZE(arg_info));
  
  if (INFO_STACK(arg_info) == NULL) 
    CTInote("Something went wrong with the symboltable in the FUNCTION node. \n");

  /* Update function administration. */
  FUNCTION_SYMBOLTABLEENTRY(arg_node) = newEntry;
  FUNCTION_SYMBOLTABLE(arg_node) = functionSymboltable;
  
  /* Continue with traversing in child nodes. */
  if (FUNCTION_PARAMETERS( arg_node) != NULL)
    FUNCTION_PARAMETERS( arg_node) = TRAVdo( FUNCTION_PARAMETERS( arg_node), arg_info);
  if (FUNCTION_FUNCTIONBODY( arg_node) != NULL)
    FUNCTION_FUNCTIONBODY( arg_node) = TRAVdo( FUNCTION_FUNCTIONBODY( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASparameters(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASparameters");

  /* Found parameter. Check if there already is one with the same name. */
  printf("Found parameter %s.\n", PARAMETERS_NAME(arg_node));

  if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), PARAMETERS_NAME(arg_node)) == FALSE)
  {
    printf("parameter %s is already declared in the symboltable!\n", PARAMETERS_NAME(arg_node));
    return NULL;
  }

  /* Else, insert the function into the symbol table linked list at the end. */
  node *newEntry = TBmakeSymboltableentry(PARAMETERS_NAME(arg_node), PARAMETERS_TYPE(arg_node), NULL);
  node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
  if (last == NULL) {
    SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
  } else {
    SYMBOLTABLEENTRY_NEXT(last) = newEntry;
  }

  PARAMETERS_SYMBOLTABLEENTRY(arg_node) = newEntry;
  
  /* Continue with traversing in child nodes. */
  if (PARAMETERS_NEXT( arg_node) != NULL)
    PARAMETERS_NEXT( arg_node) = TRAVdo( PARAMETERS_NEXT( arg_node), arg_info);
  if (PARAMETERS_DIMENSIONS( arg_node) != NULL)
    PARAMETERS_DIMENSIONS( arg_node) = TRAVdo( PARAMETERS_DIMENSIONS( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASvardeclaration(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvardeclaration");

  /* Found vardeclaration. Check if there already is one with the same name. */
  printf("Found VARDECLARATION %s. Write to function ST.\n", VARDECLARATION_NAME(arg_node));
  
  if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), VARDECLARATION_NAME(arg_node)) == FALSE)
  {
    printf("vardeclaration %s is already declared in the symboltable!\n", VARDECLARATION_NAME(arg_node));
    return NULL;
  }

  /* Else, insert the globaldef into the symbol table linked list at the end. */
  node *newEntry = TBmakeSymboltableentry(VARDECLARATION_NAME(arg_node), VARDECLARATION_TYPE(arg_node), NULL);
  node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
  if (last == NULL) {
    SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
  } else {
    SYMBOLTABLEENTRY_NEXT(last) = newEntry;
  }

  VARDECLARATION_SYMBOLTABLEENTRY(arg_node) = newEntry;

  /* Continue with traversing in child nodes. */
  if (VARDECLARATION_DIMENSIONS( arg_node) != NULL)
    VARDECLARATION_DIMENSIONS( arg_node) = TRAVdo( VARDECLARATION_DIMENSIONS( arg_node), NULL);
  if (VARDECLARATION_INIT( arg_node) != NULL)
    VARDECLARATION_INIT( arg_node) = TRAVdo( VARDECLARATION_INIT( arg_node), arg_info);
  if (VARDECLARATION_NEXT( arg_node) != NULL)
    VARDECLARATION_NEXT( arg_node) = TRAVdo( VARDECLARATION_NEXT( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASfunctioncallstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASfunctioncallstmt");

  /* Found functioncallstmt. Check if there already is one with the same name. */
  printf("Found FUNCTIONCALLSTMT %s. \n", FUNCTIONCALLSTMT_NAME(arg_node));

  /* Find the original function declaration in the scope above. */ 
  node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
  node *original;

  while (symboltable != NULL) {
    original = findOriginal(SYMBOLTABLE_NEXT(symboltable), FUNCTIONCALLSTMT_NAME(arg_node));
    
    if (original == NULL) {
      symboltable = SYMBOLTABLE_PREV(symboltable);
    } else {
      // NODE FOUND!
      FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node) = original;
    }
  }
  
  if (FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node) == NULL)
    printf("ERROR - FUNCTIONCALLSTMT BESTAAT NIET!");

  /* Continue with traversing in child nodes. */
  if (FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) != NULL)
    FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASfunctioncallexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("FUNCTIONCALLEXPR");

  /* Found functioncallexpr. Check if there already is one with the same name. */
  printf("Found FUNCTIONCALLEXPR %s. \n", FUNCTIONCALLEXPR_NAME(arg_node));
   
   /* Find the original function declaration in the scope above. */ 
  node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
  node *original;

  while (symboltable != NULL) {
    original = findOriginal(SYMBOLTABLE_NEXT(symboltable), FUNCTIONCALLEXPR_NAME(arg_node));
    
    if (original == NULL) {
      symboltable = SYMBOLTABLE_PREV(symboltable);
    } else {
      // NODE FOUND!
      FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node) = original;
    }
  }
  
  if (FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node) == NULL)
    printf("ERROR - FUNCTIONCALLEXPR BESTAAT NIET!");

  /* Continue with traversing in child nodes. */
  if (FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) != NULL)
    FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

// VARIABLES INSIDE FUNCTIONS
node *ASvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvar");

  /* Found var. Check if there already is one with the same name. */
  printf("Found VAR %s.\n", VAR_NAME(arg_node));
  
  /* Find the original function declaration in the scope above. */ 
  node *symboltable = INFO_STACK(arg_info);
  node *original;

  while (symboltable != NULL) {
    original = findOriginal(SYMBOLTABLE_NEXT(symboltable), VAR_NAME(arg_node));
    
    if (original == NULL) {
      symboltable = SYMBOLTABLE_PREV(symboltable);
    } else {
      // NODE FOUND!
      VAR_SYMBOLTABLEENTRY(arg_node) = original;
    }
  }
  
  if (VAR_SYMBOLTABLEENTRY(arg_node) == NULL)
    printf("ERROR - VAR BESTAAT NIET!");
  
  /* Continue with traversing in child nodes. */
  if (VAR_INDICES( arg_node) != NULL)
      VAR_INDICES( arg_node) = TRAVdo(VAR_INDICES( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *ASvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASvarlet");

  /* Found varlet. Check if there already is one with the same name. */
  printf("Found VARLET %s.\n", VARLET_NAME(arg_node));
  
  /* Find the original function declaration in the scope above. */ 
  node *symboltable = INFO_STACK(arg_info);
  node *original;

  while (symboltable != NULL) {
    original = findOriginal(SYMBOLTABLE_NEXT(symboltable), VARLET_NAME(arg_node));
    
    if (original == NULL) {
      symboltable = SYMBOLTABLE_PREV(symboltable);
    } else {
      // NODE FOUND!
      VARLET_SYMBOLTABLEENTRY(arg_node) = original;
    }
  }
  
  if (VARLET_SYMBOLTABLEENTRY(arg_node) == NULL)
    printf("ERROR - VARLET BESTAAT NIET!");
  
  /* Continue with traversing in child nodes. */
  if (VARLET_INDICES( arg_node) != NULL)
      VARLET_INDICES( arg_node) = TRAVdo(VARLET_INDICES( arg_node), arg_info);
  if (VARLET_NEXT( arg_node) != NULL)
      VARLET_NEXT( arg_node) = TRAVdo(VARLET_NEXT( arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

// IDS & ARRAYEXPR??
node *ASids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("ASids");
  printf("Found ID %s! \n", IDS_NAME(arg_node));

  if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), IDS_NAME(arg_node)) == FALSE)
  {
    printf("parameter %s is already declared in the symboltable!\n", IDS_NAME(arg_node));
    return NULL;
  }

  /* Else, insert the function into the symbol table linked list at the end. */
  node *newEntry = TBmakeSymboltableentry(IDS_NAME(arg_node), T_int, NULL);
  node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
  if (last == NULL) {
    SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
  } else {
    SYMBOLTABLEENTRY_NEXT(last) = newEntry;
  }

  IDS_SYMBOLTABLEENTRY(arg_node) = newEntry;

  if (IDS_NEXT(arg_node) != NULL) 
      IDS_NEXT( arg_node) = TRAVdo(IDS_NEXT( arg_node), arg_info);


  DBUG_RETURN(arg_node);
}

/* Returns a string of the type to put in the LUTS. */
char *TypetoString(type Type) 
{
  char *typeString;
  switch (Type)
    {
    case T_int:
        typeString = "int";
        break;
    case T_float:
        typeString = "float";
        break;
    case T_bool:
        typeString = "bool";
        break;
    case T_void:
        typeString = "void";
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown type detected!");
        break;
    }

    return typeString;
}

node *travList(node *symboltableentry) {
  node *trav = symboltableentry;
  if (trav == NULL) {
    return NULL;
  }

  while (SYMBOLTABLEENTRY_NEXT(trav) != NULL) {
    trav = SYMBOLTABLEENTRY_NEXT(trav);
  }

  return trav;
}

node *findOriginal(node *symboltableentry, char *name) {
  node *trav = symboltableentry;

  while (trav != NULL) {
    if (SYMBOLTABLEENTRY_NAME(trav) == name) {
      return trav;
    } else {
      trav = SYMBOLTABLEENTRY_NEXT(trav);
    }
  }

  return trav;
}

bool checkDuplicates(node *symboltableentry, char *name) {
  node *trav = symboltableentry;

  while (trav != NULL) {
    if (SYMBOLTABLEENTRY_NAME(trav) == name) {
      return FALSE;
    } else {
      trav = SYMBOLTABLEENTRY_NEXT(trav);
    }
  }

  return TRUE;
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
