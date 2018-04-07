/*
 * Module: Traverse and check correct use of types. (incl. Milestone 9 & 10)
 * Prefix: TC
 * Author: Andrea van den Hooff
 * Arrays not implemented.
 */

#include "type_checking.h"

#include "ctinfo.h"
#include "dbug.h"
#include "free.h"
#include "memory.h"
#include "str.h"
#include "tree_basic.h"
#include "traverse.h"
#include "types.h"

extern char *TypetoString(type Type);

/* INFO structure */
struct INFO
{
    node *original;
    type current;
    int errors;
    int parametercount;
    int functionreturncount;
    int stmtsscope;
    bool laststatement;
};

/* INFO macros */
#define INFO_ORIGINAL(n) ((n)->original)
#define INFO_TYPE(n) ((n)->current)
#define INFO_ERRORS(n) ((n)->errors)
#define INFO_PC(n) ((n)->parametercount)
#define INFO_FRC(n) ((n)->functionreturncount)
#define INFO_LS(n) ((n)->laststatement)
#define INFO_SS(n) ((n)->stmtsscope) // to check stmts scope for return value in last position.

/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_ORIGINAL(result) = NULL;
    INFO_TYPE(result) = T_unknown;
    INFO_ERRORS(result) = 0;
    INFO_PC(result) = 0;
    INFO_FRC(result) = 0;
    INFO_SS(result) = 0;
    INFO_LS(result) = TRUE;
    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/* Traversal functions below. */
/* Declarations. */
node *TCglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCglobaldec");

    if (basictypeCheck(GLOBALDEC_TYPE(arg_node) == FALSE))
        typeError(arg_info, arg_node, "The global declaration is not of a basic type.");

    /* Traverse into array grammar. Not implemented. */
    GLOBALDEC_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEC_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCglobaldef");

    /* Check assigned type of global definition. */
    if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    {
        GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);
        if (INFO_TYPE(arg_info) != GLOBALDEF_TYPE(arg_node))
            typeError(arg_info, arg_node, "Global definition is not of matching type.");
    }

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    /* Traverse into array grammar. Not implemented. */
    GLOBALDEF_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEF_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Check calls to return type in function node. */
node *TCfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunction");

    /* Traverse into child nodes. */
    FUNCTION_PARAMETERS(arg_node) = TRAVopt(FUNCTION_PARAMETERS(arg_node), arg_info);
    FUNCTION_FUNCTIONBODY(arg_node) = TRAVopt(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    /* Check if there is a return type. */
    if (FUNCTION_ISEXTERN(arg_node) == FALSE)
    {
        if (INFO_LS(arg_info) == FALSE && FUNCTION_TYPE(arg_node) != T_void)
            typeError(arg_info, arg_node, "Function is missing a return call as final code line.");

        if (INFO_FRC(arg_info) == 0 && FUNCTION_TYPE(arg_node) != T_void)
            typeError(arg_info, arg_node, "Function is missing a return call as final code line.");
    }

    INFO_LS(arg_info) = TRUE;

    INFO_FRC(arg_info) = 0;

    DBUG_RETURN(arg_node);
}

node *TCstmts(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunction");

    /* Traverse into child nodes. */
    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);
    STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);

    // !!!! DEZE CHECK KLOPT NIET!! DO_WHILE HEEFT OOK EEN NEXT NODE DIE NULL IS
    if (INFO_SS(arg_info) == 0 &&
        STMTS_NEXT(arg_node) == NULL &&
        NODE_TYPE(STMTS_STMT(arg_node)) != N_return)
        INFO_LS(arg_info) = FALSE;

    DBUG_RETURN(arg_node);
}

