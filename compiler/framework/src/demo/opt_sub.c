
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

    /* Replace subtraction (optimisation). */
    if (BINOP_OP(arg_node) == BO_sub)
    {
        /* If (a - a) or (num - num) */
        if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_var) &&
            (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_var) &&
            STReq(VAR_NAME(BINOP_LEFT(arg_node)), VAR_NAME(BINOP_RIGHT(arg_node))))
        {
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeNum(0);
        }
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
        }
    }
    /* Replace multiplication: var * 0, var * 1 & num * num (optimisation). */
    else if (BINOP_OP(arg_node) == BO_mul)
    {
        if (NODE_TYPE(BINOP_LEFT(arg_node)) == N_num)
        {
            if (NUM_VALUE(BINOP_LEFT(arg_node)) == 0 &&
                (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_var ||
                 NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num))
            {
                n = BINOP_LEFT(arg_node);
                BINOP_LEFT(arg_node) = NULL;
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = n;
            }
            else if (NUM_VALUE(BINOP_LEFT(arg_node)) == 1 &&
                     (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_var ||
                      NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num))
            {
                n = BINOP_RIGHT(arg_node);
                BINOP_RIGHT(arg_node) = NULL;
                arg_node = FREEdoFreeTree(arg_node);
                arg_node = n;
            }
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
    }
    /* Replace division: num / num (optimisation). */
    else if (BINOP_OP(arg_node) == BO_div)
    {
        if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_num) &&
            (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num) &&
            (NUM_VALUE(BINOP_RIGHT(arg_node)) != 0))
        {
            val = NUM_VALUE(BINOP_LEFT(arg_node)) / NUM_VALUE(BINOP_RIGHT(arg_node));
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeNum(val);
        }
    }
    /* Constant folding: 2 + 4  = 6 (optimisation). */
    else if (BINOP_OP(arg_node) == BO_add)
    {
        if ((NODE_TYPE(BINOP_LEFT(arg_node)) == N_num) &&
            (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num))
        {
            val = NUM_VALUE(BINOP_LEFT(arg_node)) + NUM_VALUE(BINOP_RIGHT(arg_node));
            arg_node = FREEdoFreeTree(arg_node);
            arg_node = TBmakeNum(val);
        }
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
