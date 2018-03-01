
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
node *PRTprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTprogram");

    PROGRAM_DECLARATIONS(arg_node) = TRAVdo(PROGRAM_DECLARATIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the declarations. */
node *PRTdeclarations(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTdeclarations");

    DECLARATIONS_DECLARATION(arg_node) = TRAVdo(DECLARATIONS_DECLARATION(arg_node), arg_info);
    if (DECLARATIONS_NEXT(arg_node) != NULL)
        DECLARATIONS_NEXT(arg_node) = TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the global declarations (empty!). */
node *PRTglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTglobaldec");
    printf("extern ");

    switch (GLOBALDEC_TYPE(arg_node))
    {
    case T_int:
        printf("int ");
        break;
    case T_float:
        printf("float ");
        break;
    case T_bool:
        printf("bool ");
        break;
    case T_void:
        printf("unknown type detected!");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown return type detected!");
        break;
    }
    printf("%s;\n", GLOBALDEC_NAME(arg_node));
    if (GLOBALDEC_DIMENSIONS(arg_node) != NULL)
        GLOBALDEC_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEC_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the global definiions. */
node *PRTglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTglobaldef");
    if (GLOBALDEF_ISEXPORT(arg_node) == TRUE)
        printf("export ");

    switch (GLOBALDEF_TYPE(arg_node))
    {
    case T_int:
        printf("int ");
        break;
    case T_float:
        printf("float ");
        break;
    case T_bool:
        printf("bool ");
        break;
    case T_void:
        printf("void ");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown return type detected!");
        break;
    }

    printf("%s", GLOBALDEF_NAME(arg_node));

    if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    {
        printf(" = ");
        GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);
    }

    if (GLOBALDEF_DIMENSIONS(arg_node) != NULL)
        GLOBALDEF_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEF_DIMENSIONS(arg_node), arg_info);

    printf(";\n");
    DBUG_RETURN(arg_node);
}

/* Prints the function definitions and declarations. */
node *PRTfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunction");
    if (FUNCTION_FUNCTIONBODY(arg_node) == NULL)
        printf("extern ");

    if (FUNCTION_ISEXPORT(arg_node) == TRUE)
        printf("export ");

    switch (FUNCTION_TYPE(arg_node))
    {
    case T_int:
        printf("int ");
        break;
    case T_float:
        printf("float ");
        break;
    case T_bool:
        printf("bool ");
        break;
    case T_void:
        printf("void ");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown return type detected!");
        break;
    }

    printf("%s(", FUNCTION_NAME(arg_node));

    if (FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);

    printf(")");

    if (FUNCTION_FUNCTIONBODY(arg_node) != NULL)
    {
        printf("\n{\n\t");
        FUNCTION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTION_FUNCTIONBODY(arg_node), arg_info);
        printf("}\n");
    }

    if (FUNCTION_FUNCTIONBODY(arg_node) == NULL)
        printf(";");

    DBUG_RETURN(arg_node);
}