/* Check parameters in a fundef. */
node *TCparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCparameters");

    if (basictypeCheck(PARAMETERS_TYPE(arg_node) == FALSE))
        typeError(arg_info, arg_node, "The parameter is not of a basic type.");

    /* Traverse into next parameters. */
    PARAMETERS_NEXT(arg_node) = TRAVopt(PARAMETERS_NEXT(arg_node), arg_info);

    /* Traverse into array grammar. Not implemented. */
    PARAMETERS_DIMENSIONS(arg_node) = TRAVopt(PARAMETERS_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Check the link with the original function. */
node *TCfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunctioncallstmt");

    /* Go through parameters of function call if it has any. */
    if (FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) != NULL)
    {
        INFO_PC(arg_info) = 0;

        node *originalFunction = SYMBOLTABLEENTRY_ORIGINAL(FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node));
        if (NODE_TYPE(originalFunction) != N_function)
            typeError(arg_info, arg_node, "Something went wrong in the function call.");
        else
        {
            INFO_ORIGINAL(arg_info) = originalFunction;
            FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);
        }
    }

    DBUG_RETURN(arg_node);
}

/* Check parameters in a function call position. */
node *TCexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCexpressions");

    INFO_PC(arg_info) += 1;

    /* Find the parameter in function definition that needs to correspond. */
    node *param = FUNCTION_PARAMETERS(INFO_ORIGINAL(arg_info));
    for (int j = 1; j < INFO_PC(arg_info); j++)
        param = PARAMETERS_NEXT(param);

    /* Find own type, put it in arg_info. */
    EXPRESSIONS_EXPR(arg_node) = TRAVopt(EXPRESSIONS_EXPR(arg_node), arg_info);

    if (INFO_TYPE(arg_info) != PARAMETERS_TYPE(param))
        typeError(arg_info, arg_node, "Parameter type is not matching function declaration.");

    /* Traverse into next. */
    EXPRESSIONS_NEXT(arg_node) = TRAVopt(EXPRESSIONS_NEXT(arg_node), arg_info);

    /* Check if function call type matches assignment to variable. */
    INFO_TYPE(arg_info) = FUNCTION_TYPE(INFO_ORIGINAL(arg_info));

    DBUG_RETURN(arg_node);
}

/* Functioncallexpr */
node *TCfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunctioncallexpr");

    INFO_PC(arg_info) = 0;

    node *originalFunction = SYMBOLTABLEENTRY_ORIGINAL(FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node));
    INFO_ORIGINAL(arg_info) = originalFunction;
    INFO_TYPE(arg_info) = FUNCTION_TYPE(INFO_ORIGINAL(arg_info));

    /* Go through parameters of function call if it has any. */
    if (FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) != NULL)
    {
        INFO_PC(arg_info) = 0;
        if (FUNCTION_TYPE(originalFunction) == T_void)
            typeError(arg_info, arg_node, "The return value of this function cannot be assigned to variable.");

        FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) = TRAVopt(FUNCTIONCALLEXPR_EXPRESSIONS(arg_node), arg_info);
    }

    /* Reset. */
    INFO_ORIGINAL(arg_info) = NULL;

    DBUG_RETURN(arg_node);
}

/* Function body statements below. */
node *TCfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfor");

    /* Start condition. */
    FOR_START(arg_node) = TRAVdo(FOR_START(arg_node), arg_info);
    if (INFO_TYPE(arg_info) != T_int)
        typeError(arg_info, arg_node, "For start expression is not of integer type.");

    /* Stop condition. */
    FOR_STOP(arg_node) = TRAVdo(FOR_STOP(arg_node), arg_info);
    if (INFO_TYPE(arg_info) != T_int)
        typeError(arg_info, arg_node, "For stop expression is not of integer type.");

    /* Optional step condition. */
    if (FOR_STEP(arg_node) != NULL)
    {
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
        if (INFO_TYPE(arg_info) != T_int)
            typeError(arg_info, arg_node, "For step expression is not of integer type.");
    }

    /* Go into for stmts and increase statements scope with 1. */
    INFO_SS(arg_info) += 1;
    FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);
    INFO_SS(arg_info) -= 1;

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    DBUG_RETURN(arg_node);
}

