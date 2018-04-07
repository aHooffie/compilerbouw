
/*
 * Module: opt_sub
 * Prefix: OS
 * Description: This module implements some simple arithmetic optimisations.
 * Author: Andrea van den Hooff
 */

#include "opt_sub.h"

#include "dbug.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"

/* Traversal functions */
node *OSbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("OSbinop");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    node *n;
    int val;
    float floatval;

    switch (BINOP_OP(arg_node))
    {

    /* Replace subtraction (optimisation). */
    case BO_sub:
        /* If (a - a) */
        if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_var) &&
            (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_var) &&
            STReq(VAR_NAME(BINOP_LEFT(arg_node)), VAR_NAME(BINOP_RIGHT(arg_node))))
        {
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeNum(0);
        }
        /* if (num - num) */
        else if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_num) &&
                 (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num))
        {
            if (NUM_VALUE(BINOP_LEFT(arg_node)) == NUM_VALUE(BINOP_RIGHT(arg_node)))
            {
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = TBmakeNum(0);
            }
            else
            {
                val = NUM_VALUE(BINOP_LEFT(arg_node)) - NUM_VALUE(BINOP_RIGHT(arg_node));
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = TBmakeNum(val);
            }
            /* If float - float */
        }
        else if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_float) &&
                 (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_float))
        {
            floatval = FLOAT_VALUE(BINOP_LEFT(arg_node)) - FLOAT_VALUE(BINOP_RIGHT(arg_node));
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeFloat(floatval);
        }
        break;
    /* Replace multiplication: var * 0, var * 1 & num * num (optimisation). */
    case BO_mul: // possibly loats too?
        if (NODE_TYPE(BINOP_LEFT(arg_node)) == N_num)
        {
            /* if 0 * var or 0 * num */
            if (NUM_VALUE(BINOP_LEFT(arg_node)) == 0 &&
                (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_var ||
                 NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num))
            {
                n = BINOP_LEFT(arg_node);
                BINOP_LEFT(arg_node) = NULL;
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = n;
            }
            /* if 1 * var or 1 * num */
            else if (NUM_VALUE(BINOP_LEFT(arg_node)) == 1 &&
                     (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_var ||
                      NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num))
            {
                n = BINOP_RIGHT(arg_node);
                BINOP_RIGHT(arg_node) = NULL;
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = n;
            }
            /* num * num */
            else if (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num)
            {
                val = NUM_VALUE(BINOP_LEFT(arg_node)) * NUM_VALUE(BINOP_RIGHT(arg_node));
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = TBmakeNum(val);
            }
        }
        else if (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num)
        {
            if (NUM_VALUE(BINOP_RIGHT(arg_node)) == 0 &&
                (NODE_TYPE(BINOP_LEFT(arg_node)) == N_var ||
                 NODE_TYPE(BINOP_LEFT(arg_node)) == N_num))
            {
                n = BINOP_RIGHT(arg_node);
                BINOP_RIGHT(arg_node) = NULL;
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = n;
            }
            else if (NUM_VALUE(BINOP_RIGHT(arg_node)) == 1 &&
                     (NODE_TYPE(BINOP_LEFT(arg_node)) == N_var ||
                      NODE_TYPE(BINOP_LEFT(arg_node)) == N_num))
            {
                n = BINOP_LEFT(arg_node);
                BINOP_LEFT(arg_node) = NULL;
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = n;
            }
        }
        break;
        /* Replace division. */
    case BO_div:
        /* num / num. */
        if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_num) &&
            (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num) &&
            (NUM_VALUE(BINOP_RIGHT(arg_node)) != 0))
        {
            val = NUM_VALUE(BINOP_LEFT(arg_node)) / NUM_VALUE(BINOP_RIGHT(arg_node));
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeNum(val);
        }
        break;
    case BO_add:
        /* Constant folding: 2 + 4  = 6 (optimisation). */
        if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_num) &&
            (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num))
        {
            val = NUM_VALUE(BINOP_LEFT(arg_node)) + NUM_VALUE(BINOP_RIGHT(arg_node));
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeNum(val);
        }
        else if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_float) &&
                 (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_float))
        {
            floatval = FLOAT_VALUE(BINOP_LEFT(arg_node)) + FLOAT_VALUE(BINOP_RIGHT(arg_node));
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeFloat(floatval);
        }
        break;
    default:
        break;
    }
    DBUG_RETURN(arg_node);
}

/* Traversal start function */
node *OSdoOptSub(node *syntaxtree)
{
    DBUG_ENTER("OSdoOptSub");

    TRAVpush(TR_os);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
