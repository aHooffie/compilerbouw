
/*****************************************************************************
 *
 * Module: make_regularexpr
 *
 * Prefix: REL
 *
 * Description:
 *
 * This module implements a traversal of the abstract syntax tree that 
 * turns local variable initialisations into regular assignments
 *
 *****************************************************************************/


#include "make_re_local.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"

#include "memory.h"
#include "ctinfo.h"

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

node *RELvardeclaration(node *arg_node, info *arg_info)
{
	DBUG_ENTER("RELvardeclaration");

    CTInote("Found vardecl!");

    if (VARDECLARATION_INIT(arg_node) != NULL)
    {
        // has an expression!

        // left hand side assign
        char *name = VARDECLARATION_NAME(arg_node);
        node *newAssign = TBmakeAssign(TBmakeVarlet(name, NULL, NULL), VARDECLARATION_INIT(arg_node));
        node *newStmt = TBmakeStmts(newAssign, NULL);

        if (INFO_STMTS(arg_info) == NULL)
        {
            INFO_HEAD(arg_info) = newStmt;
            INFO_STMTS(arg_info) = newStmt;
            CTInote("vardec: MADE NEW ASSIGN");
        }
        else
        {
            STMTS_NEXT(INFO_STMTS(arg_info)) = newStmt;
            INFO_STMTS(arg_info) = newStmt;
            CTInote("vardec: ADDED TO STMTS + UPDATED INFO STMTS");
        }

        // 'Remove' vardeclaration expression
        VARDECLARATION_INIT(arg_node) = NULL;
    }

	DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */

node *RELdoRegularExpr(node *syntaxtree)
{
    DBUG_ENTER("RELdoRegularExpr");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_rel);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    CTInote("Traversing done...\n");

    if (INFO_STMTS(arg_info) != NULL)
    {

        // prefix the original sequence of statements??

        // node *allStmts = TBmakeFunctionbody(NULL, NULL, INFO_HEAD(arg_info));
    //     node *init_func = TBmakeFunction(T_void, STRcpy("__init"), allStmts, NULL);

    //     // add to syntaxtree
    //     PROGRAM_DECLARATIONS(syntaxtree) = TBmakeDeclarations(init_func, PROGRAM_DECLARATIONS(syntaxtree));
    }

    DBUG_RETURN(syntaxtree);
}