node *TCifelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCifelse");

    /* Check condition. */
    IFELSE_CONDITION(arg_node) = TRAVdo(IFELSE_CONDITION(arg_node), arg_info);
    if (basictypeCheck(INFO_TYPE(arg_info)) == FALSE)
        typeError(arg_info, arg_node, "If condition is not a basic type.");

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    /* Go into if stmts and increase statements scope with 1. */
    INFO_SS(arg_info) += 1;
    IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);

    /* Traverse through else block of statements.*/
    if (IFELSE_ELSE(arg_node) != NULL)
    {
        INFO_SS(arg_info) += 1;
        IFELSE_ELSE(arg_node) = TRAVdo(IFELSE_ELSE(arg_node), arg_info);
        INFO_SS(arg_info) -= 1;
    }

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    DBUG_RETURN(arg_node);
}

node *TCwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCwhile");

    /* Check condition. */
    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    if (basictypeCheck(INFO_TYPE(arg_info)) == FALSE)
        typeError(arg_info, arg_node, "While condition is not a basic type.");

    /* Traverse through block of statements.*/
    INFO_SS(arg_info) += 1;
    WHILE_BLOCK(arg_node) = TRAVdo(WHILE_BLOCK(arg_node), arg_info);
    INFO_SS(arg_info) -= 1;

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    DBUG_RETURN(arg_node);
}

node *TCdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCdowhile");

    /* Traverse through block of statements.*/
    INFO_SS(arg_info) += 1;
    DOWHILE_BLOCK(arg_node) = TRAVdo(DOWHILE_BLOCK(arg_node), arg_info);
    INFO_SS(arg_info) -= 1;

    /* Check condition. */
    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);

    if (basictypeCheck(INFO_TYPE(arg_info)) == FALSE)
        typeError(arg_info, arg_node, "The dowhile condition is not a basic type.");

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;
    DBUG_RETURN(arg_node);
}

node *TCreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCreturn");

    /* Check the return type. */
    if (RETURN_EXPR(arg_node) != NULL)
        RETURN_EXPR(arg_node) = TRAVdo(RETURN_EXPR(arg_node), arg_info);
    else
        INFO_TYPE(arg_info) = T_void;

    if (INFO_TYPE(arg_info) != SYMBOLTABLEENTRY_TYPE(RETURN_SYMBOLTABLEENTRY(arg_node)))
        typeError(arg_info, arg_node, "Return expression does not match function type.");

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;
    INFO_FRC(arg_info) += 1;

    DBUG_RETURN(arg_node);
}

/* Check if left and right part of assignment are of the same types. */
node *TCassign(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCassign");

    type t;

    /* Traverse left side of an assignment. */
    ASSIGN_LET(arg_node) = TRAVopt(ASSIGN_LET(arg_node), arg_info);
    t = INFO_TYPE(arg_info);

    /* Traverse right side and compare. */
    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);
    if (t != INFO_TYPE(arg_info))
        typeError(arg_info, arg_node, "Assigning types failed. Not same types!");

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    DBUG_RETURN(arg_node);
}

/* Check if cast is eligible. */
node *TCcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCcast");

    node *new, *condition, *then, *otherwise, *expression;

    /* Type check of expression.*/
    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);
    type exprType = INFO_TYPE(arg_info);
    type cast = CAST_TYPE(arg_node);

    INFO_TYPE(arg_info) = CAST_TYPE(arg_node);
    expression = CAST_EXPR(arg_node);

    /* Change bool > int/float & int/float > bool cast into Ternary operator. */
    switch (cast)
    {
    case T_bool:
        if (exprType == T_float)
            condition = TBmakeBinop(BO_eq, expression, TBmakeFloat(0.0));
        else if (exprType == T_int)
            condition = TBmakeBinop(BO_eq, expression, TBmakeNum(0));
        else if (exprType == T_bool)
        {
            CAST_EXPR(arg_node) = NULL;
            FREEdoFreeNode(arg_node);
            DBUG_RETURN(expression);
        }
        else
            DBUG_RETURN(arg_node);

        then = TBmakeBool(FALSE);
        otherwise = TBmakeBool(TRUE);
        new = TBmakeTernop(condition, then, otherwise);
        break;
    case T_int:
        if (exprType == T_bool)
            condition = TBmakeBinop(BO_eq, expression, TBmakeBool(FALSE));
        else if (exprType == T_int)
        {
            CAST_EXPR(arg_node) = NULL;
            FREEdoFreeNode(arg_node);
            DBUG_RETURN(expression);
        }
        else
            DBUG_RETURN(arg_node);

        then = TBmakeNum(0);
        otherwise = TBmakeNum(1);
        new = TBmakeTernop(condition, then, otherwise);
        break;
    case T_float:
        if (exprType == T_bool)
            condition = TBmakeBinop(BO_eq, expression, TBmakeBool(FALSE));
        else if (exprType == T_float)
        {
            CAST_EXPR(arg_node) = NULL;
            FREEdoFreeNode(arg_node);
            DBUG_RETURN(expression);
        }
        else
            DBUG_RETURN(arg_node);

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
            CTIwarn("Tried to free a cast node but failed.");
        DBUG_RETURN(new);
    }
    else
        DBUG_RETURN(arg_node);
}

