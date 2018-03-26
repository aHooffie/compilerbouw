/*
 * Module: Changing parameters for Arrays
 * Prefix: EP
 */

/* Change parameter passing for arrays from 

void foo (int [m, n] a)
to 
void foo (int m, int n, int[m, n] a)

*/

#include "edit_paramarrays.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "globals.h"
#include "memory.h"
#include "ctinfo.h"

/* INFO structure */
struct INFO
{
    int errors;
};

/* INFO macros */
#define INFO_ERRORS(n) ((n)->errors)

/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_ERRORS(result) = 0;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/* Traversal functions */
node *EPfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("EPfunction");
    DBUG_RETURN(arg_node);
}

node *EPfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("EPfunctioncallstmt");
    DBUG_RETURN(arg_node);
}

node *EPfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("EPfunctioncallexpr");
    DBUG_RETURN(arg_node);
}

node *EPparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("EPfunctionparameters");
    DBUG_RETURN(arg_node);
}

node *EParrayexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("EParrayexpr");
    DBUG_RETURN(arg_node);
}

node *EPexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("EPexpressions");
    DBUG_RETURN(arg_node);
}

node *EPids(node *arg_node, info *arg_info)
{
    DBUG_ENTER("EPids");
    DBUG_RETURN(arg_node);
}

void *EPdoEditParamarrays(node *syntaxtree)
{
    DBUG_ENTER("EPdoEditParamarrays");

    info *arg_info;
    arg_info = MakeInfo();

    arg_info = FreeInfo(arg_info);

    TRAVpush(TR_ep);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    if (INFO_ERRORS(arg_info) != 0)
        CTIabort("Found %i error(s) during type checking. Aborting the compilation.", INFO_ERRORS(arg_info));

    CTInote("Traversing done...\n");

    DBUG_RETURN(syntaxtree);
}