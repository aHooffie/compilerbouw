/* 
 * Andrea van den Hooff
 * 10439080
 * Premaster Software Engineering
 * 
 * Module: sum_identifiers
 * Prefix: SID
 * Description:
 * This module implements a traversal of the abstract syntax tree that 
 * counts the occurence of all identifiers and prints the result at the end of the traversal.
 */

#include "sum_identifiers.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "lookup_table.h"

#include "memory.h"
#include "ctinfo.h"

/* INFO structure*/
struct INFO
{
    lut_t *table;
};

/* INFO macros */
#define INFO_TABLE(n) ((n)->table)

/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");
    result = (info *)MEMmalloc(sizeof(info));
    INFO_TABLE(result) = LUTgenerateLut();

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");
    info = MEMfree(info);

    /* Free counters. */
    DBUG_RETURN(info);
}

/* Traversal functions */
node *SIDvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("SIDvarlet");
    int *count = (int *)MEMmalloc(sizeof(int));
    *count = 1;

    /* Add a new pointer pair (VAR_NAME, COUNT) to the lookuptable. */
    INFO_TABLE(arg_info) = LUTinsertIntoLutS(
        INFO_TABLE(arg_info), VARLET_NAME(arg_node), count);

    DBUG_RETURN(arg_node);
}

/* Raise the counter for an identifier that has been added to table already. */
node *SIDvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("SIDvar");
    void **data = LUTsearchInLutS(INFO_TABLE(arg_info), VAR_NAME(arg_node)); 

    if (data == NULL) {
        int *count = (int *)MEMmalloc(sizeof(int));
        *count = 1;
        INFO_TABLE(arg_info) = LUTinsertIntoLutS(
        INFO_TABLE(arg_info), VAR_NAME(arg_node), count);
    } else {
        void ***counter;
        counter = data;
        **counter = **counter + 1;
    }
    
    DBUG_RETURN(arg_node);
}

/* Prints the counter values. */
void *print_content(void **counter) {
    DBUG_ENTER("SIDprintcontent");
    CTInote("Amount of occurences: %d", *counter);
    DBUG_RETURN(counter);
}

/* Traversal start function */
node *SIDdoSumIdentifiers(node *syntaxtree)
{
    DBUG_ENTER("SIDdoSumIdentifiers");
    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_sid);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    /* Print content of lookuptable (only the counters, not the id's.) */
    INFO_TABLE(arg_info) = LUTmapLutS(INFO_TABLE(arg_info), print_content);

    /* Free the table & info. */
    INFO_TABLE(arg_info) = LUTremoveContentLut(INFO_TABLE(arg_info));
    INFO_TABLE(arg_info) = LUTremoveLut(INFO_TABLE(arg_info));
    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