/* Monop. */
node *TCmonop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCmonop");

    MONOP_EXPR(arg_node) = TRAVdo(MONOP_EXPR(arg_node), arg_info);

    /* - Can be either a float or an int, ! can be all three basic types. */
    switch (MONOP_OP(arg_node))
    {
    case MO_neg:
        if (INFO_TYPE(arg_info) != T_int && INFO_TYPE(arg_info) != T_float)
            typeError(arg_info, arg_node, "Wrong Monop types. ");
        break;
    case MO_not:
        if (INFO_TYPE(arg_info) != T_bool)
            typeError(arg_info, arg_node, "Wrong Monop types. ");
        break;
    default:
        typeError(arg_info, arg_node, "Conflicting type + monop found.");
        break;
    }

    DBUG_RETURN(arg_node);
}

/* Binop. */
node *TCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbinop");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    nodetype left = NODE_TYPE(BINOP_LEFT(arg_node));
    if (left != N_num && left != N_bool && left != N_float)
    {
        switch (INFO_TYPE(arg_info))
        {
        case T_int:
            left = N_num;
            break;
        case T_float:
            left = N_float;
            break;
        case T_bool:
            left = N_bool;
            break;
        default:
            typeError(arg_info, arg_node, "Unknown types in binop.");
        }
    }
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);
    nodetype right = NODE_TYPE(BINOP_RIGHT(arg_node));
    if (right != N_num && right != N_bool && right != N_float)
    {
        switch (INFO_TYPE(arg_info))
        {
        case T_int:
            right = N_num;
            break;
        case T_float:
            right = N_float;
            break;
        case T_bool:
            right = N_bool;
            break;
        default:
            typeError(arg_info, arg_node, "Unknown types in binop.");
        }
    }

    BINOP_TYPE(arg_node) = INFO_TYPE(arg_info);

    switch (BINOP_OP(arg_node))
    {
    case BO_mod:
        /* Modulo; can only be done with integers. */
        if (left != N_num || right != N_num)
            typeError(arg_info, arg_node, "Modulo can only be performed on two integers.");
        else
            INFO_TYPE(arg_info) = T_int;
        break;

    case BO_add:
    case BO_sub:
    case BO_mul:
    case BO_div:
        /* +, -, *, and / can be done with integers and floats. */
        switch (left)
        {
        case N_num:
            if (right == N_num)
                INFO_TYPE(arg_info) = T_int;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_int)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_int;
            }
            else
            {
                typeError(arg_info, arg_node, "!!! Types are not matching..");
            }
            break;

        case N_float:
            if (right == N_float)
                INFO_TYPE(arg_info) = T_float;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_float;
            }
            else
                typeError(arg_info, arg_node, "Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) == T_bool &&
                (right == T_bool ||
                 SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))) == T_bool))
            {
                if (BINOP_OP(arg_node) == BO_add || BINOP_OP(arg_node) == BO_mul)
                    INFO_TYPE(arg_info) = T_bool;
                else
                    typeError(arg_info, arg_node, "arithmetic operations - and / are not allowed on booleans.");
            }
            else if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                     SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, arg_node, "Types are not matching..");
            else
            {
                if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) == T_int)
                    INFO_TYPE(arg_info) = T_int;
                else
                    INFO_TYPE(arg_info) = T_float;
            }
            break;
        case N_bool:
            if (right == N_bool)
            {
                if (BINOP_OP(arg_node) == BO_add || BINOP_OP(arg_node) == BO_mul)
                    INFO_TYPE(arg_info) = T_bool;
                else
                    typeError(arg_info, arg_node, "arithmetic operations - and / are not allowed on booleans.");
            }
            else if (right == N_var)
            {
                if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))) == T_bool)
                {
                    if (BINOP_OP(arg_node) == BO_add || BINOP_OP(arg_node) == BO_mul)
                        INFO_TYPE(arg_info) = T_bool;
                    else
                        typeError(arg_info, arg_node, "arithmetic operations - and / are not allowed on booleans.");
                }
                else
                    typeError(arg_info, arg_node, "Types are not matching.. ");
            }
            else
                typeError(arg_info, arg_node, "arithmetic operations - and / are not allowed on booleans.");
            break;

        default:
            typeError(arg_info, arg_node, "Conflicting type + binop found.");
            break;
        }
        break;

    case BO_lt:
    case BO_le:
    case BO_gt:
    case BO_ge:
        /* <, <=, >, and >= can be done with integers and floats, return bools. */
        switch (left)
        {
        case N_num:
            if (right == N_num)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_int)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, arg_node, "Types are not matching..");
            break;
        case N_float:
            if (right == N_float)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, arg_node, "Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, arg_node, "Types are not matching..");
            else
                INFO_TYPE(arg_info) = T_bool;
            break;
        default:
            typeError(arg_info, arg_node, "Unknown / not allowed types.");
            break;
        }
        break;
    case BO_eq:
    case BO_ne:
        /* != and == can be done with all types. */
        switch (left)
        {
        case N_num:
            if (right == N_num)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_int)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, arg_node, "Types are not matching..");
            break;
        case N_float:
            if (right == N_float)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, arg_node, "Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, arg_node, "Types are not matching..");
            else
                INFO_TYPE(arg_info) = T_bool;
            break;
        case N_bool:
            if (right == N_bool)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_bool)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, arg_node, "Types are not matching..");
            break;
        default:
            typeError(arg_info, arg_node, "Types are not matching..");
            break;
        }
        break;

    case BO_and:
    case BO_or:
        switch (left)
        {

        case N_bool:
            if (right == N_bool)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_bool)
                    typeError(arg_info, arg_node, "Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, arg_node, "Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, arg_node, "Types are not matching..");
            else
                INFO_TYPE(arg_info) = T_bool;
            break;
        default:
            typeError(arg_info, arg_node, "Types are not matching..");
            break;
        }
        break;
    default:
        typeError(arg_info, arg_node, "Unknown binop type.");
        break;
    }

    // MILESTONE 10!
    node *new;
    if (BINOP_OP(arg_node) == BO_and || BINOP_OP(arg_node) == BO_or)
    {
        node *tempR = BINOP_RIGHT(arg_node);
        node *tempL = BINOP_LEFT(arg_node);

        if (BINOP_OP(arg_node) == BO_and)
        {
            /* Create new node. */
            node *otherwise = TBmakeBool(FALSE);
            new = TBmakeTernop(tempL, tempR, otherwise);
            TERNOP_OP(new) = BO_and;
            if (new == NULL)
                CTIabort("Something went wrong in checking the types in line %i.", NODE_LINE(arg_node));
        }
        else
        {
            /* Create new node. */
            node *then = TBmakeBool(TRUE);
            new = TBmakeTernop(tempL, then, tempR);
            TERNOP_OP(new) = BO_or;
            if (new == NULL)
                CTIabort("Something went wrong in checking the types in line %i.", NODE_LINE(arg_node));
        }

        /* Free the original binop node? */
        BINOP_RIGHT(arg_node) = NULL;
        BINOP_LEFT(arg_node) = NULL;

        node *next = FREEdoFreeNode(arg_node);
        if (next != NULL)
            CTIabort("Something went wrong in checking the types in line %i.", NODE_LINE(arg_node));

        DBUG_RETURN(new);
    }

    DBUG_RETURN(arg_node);
}

