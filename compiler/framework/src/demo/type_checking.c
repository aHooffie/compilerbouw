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
/* Declarations. */
node *TCglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCglobaldec");

    if (GLOBALDEC_TYPE(arg_node) != T_bool &&
        GLOBALDEC_TYPE(arg_node) != T_int &&
        GLOBALDEC_TYPE(arg_node) != T_float)
        typeError(arg_info, arg_node, "The global declaration is not of a basic type.");

    if (GLOBALDEC_DIMENSIONS(arg_node) != NULL)
        GLOBALDEC_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEC_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCglobaldef");

    if (GLOBALDEF_DIMENSIONS(arg_node) != NULL)
        GLOBALDEF_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEF_DIMENSIONS(arg_node), arg_info);

    if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    {
        GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);
        if (INFO_TYPE(arg_info) != GLOBALDEF_TYPE(arg_node))
            typeError(arg_info, arg_node, "Global definition.");
    }
    DBUG_RETURN(arg_node);
}

// MOET NOG
node *TCparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCparameters");

    // Check if params are allowed types.
    // Check if params are corresponding with function.

    DBUG_RETURN(arg_node);
}

// MOET NOG
node *TCfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunction");

    FUNCTION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTION_FUNCTIONBODY(arg_node));
    FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node));

    //     // Check if dimensions are integers
    //      // Check if return type(s) corresponds.

    DBUG_RETURN(arg_node);
}

// MOET NOG
node *TCfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunctioncallstmt");
    DBUG_RETURN(arg_node);
}

/* Functioncallexpr */
node *TCfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCfunctioncallexpr");
    DBUG_RETURN(arg_node);
}

/* Statements (for, if, while, dowhile, return) . */
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

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    DBUG_RETURN(arg_node);
}

node *TCifelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCifelse");

    /* Check condition. */
    IFELSE_CONDITION(arg_node) = TRAVdo(IFELSE_CONDITION(arg_node), arg_info);
    if (basictypeCheck(arg_info) == FALSE)
        typeError(arg_info, arg_node, "If condition is not a basic type.");

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    /* Traverse through block of statements.*/
    IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);

    /* Traverse through else block of statements.*/
    if (IFELSE_ELSE(arg_node) != NULL)
        IFELSE_ELSE(arg_node) = TRAVdo(IFELSE_ELSE(arg_node), arg_info);

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    DBUG_RETURN(arg_node);
}

node *TCwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCwhile");

    /* Check condition. */
    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    if (basictypeCheck(arg_info) == FALSE)
        typeError(arg_info, arg_node, "While condition is not a basic type.");

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;

    /* Traverse through block of statements.*/
    WHILE_BLOCK(arg_node) = TRAVdo(WHILE_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *TCdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCdowhile");

    /* Traverse through block of statements.*/
    DOWHILE_BLOCK(arg_node) = TRAVdo(DOWHILE_BLOCK(arg_node), arg_info);

    /* Check condition. */
    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);

    if (basictypeCheck(arg_info) == FALSE)
        typeError(arg_info, arg_node, "The dowhile condition is not a basic type.");

    /* Reset. */
    INFO_TYPE(arg_info) = T_unknown;
    DBUG_RETURN(arg_node);
}

node *TCreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCwhile");

    /* Check condition. */
    if (RETURN_EXPR(arg_node) != NULL)
        RETURN_EXPR(arg_node) = TRAVdo(RETURN_EXPR(arg_node), arg_info);

    if (basictypeCheck(arg_info) == FALSE)
        typeError(arg_info, arg_node, "Return expression is not a basic type.");

    DBUG_RETURN(arg_node);
}

/* Assign. */
node *TCassign(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCassign");

    /* Traverse left side of an assignment. */
    if (ASSIGN_LET(arg_node) != NULL)
        ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);

    type t = INFO_TYPE(arg_info);

    /* Traverse right side and compare. */
    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    if (t != INFO_TYPE(arg_info))
        typeError(arg_info, arg_node, "Assigning types failed. Not same types!");

    /* Reset. */
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

    /* - Can be either a float or an int, ! can be all three basic types. */
    switch (MONOP_OP(arg_node))
    {
    case MO_neg:
        if (INFO_TYPE(arg_info) != T_int && INFO_TYPE(arg_info) != T_float)
            typeError(arg_info, arg_node, "Wrong Monop types. ");
        break;
    case MO_not:
        if (basictypeCheck(arg_info) == FALSE)
            typeError(arg_info, arg_node, "Wrong Monop types. ");
        break;
    default:
        typeError(arg_info, arg_node, "Conflicting type + monop found.");
        break;
    }

    DBUG_RETURN(arg_node);
}

/* Binop - werkt niet helemaal goed. */
node *TCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("TCbinop");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    nodetype left = NODE_TYPE(BINOP_LEFT(arg_node));
    nodetype right = NODE_TYPE(BINOP_RIGHT(arg_node));
    // == fout! Moet travdo zijn in links, dat opslaan in arg_info, en dan travdo right en kijken of dat ook kan.
    // printf("Type: %s & Type: %s\n", NodetypetoString(BINOP_LEFT(arg_node)), NodetypetoString(BINOP_RIGHT(arg_node)));

    switch (BINOP_OP(arg_node))
    {
        /* Modulo; can only be done with integers. */
    case BO_mod:
        if (NODE_TYPE(BINOP_LEFT(arg_node)) != N_num || NODE_TYPE(BINOP_RIGHT(arg_node)) != N_num)
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
                typeError(arg_info, arg_node, "Types are not matching..");
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
            if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))) == T_bool ||
                SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_RIGHT(arg_node))) == T_bool)
                typeError(arg_info, arg_node, "Bools cannot have arithmetic operations.");
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

/* Function to call when a typecheck error arises. */
void typeError(info *arg_info, node *arg_node, char *message)
{
    CTInote("! Error on line %i, col %i. %s", NODE_LINE(arg_node), NODE_COL(arg_node), message);
    INFO_ERRORS(arg_info) += 1;
    INFO_TYPE(arg_info) = T_unknown;
}

/* Function to call when a typecheck error arises. */
bool basictypeCheck(info *arg_info)
{
    if (INFO_TYPE(arg_info) != T_bool &&
        INFO_TYPE(arg_info) != T_int &&
        INFO_TYPE(arg_info) != T_float)
        return FALSE;
    return TRUE;
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