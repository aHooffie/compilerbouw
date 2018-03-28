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
    CTInote("FOUND A CAST!");
    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);
    INFO_TYPE(arg_info) = CAST_TYPE(arg_node);

    node *new, *condition, *then, *otherwise, *temp;
    type expr = INFO_TYPE(arg_info);
    type cast = CAST_TYPE(arg_node);

    temp = CAST_EXPR(arg_node);
    // If bool = False, then 0, else 1;
    // if 5 == 0 then false else true

    switch (cast)
    {
    case T_bool:
        CTInote("Casting to bool!");
        if (expr == T_float)
            condition = TBmakeBinop(BO_eq, temp, TBmakeFloat(0.0));
        else if (expr == T_int)
            condition = TBmakeBinop(BO_eq, temp, TBmakeNum(0));
        else if (expr == T_bool)
            condition = NULL; // TODO
        else
        {
            condition = NULL;
            CTIabort("HELP");
        }

        then = TBmakeBool(FALSE);
        otherwise = TBmakeBool(TRUE);
        new = TBmakeTernop(condition, then, otherwise);
        break;
    case T_int:
        CTInote("Casting to int!");
        if (expr == T_float)
            condition = TBmakeBinop(BO_eq, temp, TBmakeFloat(0.0));
        else if (expr == T_bool)
            condition = TBmakeBinop(BO_eq, temp, TBmakeBool(FALSE));
        else if (expr == T_int)
            condition = NULL; // TODO
        else
        {
            condition = NULL;
            CTIabort("HELP");
        }
        then = TBmakeNum(0);
        otherwise = TBmakeNum(1);

        new = TBmakeTernop(condition, then, otherwise);
        break;
    case T_float:
        CTInote("Casting to float!");
        if (expr == T_int)
            condition = TBmakeBinop(BO_eq, temp, TBmakeNum(0));
        else if (expr == T_bool)
            condition = TBmakeBinop(BO_eq, temp, TBmakeBool(FALSE));
        else if (expr == T_float)
            condition = NULL; // TODO
        else
        {
            condition = NULL;
            CTIabort("HELP");
        }

        then = TBmakeFloat(0.0);
        otherwise = TBmakeFloat(1.0);

        new = TBmakeTernop(condition, then, otherwise);
        break;
    default:
        new = NULL;
        CTIabort("Cannot cast to this type, line: %i", NODE_LINE(arg_node));
        break;
    }

    if (new != NULL)
    {
        CAST_EXPR(arg_node) = NULL;
        node *n = FREEdoFreeNode(arg_node);
        if (n != NULL)
            CTInote("Whoops.");
        DBUG_RETURN(new);
    }
    else
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