/* Vars */
node *TCvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvar");

    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(arg_node));

    /* Array indices, not implemented. */
    VAR_INDICES(arg_node) = TRAVopt(VAR_INDICES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvarlet");

    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(VARLET_SYMBOLTABLEENTRY(arg_node));

    /* Traverse over rest. */
    VARLET_NEXT(arg_node) = TRAVopt(VARLET_NEXT(arg_node), arg_info);

    /* Array indices, not implemented. */
    VARLET_INDICES(arg_node) = TRAVopt(VARLET_INDICES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Check calls to return type in function node. */
node *TCvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvardeclaration");

    VARDECLARATION_INIT(arg_node) = TRAVopt(VARDECLARATION_INIT(arg_node), arg_info);

    if (VARDECLARATION_INIT(arg_node) != NULL &&
        INFO_TYPE(arg_info) != VARDECLARATION_TYPE(arg_node))
        typeError(arg_info, arg_node, "Type of vardeclaration isn't matching assignment. ");

    /* Traverse over rest. */
    VARDECLARATION_NEXT(arg_node) = TRAVopt(VARDECLARATION_NEXT(arg_node), arg_info);

    /* Array indices, not implemented. */
    VARDECLARATION_DIMENSIONS(arg_node) = TRAVopt(VARDECLARATION_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Basic Types. */
node *TCfloat(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfloat");

    INFO_TYPE(arg_info) = T_float;

    DBUG_RETURN(arg_node);
}

node *TCnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCnum");

    INFO_TYPE(arg_info) = T_int;

    DBUG_RETURN(arg_node);
}

node *TCbool(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbool");

    INFO_TYPE(arg_info) = T_bool;

    DBUG_RETURN(arg_node);
}

/* Part of array grammar. Not implemented. */
node *TCids(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCids");

    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(arg_node));

    if (INFO_TYPE(arg_info) != T_int)
        typeError(arg_info, arg_node, "Type of an array index has to be an integer. ");

    /* Array indices, not implemented. */
    IDS_NEXT(arg_node) = TRAVopt(IDS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Traversal start function */
node *TCdoTypeChecking(node *syntaxtree)
{
    DBUG_ENTER("TCdoTypeChecking");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_tc);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    if (INFO_ERRORS(arg_info) != 0)
        CTIabort("Found %i error(s) during type checking. Aborting the compilation.", INFO_ERRORS(arg_info));

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

/* Called when a typecheck error arises. */
void typeError(info *arg_info, node *arg_node, char *message)
{
    CTIwarn("! Error on line %i, col %i. %s", NODE_LINE(arg_node), NODE_COL(arg_node), message);
    INFO_ERRORS(arg_info) += 1;
    INFO_TYPE(arg_info) = T_unknown;
}

/* Checks if a type is an int, bool or float. */
bool basictypeCheck(type t)
{
    if (t != T_bool &&
        t != T_int &&
        t != T_float)
        return FALSE;
    return TRUE;
}

/* Prints the node type as string, for testing. */
char *nodetypetoString(node *arg_node)
{
    char *typeString;
    switch (NODE_TYPE(arg_node))
    {
    case N_num:
        typeString = "int";
        break;
    case N_float:
        typeString = "float";
        break;
    case N_bool:
        typeString = "bool";
        break;
    case N_var:
        typeString = "var";
        break;
    case N_cast:
        typeString = "cast";
        break;
    case N_functioncallexpr:
        typeString = "functioncallexpr";
        break;
    case N_arrayexpr:
        typeString = "arrayexpr";
        break;
    case N_binop:
        typeString = "int";
        break;
    case N_monop:
        typeString = "int";
        break;
    case N_globaldef:
        typeString = "globaldef";
        break;
    case N_globaldec:
        typeString = "globaldec";
        break;
    case N_vardeclaration:
        typeString = "vardeclaration";
        break;
    default:
        typeString = "Not known yet";
        break;
    }

    return typeString;
}