/* Prints the parameters of a function. */
node *PRTparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTparameters");

    if (PARAMETERS_NEXT(arg_node) != NULL)
    {
        printf(", ");
        PARAMETERS_NEXT(arg_node) = TRAVdo(PARAMETERS_NEXT(arg_node), arg_info);
    }
    if (PARAMETERS_DIMENSIONS(arg_node) != NULL)
        PARAMETERS_DIMENSIONS(arg_node) = TRAVdo(PARAMETERS_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the body of a function. */
node *PRTfunctionbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctionbody");

    if (FUNCTIONBODY_VARDECLARATIONS(arg_node) != NULL)
        FUNCTIONBODY_VARDECLARATIONS(arg_node) =
            TRAVdo(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);

    if (FUNCTIONBODY_STMTS(arg_node) != NULL)
        FUNCTIONBODY_STMTS(arg_node) = TRAVdo(FUNCTIONBODY_STMTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *PRTvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTvardeclaration");
    switch (VARDECLARATION_TYPE(arg_node))
    {
    case T_int:
        printf("int ");
        break;
    case T_float:
        printf("float ");
        break;
    case T_bool:
        printf("bool ");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown type detected!");
        break;
    case T_void:
        DBUG_ASSERT(0, "vardeclaration type cannot be void!");
        break;
    }
    printf("%s", VARDECLARATION_NAME(arg_node));

    if (VARDECLARATION_INIT(arg_node) != NULL)
    {
        printf(" = ");
        VARDECLARATION_INIT(arg_node) = TRAVdo(VARDECLARATION_INIT(arg_node), arg_info);
        printf(";\n");
    }

    if (VARDECLARATION_DIMENSIONS(arg_node) != NULL)
        VARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(VARDECLARATION_DIMENSIONS(arg_node), arg_info);

    if (VARDECLARATION_NEXT(arg_node) != NULL)
        VARDECLARATION_NEXT(arg_node) = TRAVdo(VARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* @fn PRTstmts
 * @brief Prints the node and its sons/attributes.
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTstmts(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTstmts");
    printf("\t");

    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);
    if (STMTS_NEXT(arg_node) != NULL)
    {
        STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);
    }
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

    printf("(");
    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);
    printf(")");

    printf(";\n");

    DBUG_RETURN(arg_node);
}

/* Prints the if-node. */
node *PRTifelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTif");

    printf("(if (");
    IFELSE_CONDITION(arg_node) = TRAVdo(IFELSE_CONDITION(arg_node), arg_info);
    printf(") \n\t{\n");

    if (IFELSE_BLOCK(arg_node) != NULL)
    {
        IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);
        printf("\t}\n");
    }
    if (IFELSE_ELSE(arg_node) != NULL)
    {
        printf("\t else \n");
        IFELSE_ELSE(arg_node) = TRAVdo(IFELSE_ELSE(arg_node), arg_info);
        printf("\n");
    }
    printf("\t)\n");

    DBUG_RETURN(arg_node);
}

/* Prints the while-node. */
node *PRTwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTwhile");
    printf("while (");
    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);
    printf(") \n\t{\n");

    if (WHILE_BLOCK(arg_node) != NULL)
        WHILE_BLOCK(arg_node) = TRAVdo(WHILE_BLOCK(arg_node), arg_info);
    printf("\t}\n");

    DBUG_RETURN(arg_node);
}

/* Prints the return-node. */
node *PRTreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTreturn");
    printf("return");
    if (RETURN_EXPR(arg_node) != NULL)
    {
        printf(" ");
        RETURN_EXPR(arg_node) = TRAVdo(RETURN_EXPR(arg_node), arg_info);
    }
    printf(";\n");

    DBUG_RETURN(arg_node);
}

/* Prints the for-loop-node. */
node *PRTfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfor");
    printf("for (int %s = ", FOR_INITVAR(arg_node));
    FOR_START(arg_node) = TRAVdo(FOR_START(arg_node), arg_info);
    printf(", ");
    FOR_STOP(arg_node) = TRAVdo(FOR_STOP(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL)
    {
        printf(", ");
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
    }
    printf(") \n");
    if (FOR_BLOCK(arg_node) != NULL)
    {
        printf("\t{\n");
        FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);
        printf("\t}\n");
    }

    DBUG_RETURN(arg_node);
}

/* Prints the do-while-node. */
node *PRTdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTdowhile");

    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);

    if (DOWHILE_BLOCK(arg_node) != NULL)
        DOWHILE_BLOCK(arg_node) = TRAVdo(DOWHILE_BLOCK(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the function call statement. */
node *PRTfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctioncallstmt");
    printf("%s(", FUNCTIONCALLSTMT_NAME(arg_node));

    if (FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) != NULL)
        FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) =
            TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);
    printf(");\n");

    DBUG_RETURN(arg_node);
}

