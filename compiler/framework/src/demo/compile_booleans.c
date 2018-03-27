/* 
    BO_add = disjunction (||), BO_mul = conjunction (&&),
    if (bool_left) && (bool_right) -> pred ?then :else 

    1) Extend the intermediate representation of the compiler 
    2) Systematically transform all Boolean operations in question into semantically equivalent conditional expressions.

    Notes: Conditional expressions may be left until code generation, which turns out to be much easier.
    Note that CiviC also features eager disjunction (Boolean addition) and conjunction (Boolean
    multiplication) with standard left-to-right operand evaluation semantics. They are not concerned
    by this milestone and can be compiled into corresponding VM instructions during code generation.
*/

#include "compile_booleans.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"

/* Traversal functions */
node *CBbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CBbinop");

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), NULL);
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), NULL);

    // if (BINOP_OP(arg_node) == BO_and || BINOP_OP(arg_node) == BO_or)
    //     CTInote("Found binop.");

    // // Create new node?
    // node *new = TBmakeCompBoolean(pred, then, else);

    // // Free this binop node?
    // node *next = FREEdoFreeNode(arg_node);
    // if (next != NULL)
    //     CTInote("Whoops.");

    DBUG_RETURN(arg_node);
}

/* Traversal start function. */
node *CBdoCompileBooleans(node *syntaxtree)
{
    DBUG_ENTER("CBdoCompileBooleans");

    TRAVpush(TR_cb);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
