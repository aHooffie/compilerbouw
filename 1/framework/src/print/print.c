
/**
 * @file print.c
 *
 * Functions needed by print traversal.
 *
 */

/**
 * @defgroup print Print Functions.
 *
 * Functions needed by print traversal.
 *
 * @{
 */

#include "print.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"

/* Functions related to INFO structure */
struct INFO
{
    bool firsterror;
};

#define INFO_FIRSTERROR(n) ((n)->firsterror)

static info *MakeInfo()
{
    info *result;

    result = MEMmalloc(sizeof(info));

    INFO_FIRSTERROR(result) = FALSE;

    return result;
}

static info *FreeInfo(info *info)
{
    info = MEMfree(info);

    return info;
}

/* !! MODULE + DECLARATIONS AKA TOP BELOW!! */

/* @brief Prints the given syntaxtree
 * @param syntaxtree a node structure
 * @return the unchanged nodestructure */
node *PRTdoPrint(node *syntaxtree)
{
    info *info;

    DBUG_ENTER("PRTdoPrint");

    DBUG_ASSERT((syntaxtree != NULL), "PRTdoPrint called with empty syntaxtree");

    printf("\n\n------------------------------\n\n");

    info = MakeInfo();

    TRAVpush(TR_prt);

    syntaxtree = TRAVdo(syntaxtree, info);

    TRAVpop();

    info = FreeInfo(info);

    printf("\n------------------------------\n\n");

    DBUG_RETURN(syntaxtree);
}