/* Prints the node handler for nodeset expressions. */
node *PRTexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTexpressions");

    if (EXPRESSIONS_EXPR(arg_node) != NULL)
    {
        EXPRESSIONS_EXPR(arg_node) =
            TRAVdo(EXPRESSIONS_EXPR(arg_node), arg_info);
    }
    if (EXPRESSIONS_NEXT(arg_node) != NULL)
    {
        printf(", ");
        EXPRESSIONS_NEXT(arg_node) =
            TRAVopt(EXPRESSIONS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the local functions. */
node *PRTlocalfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTlocalfunction");

    if (LOCALFUNCTION_NEXT(arg_node) != NULL)
        LOCALFUNCTION_NEXT(arg_node) =
            TRAVdo(LOCALFUNCTION_NEXT(arg_node), arg_info);
    if (LOCALFUNCTION_FUNCTION(arg_node) != NULL)
        LOCALFUNCTION_FUNCTION(arg_node) =
            TRAVdo(LOCALFUNCTION_FUNCTION(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* @fn PRTbinop
 * @brief Prints the node and its sons/attributes
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 * @return processed node */
node *PRTbinop(node *arg_node, info *arg_info)
{
    char *tmp;

    DBUG_ENTER("PRTbinop");

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
        printf("true");
    else
        printf("false");

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
    if (VAR_INDICES(arg_node) != NULL)
    {
        printf("[");
        VAR_INDICES(arg_node) = TRAVdo(VAR_INDICES(arg_node), arg_info);
        printf("]");
    }

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

    if (VARLET_NEXT(arg_node) != NULL)
        VARLET_NEXT(arg_node) = TRAVdo(VARLET_NEXT(arg_node), arg_info);
    if (VARLET_INDICES(arg_node) != NULL)
    {
        printf("[");
        VARLET_INDICES(arg_node) = TRAVdo(VARLET_INDICES(arg_node), arg_info);
        printf("]");
    }

    printf("%s", VARLET_NAME(arg_node));

    DBUG_RETURN(arg_node);
}

/* Prints the monop node. */
node *PRTmonop(node *arg_node, info *arg_info)
{
    char *tmp;

    DBUG_ENTER("PRTmonop");

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
    MONOP_EXPR(arg_node) = TRAVdo(MONOP_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the function call expression. */
node *PRTfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctioncallstmt");

    printf("%s(", FUNCTIONCALLEXPR_NAME(arg_node));

    if (FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) != NULL)
        FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) =
            TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS(arg_node), arg_info);
    printf(")");

    DBUG_RETURN(arg_node);
}

/* Prints the cast node. */
node *PRTcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTcast");

    switch (CAST_TYPE(arg_node))
    {
    case T_int:
        printf("(int) ");
        break;
    case T_float:
        printf("(float) ");
        break;
    case T_bool:
        printf("(bool) ");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown type detected!");
        break;
    case T_void:
        DBUG_ASSERT(0, "cast type cannot be void!");
        break;
    }

    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);
    DBUG_RETURN(arg_node);
}

/* Prints the arrayexpression node. */
node *PRTarrayexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTarrayexpr");
    ARRAYEXPR_EXPRS(arg_node) = TRAVdo(ARRAYEXPR_EXPRS(arg_node), arg_info);
    DBUG_RETURN(arg_node);
}

/* @fn PRTsymboltableentry
 * @brief Prints the node and its sons/attributes
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 * @return processed node */

node *PRTsymboltableentry(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTsymboltableentry");
    // DOES NOTHING YET!
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
        NODE_ERROR(arg_node) = TRAVdo(NODE_ERROR(arg_node), arg_info);

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
            TRAVopt(ERROR_NEXT(arg_node), arg_info);

        if (first_error)
        {
            printf("********  END TREE CORRUPTION  *******/\n");
            INFO_FIRSTERROR(arg_info) = TRUE;
        }
    }

    DBUG_RETURN(arg_node);
}

/* Prints the arrayexpression node. */
node *PRTids(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTids");

    if (IDS_NEXT(arg_node) != NULL)
        IDS_NEXT(arg_node) = TRAVdo(IDS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/**
 * @}
 */