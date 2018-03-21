/*
 * Module: Traversing and type checking
 * Prefix: TC
 */

/*

 All binary operators are only defined on operands of exactly the same type,
 i.e. there is no implicit type conversion in CiviC.

 Arithmetic operators are defined on integer numbers,
 where they again yield an integer number, and on floating point numbers,
 where they again yield a floating point number.

 As an exception, the modulo operator is only defined on integer numbers and yields an integer number.
 
 The arithmetic operators for addition and multiplication are also defined
 on Boolean operands where they implement strict logic disjunction and conjunction, respectively.
 */

// Arrayexpr doen we niet meer

#include <stdio.h> // for standard error message?? eventueel niet meer nodig == printf
#include "type_checking.h"
#include "add_symboltables.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "ctinfo.h"

/* INFO structure */
struct INFO
{
    int errors;
    type current;
};

/* INFO macros */
#define INFO_ERRORS(n) ((n)->errors)
#define INFO_TYPE(n) ((n)->current)

/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_ERRORS(result) = 0;
    INFO_TYPE(result) = T_unknown;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/* Traversal functions */
node *TCglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCglobaldef");

    // Check if dimensions are integers

    DBUG_RETURN(arg_node);
}

node *TCglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCglobaldec");

    // Check if dimensions are integers

    DBUG_RETURN(arg_node);
}

node *TCparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCparameters");

    // node *entry = PARAMETERS_SYMBOLTABLEENTRY(arg_node);

    DBUG_RETURN(arg_node);
}

node *TCfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunction");

    // Check if dimensions are integers

    DBUG_RETURN(arg_node);
}

/* Assign. */
node *TCassign(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCassign");

    ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);
    type t = INFO_TYPE(arg_info);

    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    if (t != INFO_TYPE(arg_info))
        typeError(arg_info, "Assigning types failed. Not same types!");

    INFO_TYPE(arg_info) = T_unknown;

    DBUG_RETURN(arg_node);
}

/* Cast. */
node *TCcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCcast");

    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);

    INFO_TYPE(arg_info) = CAST_TYPE(arg_node);

    DBUG_RETURN(arg_node);
}

/* Monop. */
node *TCmonop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCmonop");

    MONOP_EXPR(arg_node) = TRAVdo(MONOP_EXPR(arg_node), arg_info);

    switch (MONOP_OP(arg_node))
    {
    case MO_neg:
        if (INFO_TYPE(arg_info) != T_int || INFO_TYPE(arg_info) != T_float)
            typeError(arg_info, "! Error: Wrong Monop types. ");
        break;
    case MO_not:
        break;
    default:
        typeError(arg_info, "! Error: Conflicting type + monop found.");
        break;
    }

    DBUG_RETURN(arg_node);
}

/* Binop - werkt nog niet helemaal goed. */
node *TCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbinop");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    nodetype left = NODE_TYPE(BINOP_LEFT(arg_node));
    nodetype right = NODE_TYPE(BINOP_RIGHT(arg_node));
    printf("Type: %s & Type: %s\n", NodetypetoString(BINOP_LEFT(arg_node)), NodetypetoString(BINOP_RIGHT(arg_node)));

    switch (BINOP_OP(arg_node))
    {
        /* Modulo; can only be done with integers. */
    case BO_mod:
        if (NODE_TYPE(BINOP_LEFT(arg_node)) != N_num || NODE_TYPE(BINOP_RIGHT(arg_node)) != N_num)
            typeError(arg_info, "! Error: Modulo can only be performed on two integers.");
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
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_int;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;

        case N_float:
            if (right == N_float)
                INFO_TYPE(arg_info) = T_float;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_float;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) == T_bool ||
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))) == T_bool)
                typeError(arg_info, "! Error: Bools cannot have arithmetic operations.");
            else if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                     SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, "! Error: Types are not matching..");
            else
            {
                if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) == T_int)
                    INFO_TYPE(arg_info) = T_int;
                else
                    INFO_TYPE(arg_info) = T_float;
            }
            break;

        default:
            typeError(arg_info, "! Error: Conflicting type + binop found.");
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
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;
        case N_float:
            if (right == N_float)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, "! Error: Types are not matching..");
            else
                INFO_TYPE(arg_info) = T_bool;
            break;
        default:
            typeError(arg_info, "Unknown / not allowed types.");
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
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;
        case N_float:
            if (right == N_float)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, "! Error: Types are not matching..");
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
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;
        default:
            typeError(arg_info, "! Error: Types are not matching..");
            break;
        }
        break;

    case BO_and:
    case BO_or:
        /* && and || can only be done with bools. */
        switch (left)
        {

        case N_bool:
            if (right == N_bool)
                INFO_TYPE(arg_info) = T_bool;
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_bool)
                    typeError(arg_info, "! Error: Types are not matching..");
                else
                    INFO_TYPE(arg_info) = T_bool;
            }
            else
                typeError(arg_info, "! Error: Types are not matching..");
            break;
        case N_var:
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
                typeError(arg_info, "! Error: Types are not matching..");
            else
                INFO_TYPE(arg_info) = T_bool;
            break;
        default:
            typeError(arg_info, "! Error: Types are not matching..");
            break;
        }
        break;
    default:
        typeError(arg_info, "Unknown binop type.");
        break;
    }

    DBUG_RETURN(arg_node);
}

/* Vars */
node *TCvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvar");

    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(arg_node));

    if (VAR_INDICES(arg_node) != NULL)
        VAR_INDICES(arg_node) = TRAVdo(VAR_INDICES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCvarlet");

    INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(VARLET_SYMBOLTABLEENTRY(arg_node));

    if (VARLET_NEXT(arg_node) != NULL)
        VARLET_NEXT(arg_node) = TRAVdo(VARLET_NEXT(arg_node), arg_info);

    if (VARLET_INDICES(arg_node) != NULL)
        VARLET_INDICES(arg_node) = TRAVdo(VARLET_INDICES(arg_node), arg_info);

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

/* Traversal start function */
node *TCdoTypeChecking(node *syntaxtree)
{
    DBUG_ENTER("TCdoTypeChecking");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_tc);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    CTInote("Traversing for TC done...");

    if (INFO_ERRORS(arg_info) != 0)
        CTIabort("Found %i error(s) during type checking. Aborting the compilation.", INFO_ERRORS(arg_info));

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

/* Extra errors. */
void typeError(info *arg_info, char *message)
{
    CTInote("%s", message);
    INFO_ERRORS(arg_info) += 1;
    INFO_TYPE(arg_info) = T_unknown;
}

/* Prints the node type as string, for testing. */
char *NodetypetoString(node *arg_node)
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
        typeString = "cast";
        break;
    case N_arrayexpr:
        typeString = "cast";
        break;

    case N_binop:
        typeString = "int";
        break;
    case N_monop:
        typeString = "int";
        break;
    default:
        typeString = "Not known yet";
        break;
    }

    return typeString;
}