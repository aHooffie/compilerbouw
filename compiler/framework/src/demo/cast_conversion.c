/*
    Conversions between Boolean values and numerical values or vice versa must be represented
    differently in the intermediate representation, e.g. again by making use of conditional expressions
    as already recommended for milestone 9. Implement a compiler pass for the systematic transformation of cast
    expressions with Boolean argument or result value into semantically equivalent non-cast expressions.
 */

#include "cast_conversion.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"

node *CCcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CCcast");

    CTInote("Found a Cast");
    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), NULL);

    DBUG_RETURN(arg_node);
}

node *CCdoCastConversion(node *syntaxtree)
{
    DBUG_ENTER("CCdoCastConversion");

    TRAVpush(TR_cc);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}