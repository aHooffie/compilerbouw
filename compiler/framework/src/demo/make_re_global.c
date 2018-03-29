
/*****************************************************************************
 *
 * Module: make_regularexpr
 *
 * Prefix: REG
 *
 * Description:
 *
 * This module implements a traversal of the abstract syntax tree that 
 * turns global variable initialisations into regular assignments
 *
 *****************************************************************************/


/*
VRAGEN:

- FREE'EN GAAT NOG NIET GOED

*/


#include "make_re_global.h"
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

node *REGglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("REGglobaldef");

    if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    {
        // left hand side assign
        char *name = STRcpy(GLOBALDEF_NAME(arg_node));
        node *newAssign = TBmakeAssign(TBmakeVarlet(name, NULL, NULL), GLOBALDEF_ASSIGN(arg_node));
        node *newStmt = TBmakeStmts(newAssign, NULL);

        if (INFO_STMTS(arg_info) == NULL)
        {
            INFO_HEAD(arg_info) = newStmt;
            INFO_STMTS(arg_info) = newStmt;
            CTInote("globDEF: MADE NEW ASSIGN");
        }
        else
        {
            STMTS_NEXT(INFO_STMTS(arg_info)) = newStmt;
            INFO_STMTS(arg_info) = newStmt;
            CTInote("globDEF: ADDED TO STMTS + UPDATED INFO STMTS");
        }

        // 'Remove' globaldef expression
        GLOBALDEF_ASSIGN(arg_node) = NULL;
    }

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


/*
 * Traversal start function
 */

node *REGdoRegularExpr(node *syntaxtree)
{
    DBUG_ENTER("REGdoRegularExpr");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_reg);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    CTInote("Traversing done...\n");

    if (INFO_STMTS(arg_info) != NULL)
    {
        node *allStmts = TBmakeFunctionbody(NULL, NULL, INFO_HEAD(arg_info));
        node *init_func = TBmakeFunction(T_void, STRcpy("__init"), allStmts, NULL);

        // add to syntaxtree
        PROGRAM_DECLARATIONS(syntaxtree) = TBmakeDeclarations(PROGRAM_DECLARATIONS(syntaxtree), init_func);
    }

    DBUG_RETURN(syntaxtree);
}