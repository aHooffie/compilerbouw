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

// NOTE: CALLS OPZOEKEN FAALT NOG ( = TRAVERSEN! )

/* INFO structure */
struct INFO 
{
  node *stack;  
  int size; 
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
    DBUG_ENTER("ASprogram");

    printf("Found a Program. \n");

    /* Create a symbol table node for the global scoped variables and functions. */
    node *globals = TBmakeSymboltable(NULL, NULL);
    stackPush(globals, arg_info);
    PROGRAM_SYMBOLTABLE(arg_node) = globals;

    if (INFO_STACK(arg_info) == NULL) 
        CTInote("Something went wrong with the symboltable in the Program node. \n");
  
    /* Continue with traversing in child nodes. */
    PROGRAM_DECLARATIONS( arg_node) = TRAVdo( PROGRAM_DECLARATIONS( arg_node), arg_info);

    /* Remove the linked list at the end of the traversal. */
    stackPop(arg_info);

    DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASglobaldec");

    /* Found globaldec. Check if there already is one with the same name. */
    printf("Found a Globaldec: %s. \n", GLOBALDEC_NAME(arg_node));

    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), GLOBALDEC_NAME(arg_node)) == FALSE)
        CTInote("Globaldec %s is already declared in the symboltable.\n", GLOBALDEC_NAME(arg_node));
    else {
        /* Else, insert the globaldec into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(GLOBALDEC_NAME(arg_node), GLOBALDEC_TYPE(arg_node), NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        GLOBALDEC_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }
    /* Continue with traversing in child nodes. */
    if (GLOBALDEC_DIMENSIONS( arg_node) != NULL)
        GLOBALDEC_DIMENSIONS( arg_node) = TRAVdo( GLOBALDEC_DIMENSIONS( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASglobaldef");

    printf("Found a Globaldef: %s.\n", GLOBALDEF_NAME(arg_node));

    /* Found globaldef. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), GLOBALDEF_NAME(arg_node)) == FALSE)
        CTInote("Globaldef %s is already declared in the symboltable!\n", GLOBALDEF_NAME(arg_node));
    else {
        /* Else, insert the globaldef into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(GLOBALDEF_NAME(arg_node), GLOBALDEF_TYPE(arg_node), NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        GLOBALDEF_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }
        
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

    printf("Found a function: %s.\n", FUNCTION_NAME(arg_node));

    /* Found function. Check if there already is one with the same name in global ST. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), FUNCTION_NAME(arg_node)) == FALSE)
        CTInote("Function %s is already declared in the symboltable.\n", FUNCTION_NAME(arg_node));
    else {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(FUNCTION_NAME(arg_node), FUNCTION_TYPE(arg_node), NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        FUNCTION_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

    /* Create new symbol table and add it to the stack. */
    node *functionSymboltable = TBmakeSymboltable(NULL, INFO_STACK(arg_info));
    stackPush(functionSymboltable, arg_info);
    
    printf("Nieuwe symbol table voor function %s gemaakt: stack size: %i\n", FUNCTION_NAME(arg_node), INFO_SIZE(arg_info));
    
    if (INFO_STACK(arg_info) == NULL) 
        CTInote("Something went wrong with the symboltable in the FUNCTION node. \n");

    /* Update function administration. */
    FUNCTION_SYMBOLTABLE(arg_node) = functionSymboltable;
    
    /* Continue with traversing in child nodes. */
    if (FUNCTION_PARAMETERS( arg_node) != NULL)
        FUNCTION_PARAMETERS( arg_node) = TRAVdo( FUNCTION_PARAMETERS( arg_node), arg_info);
    if (FUNCTION_FUNCTIONBODY( arg_node) != NULL)
        FUNCTION_FUNCTIONBODY( arg_node) = TRAVdo( FUNCTION_FUNCTIONBODY( arg_node), arg_info);

    /* Remove the linked list at the end of the traversal. */
    stackPop(arg_info);

    DBUG_RETURN(arg_node);
}

node *ASparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASparameters");

    printf("Found Parameter %s.\n", PARAMETERS_NAME(arg_node));

    /* Found parameter. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), PARAMETERS_NAME(arg_node)) == FALSE)
        CTInote("Parameter %s is already declared in the symboltable!\n", PARAMETERS_NAME(arg_node));
    else {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(PARAMETERS_NAME(arg_node), PARAMETERS_TYPE(arg_node), NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;


        PARAMETERS_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }    

    /* Continue with traversing in child nodes. */
    if (PARAMETERS_DIMENSIONS( arg_node) != NULL)
        PARAMETERS_DIMENSIONS( arg_node) = TRAVdo( PARAMETERS_DIMENSIONS( arg_node), arg_info);

    if (PARAMETERS_NEXT( arg_node) != NULL)
        PARAMETERS_NEXT( arg_node) = TRAVdo( PARAMETERS_NEXT( arg_node), arg_info);
    
    DBUG_RETURN(arg_node);
}

node *ASvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvardeclaration");

    printf("Found Vardeclaration %s. \n", VARDECLARATION_NAME(arg_node));
    
    /* Found vardeclaration. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), VARDECLARATION_NAME(arg_node)) == FALSE)
        printf("Vardeclaration %s is already declared in the symboltable!\n", VARDECLARATION_NAME(arg_node));
    else {
        /* Else, insert the globaldef into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(VARDECLARATION_NAME(arg_node), VARDECLARATION_TYPE(arg_node), NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        VARDECLARATION_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

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

    printf("Found Functioncallstmt %s. \n", FUNCTIONCALLSTMT_NAME(arg_node));

    /* Find the original function declaration in the scope above. */ 
    node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    node *original;

    while (symboltable != NULL) {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), FUNCTIONCALLSTMT_NAME(arg_node));
        
        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else {
            FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node) = original;
            break;
        }
    }
    
    if (FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node) == NULL)
        CTInote("!! Error. Functioncallstmt %s is nog niet gedeclareerd.\n", FUNCTIONCALLSTMT_NAME(arg_node));

    /* Continue with traversing in child nodes. */
    if (FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) != NULL)
        FUNCTIONCALLSTMT_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("FUNCTIONCALLEXPR");

    printf("Found Functioncallexpr %s. \n", FUNCTIONCALLEXPR_NAME(arg_node));
    
    /* Find the original function declaration in the scope above. */ 
    node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    node *original;

    while (symboltable != NULL) {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), FUNCTIONCALLEXPR_NAME(arg_node));
        
        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else {
            FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node) = original;
            break;
        }
    }
    
    if (FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node) == NULL)
        CTInote("!! Error. Functioncallexpr %s is nog niet gedeclareerd.\n", FUNCTIONCALLEXPR_NAME(arg_node));

    /* Continue with traversing in child nodes. */
    if (FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) != NULL)
        FUNCTIONCALLEXPR_EXPRESSIONS( arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Found nodes that should read from a symbol table entry  */
node *ASvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvar");

    printf("Found Var %s.\n", VAR_NAME(arg_node));
    
    /* Find the original function declaration in the scope above. */ 
    node *symboltable = INFO_STACK(arg_info);
    node *original;

    while (symboltable != NULL) {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), VAR_NAME(arg_node));
        
        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else {
            VAR_SYMBOLTABLEENTRY(arg_node) = original;
            break;
        }
    }
    
    if (VAR_SYMBOLTABLEENTRY(arg_node) == NULL)
        CTInote("!! Error. Var %s is nergens gedeclareerd.\n", VAR_NAME(arg_node));
    
    /* Continue with traversing in child nodes. */
    if (VAR_INDICES( arg_node) != NULL)
        VAR_INDICES( arg_node) = TRAVdo(VAR_INDICES( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvarlet");

    printf("Found Varlet %s.\n", VARLET_NAME(arg_node));
    
    /* Find the original function declaration in the scope above. */ 
    node *symboltable = INFO_STACK(arg_info);
    node *original;

    while (symboltable != NULL) {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), VARLET_NAME(arg_node));
        
        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else {
            VARLET_SYMBOLTABLEENTRY(arg_node) = original;
            break;
        }
    }
    
    if (VARLET_SYMBOLTABLEENTRY(arg_node) == NULL)
        CTInote("!! Error. Varlet %s is nergens gedeclareerd.\n", VARLET_NAME(arg_node));
    
    /* Continue with traversing in child nodes. */
    if (VARLET_INDICES( arg_node) != NULL)
        VARLET_INDICES( arg_node) = TRAVdo(VARLET_INDICES( arg_node), arg_info);
    if (VARLET_NEXT( arg_node) != NULL)
        VARLET_NEXT( arg_node) = TRAVdo(VARLET_NEXT( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASids(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASids");

    printf("Found Id %s! \n", IDS_NAME(arg_node));

    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), IDS_NAME(arg_node)) == FALSE)
        CTInote("ID %s is already declared in the symboltable.\n", IDS_NAME(arg_node));
    else {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(IDS_NAME(arg_node), T_int, NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;
    
        IDS_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

    if (IDS_NEXT(arg_node) != NULL) 
        IDS_NEXT( arg_node) = TRAVdo(IDS_NEXT( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Stack functions. */
void stackPush(node *symboltable, info *arg_info) {
    INFO_STACK(arg_info) = symboltable;
    INFO_SIZE(arg_info) += 1;
    return;
}

void stackPop(info *arg_info) {
    INFO_STACK(arg_info) = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    INFO_SIZE(arg_info) -= 1;
    return;
}

bool stackEmpty(info *arg_info) {
    if (INFO_STACK(arg_info) == NULL && INFO_SIZE(arg_info) == 0)
        return TRUE;
    else 
        return FALSE;
}


/* Returns a string of the type. Unnecessary now, might be useful later? */
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
/* Functions to traverse to symbol table entry linked lists. */
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

void printSymboltable(node *symboltableentry) {
    node *trav = symboltableentry;

    while (trav != NULL) {
        printf("%s \n", SYMBOLTABLEENTRY_NAME(trav));  
        trav = SYMBOLTABLEENTRY_NEXT(trav);
    }
}
/* Traversal start function */
node *ASdoAddSymbolTables(node *syntaxtree)
{
  DBUG_ENTER("ASdoAddSymboltables");

  info *arg_info;
  arg_info = MakeInfo();

  TRAVpush( TR_as);
  syntaxtree = TRAVdo(syntaxtree, arg_info);
  TRAVpop();

  CTInote("Traversing done...\n");

  arg_info = FreeInfo(arg_info);

  DBUG_RETURN(syntaxtree);
}
