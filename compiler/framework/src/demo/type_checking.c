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

// !! CHECK IF ALL THESE INCLUDES ARE NESSECARY...

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

node *TCfunction(node *arg_node, info *arg_info)
{
}

// !! NOG EVEN LETTEN OP VOID !!
/* = Alleen rechter deel van een assign, moet nog in een stack oid komen. */
node *TCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbinop");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    nodetype left = NODE_TYPE(BINOP_LEFT(arg_node));
    nodetype right = NODE_TYPE(BINOP_RIGHT(arg_node));
    printf("Type: %s & Type: %s\n", NodetypetoString(BINOP_LEFT(arg_node)), NodetypetoString(BINOP_RIGHT(arg_node)));

    /* Modulo; can only be done with integers. */
    if (BINOP_OP(arg_node) == BO_mod)
    {
        if (NODE_TYPE(BINOP_LEFT(arg_node)) != N_num || NODE_TYPE(BINOP_RIGHT(arg_node)) != N_num)
        {
            CTInote("! Error: Modulo can only be performed on two integers.\n");
            INFO_ERRORS(arg_info) += 1;
        }
    }
    /* Addition, Subtraction, Multiplication, Div*/
    else if (BINOP_OP(arg_node) == BO_add ||
             BINOP_OP(arg_node) == BO_sub ||
             BINOP_OP(arg_node) == BO_mul ||
             BINOP_OP(arg_node) == BO_div)
    {
        if (left == N_num)
        {
            if (right == N_num)
            {
            }
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_int)
                {
                    CTInote("! Error: Types are not matching..\n");
                    INFO_ERRORS(arg_info) += 1;
                }
                else
                    printf("2 Integers\n"); // mag.
            }
            else
            {
                CTInote("! Error: Types are not matching..\n");
                INFO_ERRORS(arg_info) += 1;
            }
        }
        else if (left == N_float)
        {
            if (right == N_float)
                printf("2 Floats\n"); // mag.
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                {
                    CTInote("! Error: Types are not matching..\n");
                    INFO_ERRORS(arg_info) += 1;
                }

                else
                    printf("2 Floats\n"); // mag.
            }
            else
            {
                CTInote("! Error: Types are not matching..\n");
                INFO_ERRORS(arg_info) += 1;
            }
        }
        else if (left == N_var)
        {
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) == T_bool ||
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))) == T_bool)
            {
                CTInote("! Error: Bools cannot have arithmetic operations.\n");
                INFO_ERRORS(arg_info) += 1;
            }
            else if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                     SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
            {
                CTInote("! Error: Types are not matching..\n");
                INFO_ERRORS(arg_info) += 1;
            }
            else
                printf("Corresponding vars.\n");
        }
        else
        {
            CTInote("! Error: Conflicting type + binop found. \n");
            INFO_ERRORS(arg_info) += 1;
        }
    }
    else if (BINOP_OP(arg_node) == BO_lt ||
             BINOP_OP(arg_node) == BO_le ||
             BINOP_OP(arg_node) == BO_gt ||
             BINOP_OP(arg_node) == BO_ge)
    {
        if (left == N_num)
        {
            if (right == N_num)
                printf("2 Integers\n"); // mag.
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_int)
                {
                    CTInote("! Error: Types are not matching..\n");
                    INFO_ERRORS(arg_info) += 1;
                }
                else
                    printf("2 Integers\n"); // mag.
            }
            else
            {
                CTInote("! Error: Types are not matching..\n");
                INFO_ERRORS(arg_info) += 1;
            }
        }
        else if (left == N_float)
        {
            if (right == N_float)
                printf("2 Floats\n"); // mag.
            else if (right == N_var)
            {
                node *original = VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node));
                if (SYMBOLTABLEENTRY_TYPE(original) != T_float)
                {
                    CTInote("! Error: Types are not matching..\n");
                    INFO_ERRORS(arg_info) += 1;
                }
                else
                    printf("2 Floats\n"); // mag.
            }
            else
            {
                CTInote("! Error: Types are not matching..\n");
                INFO_ERRORS(arg_info) += 1;
            }
        }
        else if (left == N_var)
        {
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) !=
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))))
            {
                CTInote("! Error: Types are not matching..\n");
                INFO_ERRORS(arg_info) += 1;
            }
            else
                printf("Corresponding vars.\n");
        }
        else
        {
            CTInote("Unknown / not allowed types.\n");
            INFO_ERRORS(arg_info) += 1;
        }
    }
    else if (BINOP_OP(arg_node) == BO_eq ||
             BINOP_OP(arg_node) == BO_ne)
    {
        // num, float & bool
    }
    else if (BINOP_OP(arg_node) == BO_and ||
             BINOP_OP(arg_node) == BO_or)
    {
        // bool
    }
    DBUG_RETURN(arg_node);
}

node *TCparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCparameters");

    node *entry = PARAMETERS_SYMBOLTABLEENTRY(arg_node);

    DBUG_RETURN(arg_node);
}

/* Traversal start function */
node *TCdoTypeChecking(node *syntaxtree)
{
    DBUG_ENTER("TCdoTypeChecking");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_tc);                           // push traversal "tc" as defined in ast.xml
    syntaxtree = TRAVdo(syntaxtree, arg_info); // initiate ast traversal
    TRAVpop();                                 // pop current traversal

    CTInote("Traversing for TC done...\n");

    if (INFO_ERRORS(arg_info) != 0)
        CTIabort("Found %i error(s) during type checking. Aborting the compilation.\n", INFO_ERRORS(arg_info));

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
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