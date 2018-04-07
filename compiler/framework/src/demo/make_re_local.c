
/*
 * Module: make_re_local.c
 * Prefix: REL
 * Description: This module implements a traversal of the abstract syntax tree that 
 * turns local variable initialisations into regular assignments
 * Author: Aynel Gul
 * Arrays not implemented. 
 */

#include "make_re_local.h"

#include "ctinfo.h"
#include "dbug.h"
#include "globals.h"
#include "memory.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"

/* INFO structure */
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

/* Traversal functions */
node *RELfunctionbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("RELfunctionbody");

    /* Traverse through vardecls */
    FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVopt(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);

    if (INFO_HEAD(arg_info) != NULL)
    {
        if (FUNCTIONBODY_STMTS(arg_node) == NULL)
            FUNCTIONBODY_STMTS(arg_node) = INFO_HEAD(arg_info);
        else
        {
            /* Add the new statements in front of old statements */
            STMTS_NEXT(INFO_STMTS(arg_info)) = FUNCTIONBODY_STMTS(arg_node);
            FUNCTIONBODY_STMTS(arg_node) = INFO_HEAD(arg_info);
        }

        /* Reset the head node */
        INFO_HEAD(arg_info) = NULL;
    }

    FUNCTIONBODY_LOCALFUNCTION(arg_node) = TRAVopt(FUNCTIONBODY_LOCALFUNCTION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *RELvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("RELvardeclaration");

    if (VARDECLARATION_INIT(arg_node) != NULL)
    {
        /* Make the regular expression as Statement node. */
        char *name = STRcpy(VARDECLARATION_NAME(arg_node));
        node *varlet = TBmakeVarlet(name, NULL, NULL);
        VARLET_SYMBOLTABLEENTRY(varlet) = VARDECLARATION_SYMBOLTABLEENTRY(arg_node);
        VARLET_SCOPE(varlet) = VARDECLARATION_SCOPE(arg_node);
        node *newAssign = TBmakeAssign(VARDECLARATION_INIT(arg_node), varlet);
        node *newStmt = TBmakeStmts(newAssign, NULL);

        /* Check if it is the first new vardeclaration */
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

        /* 'Remove' vardeclaration expression */
        VARDECLARATION_INIT(arg_node) = NULL;
    }

    VARDECLARATION_NEXT(arg_node) = TRAVopt(VARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Traversal start function */

node *RELdoRegularExpr(node *syntaxtree)
{
    DBUG_ENTER("RELdoRegularExpr");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_rel);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}