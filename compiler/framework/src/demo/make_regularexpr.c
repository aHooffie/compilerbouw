
/*****************************************************************************
 *
 * Module: make_regularexpr
 *
 * Prefix: RE
 *
 * Description:
 *
 * This module implements a traversal of the abstract syntax tree that 
 * turns variable initialisations into regular assignments
 *
 *****************************************************************************/


#include "make_regularexpr.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"

#include "memory.h"
#include "ctinfo.h"



// een variable declaration with initialisation:
// int a = a + 1;
// bedoeling: dit mag in civic als a al eerder declared is; dan neemt ie die waarde over:

// More precisely, the initialisation expression of a variable declaration
// is not in the scope of the declared variable. Technically, a compiler would
// always first traverse the initialisation expression in the existing scope
// before creating a new variable a and updating the current scope accordingly.


/*
 * INFO structure
 */

struct INFO
{
    node *head;
    node *stmts;
};


/* struct macros */
#define INFO_HEAD(n) ((n)->head)
#define INFO_STMTS(n) ((n)->stmts)


/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_HEAD(result) = NULL;
    INFO_STMTS(result) = NULL;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}


/*
 * Traversal functions
 */

node *REglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("REglobaldec");

    CTInote("Found globaldecl!");

    // if (INFO_STMTS(arg_info) == NULL)
    // {
        // create stmts and add stmt

        // node *stmt = arg_node;
        // INFO_STMTS(arg_info) = TBmakeStmts(stmt, NULL);

    // }
    // else
    // {   
    //     // add stmt to existing stmts (next)
    //     node *newStmt = arg_node;
    //     STMTS_NEXT(INFO_STMTS(arg_info)) = TBmakeStmts(newStmt, NULL);
    //     INFO_STMTS(arg_info) = STMTS_NEXT(INFO_STMTS(arg_info));

    // }

    /* continue traversing child nodes */
    if (GLOBALDEC_DIMENSIONS(arg_node) != NULL)
    {
        GLOBALDEC_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEC_DIMENSIONS(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *REglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("REglobaldef");

    CTInote("Found globaldef!");

    // left hand side assign
    char *name = GLOBALDEF_NAME(arg_node);
    node *newAssign = TBmakeAssign(TBmakeVarlet(name, NULL, NULL), GLOBALDEF_ASSIGN(arg_node));
    node *newStmt = TBmakeStmts(newAssign, NULL);

    if (INFO_STMTS(arg_info) == NULL)
    {
        INFO_HEAD(arg_info) = newStmt;
        INFO_STMTS(arg_info) = newStmt;
        CTInote("MADE NEW ASSIGN");
    }
    else
    {
        STMTS_NEXT(INFO_STMTS(arg_info)) = newStmt;
        INFO_STMTS(arg_info) = newStmt;
        CTInote("ADDED TO STMTS + UPDATED INFO STMTS");
    }
    
    // Remove' globaldef expression 
    GLOBALDEF_ASSIGN(arg_node) = NULL;



    // /* continue traversing child nodes */
    // if (GLOBALDEF_DIMENSIONS(arg_node) != NULL)
    // {
    //     GLOBALDEF_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEF_DIMENSIONS(arg_node), arg_info);
    // }
    // if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    // {
    //     GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);
    // }

    DBUG_RETURN(arg_node);
}

node *REvardeclaration(node *arg_node, info *arg_info)
{
	DBUG_ENTER("REvardeclaration");

    CTInote("Found vardecl!");

	// node *entry = VARDECLARATION_SYMBOLTABLEENTRY(arg_node);

	DBUG_RETURN(arg_node);
}


/*
 * Traversal start function
 */

node *REdoRegularExpressions(node *syntaxtree)
{
    DBUG_ENTER("REmakeRegularExpressions");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_re);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    CTInote("Traversing done...\n");

    if (INFO_STMTS(arg_info) != NULL)
    {
        node *allStmts = TBmakeFunctionbody(NULL, NULL, INFO_HEAD(arg_info));

        node *init_func = TBmakeFunction(T_void, "__init", allStmts, NULL);

        // add to tree
        PROGRAM_DECLARATIONS(syntaxtree) = TBmakeDeclarations(init_func, PROGRAM_DECLARATIONS(syntaxtree));

        CTInote("STMTS NOT NULL! %s function exists.", FUNCTION_NAME(init_func));

    }


    // freed niet alles!
    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}