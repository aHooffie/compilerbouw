
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


/*
 * Traversal functions
 */

node *RELfunctionbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("RELfunctionbody");

    // traverse through vardecls
    if (FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL)
        FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);

    if (INFO_HEAD(arg_info) != NULL)
    {
        if (FUNCTIONBODY_STMTS(arg_node) == NULL)
        {
            FUNCTIONBODY_STMTS(arg_node) = INFO_HEAD(arg_info);

        }
        else
        {
            // add before old stmts
            STMTS_NEXT(INFO_STMTS(arg_info)) = FUNCTIONBODY_STMTS(arg_node);
            FUNCTIONBODY_STMTS(arg_node) = INFO_HEAD(arg_info);
        }

        // reset head node
        INFO_HEAD(arg_info) = NULL;
    }

    if (FUNCTIONBODY_LOCALFUNCTION(arg_node) != NULL)
        FUNCTIONBODY_LOCALFUNCTION(arg_node) = TRAVdo(FUNCTIONBODY_LOCALFUNCTION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RELvardeclaration(node *arg_node, info *arg_info)
{
	DBUG_ENTER("RELvardeclaration");


    if (VARDECLARATION_INIT(arg_node) != NULL)
    {
        // make regular expression (stmt)
        char *name = STRcpy(VARDECLARATION_NAME(arg_node));
        node *varlet = TBmakeVarlet(name, NULL, NULL);
        VARLET_SYMBOLTABLEENTRY(varlet) = VARDECLARATION_SYMBOLTABLEENTRY(arg_node);
        node *newAssign = TBmakeAssign(varlet, VARDECLARATION_INIT(arg_node));
        node *newStmt = TBmakeStmts(newAssign, NULL);

        // if first vardecl
        if (INFO_HEAD(arg_info) == NULL)
        {
            INFO_HEAD(arg_info) = newStmt;
            INFO_STMTS(arg_info) = newStmt;
        }
        else
        {
            STMTS_NEXT(INFO_STMTS(arg_info)) = newStmt;
            INFO_STMTS(arg_info) = newStmt;
        }

        // 'Remove' vardeclaration expression
        VARDECLARATION_INIT(arg_node) = NULL;
    }

    if (VARDECLARATION_NEXT(arg_node) != NULL)
        VARDECLARATION_NEXT(arg_node) = TRAVdo(VARDECLARATION_NEXT(arg_node), arg_info);

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

    DBUG_RETURN(syntaxtree);
}