/* Adds module node - changed after assignment 2! */
node *PRTmodule(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTmodule");

    MODULE_DECLARATIONS(arg_node) = TRAVdo(MODULE_DECLARATIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the declarations. */
node *PRTdeclarations(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTdeclaration");

    DECLARATIONS_DECLARATION(arg_node) = TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);
    DECLARATIONS_NEXT(arg_node) = TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the function definitions and declarations. */
node *PRTfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunction");

    FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    FUNCTION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the parameters of a function. */
node *PRTparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTparameters");

    PARAMETERS_NEXT(arg_node) = TRAVdo(PARAMETERS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the body of a function. */
node *PRTfunctionbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctionbody");

    FUNCTIONBODY_STMTS(arg_node) = TRAVdo(FUNCTIONBODY_STMTS(arg_node), arg_info);
    FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVdo(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the global declarations (empty!). */
node *PRTglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTglobaldec");
    DBUG_RETURN(arg_node);
}

/* Prints the global definiions. */
node *PRTglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTglobaldef");

    if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    {
        GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* !! STATEMENTS BELOW !!*/

/* @fn PRTstmts
 * @brief Prints the node and its sons/attributes.
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTstmts(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTstmts");

    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);
    STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* @fn PRTassign
 * @brief Prints the node and its sons/attributes
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTassign(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTassign");

    if (ASSIGN_LET(arg_node) != NULL)
    {
        ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);
        printf(" = ");
    }

    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);
    printf(";\n");

    DBUG_RETURN(arg_node);
}

/* !! FUNCTION ELEMENTS BELOW !! */

/* Prints the if-node. */
node *PRTif(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTif");

    if (IF_ELSE(arg_node) != NULL)
    {
        IF_ELSE(arg_node) = TRAVdo(IF_ELSE(arg_node), arg_info);
    }

    IF_STMTS(arg_node) = TRAVdo(IF_STMTS(arg_node), arg_info);
    IF_EXPR(arg_node) = TRAVdo(IF_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the else-node. */
node *PRTelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTelse");

    ELSE_STMTS(arg_node) = TRAVdo(ELSE_STMTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the while-node. */
node *PRTwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTwhile");

    WHILE_STMTS(arg_node) = TRAVdo(WHILE_STMTS(arg_node), arg_info);
    WHILE_EXPR(arg_node) = TRAVdo(WHILE_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the return-node. */
node *PRTreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTreturn");

    RETURN_EXPR(arg_node) = TRAVdo(RETURN_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the for-loop-node. */
node *PRTfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfor");

    FOR_ASSIGN(arg_node) = TRAVdo(FOR_ASSIGN(arg_node), arg_info);
    FOR_EXPR(arg_node) = TRAVdo(FOR_EXPR(arg_node), arg_info);
    FOR_STMTS(arg_node) = TRAVdo(FOR_STMTS(arg_node), arg_info);

    if (FOR_EXPROPT(arg_node) != NULL)
    {
        FOR_EXPROPT(arg_node) = TRAVdo(FOR_EXPROPT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the do-while-node. */
node *PRTdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTdowhile");

    DOWHILE_STMTS(arg_node) = TRAVdo(DOWHILE_STMTS(arg_node), arg_info);
    DOWHILE_WHILE(arg_node) = TRAVdo(DOWHILE_WHILE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the function call statement. */
node *PRTfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctioncallstmt");

    if (FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) != NULL)
    {
        FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the node handler for nodeset expressions. */
node *PRTexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTexpressions");

    EXPRESSIONS_EXPR(arg_node) = TRAVdo(EXPRESSIONS_EXPR(arg_node), arg_info);
    EXPRESSIONS_NEXT(arg_node) = TRAVopt(EXPRESSIONS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the local functions. */
node *PRTlocalfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTlocalfunction");

    LOCALFUNCTION_NEXT(arg_node) = TRAVdo(LOCALFUNCTION_NEXT(arg_node), arg_info);
    LOCALFUNCTION_PARAMETERS(arg_node) = TRAVdo(LOCALFUNCTION_PARAMETERS(arg_node), arg_info);
    LOCALFUNCTION_FUNCTIONBODY(arg_node) = TRAVdo(LOCALFUNCTION_FUNCTIONBODY(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* !! EXPRESSIONS ARE BELOW !! */

/* @fn PRTbinop
 * @brief Prints the node and its sons/attributes
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTbinop(node *arg_node, info *arg_info)
{
    char *tmp;

    DBUG_ENTER("PRTbinop");

    printf("( ");

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);

    switch (BINOP_OP(arg_node))
    {
    case BO_add:
        tmp = "+";
        break;
    case BO_sub:
        tmp = "-";
        break;
    case BO_mul:
        tmp = "*";
        break;
    case BO_div:
        tmp = "/";
        break;
    case BO_mod:
        tmp = "%";
        break;
    case BO_lt:
        tmp = "<";
        break;
    case BO_le:
        tmp = "<=";
        break;
    case BO_gt:
        tmp = ">";
        break;
    case BO_ge:
        tmp = ">=";
        break;
    case BO_eq:
        tmp = "==";
        break;
    case BO_ne:
        tmp = "!=";
        break;
    case BO_or:
        tmp = "||";
        break;
    case BO_and:
        tmp = "&&";
        break;
    case BO_unknown:
        DBUG_ASSERT(0, "unknown binop detected!");
    }

    printf(" %s ", tmp);

    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    printf(")");

    DBUG_RETURN(arg_node);
}

/* @fn PRTfloat
 * @brief Prints the node and its sons/attributes
 * @param arg_node Float node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTfloat(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfloat");

    printf("%f", FLOAT_VALUE(arg_node));

    DBUG_RETURN(arg_node);
}

/* @fn PRTnum
 * @brief Prints the node and its sons/attributes
 * @param arg_node Num node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTnum");

    printf("%i", NUM_VALUE(arg_node));

    DBUG_RETURN(arg_node);
}

/* @fn PRTboolean
 * @brief Prints the node and its sons/attributes
 * @param arg_node Boolean node to process
 * @param arg_info pointer to info structure
 * @return processed node. */
node *PRTbool(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTbool");

    if (BOOL_VALUE(arg_node))
    {
        printf("true");
    }
    else
    {
        printf("false");
    }

    DBUG_RETURN(arg_node);
}

/* @fn PRTvar
 * @brief Prints the node and its sons/attributes
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 * @return processed node. */
node *PRTvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTvar");

    printf("%s", VAR_NAME(arg_node));

    DBUG_RETURN(arg_node);
}

/* @fn PRTvarlet
 * @brief Prints the node and its sons/attributes
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 * @return processed node. */
node *PRTvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTvarlet");

    VARLET_NEXT(arg_node) = TRAVdo(VARLET_NEXT(arg_node), arg_info);
    VARLET_ASSIGN(arg_node) = TRAVdo(VARLET_ASSIGN(arg_node), arg_info);

    printf("%s", VARLET_NAME(arg_node));

    DBUG_RETURN(arg_node);
}

/* Prints the monop node. */
node *PRTmonop(node *arg_node, info *arg_info)
{
    char *tmp;

    DBUG_ENTER("PRTmonop");
    MONOP_EXPR(arg_node) = TRAVdo(MONOP_EXPR(arg_node), arg_info);
    switch (MONOP_OP(arg_node))
    {
    case MO_neg:
        tmp = "-";
        break;
    case MO_not:
        tmp = "!";
        break;
    case MO_unknown:
        DBUG_ASSERT(0, "unknown monop detected!");
    }

    printf("%s", tmp);
    DBUG_RETURN(arg_node);
}

/* Prints the function call expression. */
node *PRTfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctioncallstmt");

    if (FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) != NULL)
    {
        FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the cast node. */
node *PRTcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTcast");
    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);
    DBUG_RETURN(arg_node);
}

/* MISCELLANEOUS !! */

/* @fn PRTsymboltableentry
 * @brief Prints the node and its sons/attributes
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 * @return processed node */

node *PRTsymboltableentry(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTsymboltableentry");

    DBUG_RETURN(arg_node);
}

/* @fn PRTerror
 * @brief Prints the node and its sons/attributes
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTerror(node *arg_node, info *arg_info)
{
    bool first_error;

    DBUG_ENTER("PRTerror");

    if (NODE_ERROR(arg_node) != NULL)
    {
        NODE_ERROR(arg_node) = TRAVdo(NODE_ERROR(arg_node), arg_info);
    }

    first_error = INFO_FIRSTERROR(arg_info);

    if ((global.outfile != NULL) && (ERROR_ANYPHASE(arg_node) == global.compiler_anyphase))
    {

        if (first_error)
        {
            printf("\n/******* BEGIN TREE CORRUPTION ********\n");
            INFO_FIRSTERROR(arg_info) = FALSE;
        }

        printf("%s\n", ERROR_MESSAGE(arg_node));

        if (ERROR_NEXT(arg_node) != NULL)
        {
            TRAVopt(ERROR_NEXT(arg_node), arg_info);
        }

        if (first_error)
        {
            printf("********  END TREE CORRUPTION  *******/\n");
            INFO_FIRSTERROR(arg_info) = TRUE;
        }
    }

    DBUG_RETURN(arg_node);
}

/**
 * @}
 */