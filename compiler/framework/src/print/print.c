/*
 * Module: print.c
 * Description: prints the contents of nodes. 
 * Authors: Aynel Gul & Andrea van den Hooff
 * Arrays implemented.
 */

#include "print.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"
#include "myglobals.h"

/* Functions related to INFO structure */
struct INFO
{
    bool firsterror;
    int indent;
};

#define INFO_FIRSTERROR(n) ((n)->firsterror)
#define INFO_INDENT(n) ((n)->indent)

static info *MakeInfo()
{
    info *result;

    result = MEMmalloc(sizeof(info));

    INFO_FIRSTERROR(result) = FALSE;
    INFO_INDENT(result) = 0;

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

/* Adds program node. */
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
    DECLARATIONS_NEXT(arg_node) = TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the global declarations. */
node *PRTglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTglobaldec");
    printf("extern ");

    switch (GLOBALDEC_TYPE(arg_node))
    {
    case T_int:
        printf("int");
        break;
    case T_float:
        printf("float");
        break;
    case T_bool:
        printf("bool");
        break;
    case T_void:
        printf("unknown type detected!");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown return type detected!");
        break;
    }
    if (GLOBALDEC_DIMENSIONS(arg_node) != NULL)
    {
        printf(" [");
        GLOBALDEC_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEC_DIMENSIONS(arg_node), arg_info);
        printf("]");
    }
    printf(" %s;\n", GLOBALDEC_NAME(arg_node));

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

    /* Traverse child nodes. */
    if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    {
        printf(" = ");
        GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);
    }

    GLOBALDEF_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEF_DIMENSIONS(arg_node), arg_info);

    printf(";\n");
    DBUG_RETURN(arg_node);
}

/* Prints the function definitions and declarations. */
node *PRTfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunction");
    if (FUNCTION_ISEXTERN(arg_node) == TRUE)
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
    FUNCTION_PARAMETERS(arg_node) = TRAVopt(FUNCTION_PARAMETERS(arg_node), arg_info);
    printf(")");

    if (FUNCTION_FUNCTIONBODY(arg_node) != NULL)
    {
        printf("\n");
        PRTindent(arg_info);
        printf("{");
        INFO_INDENT(arg_info) += 4;
        printf("\n");

        FUNCTION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

        INFO_INDENT(arg_info) -= 4;
        PRTindent(arg_info);
        printf("}\n");
    }

    if (FUNCTION_FUNCTIONBODY(arg_node) == NULL)
        printf(";\n");

    DBUG_RETURN(arg_node);
}

/* Prints the parameters of a function. */
node *PRTparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTparameters");

    switch (PARAMETERS_TYPE(arg_node))
    {
    case T_int:
        printf("int");
        break;
    case T_float:
        printf("float");
        break;
    case T_bool:
        printf("bool");
        break;
    case T_unknown:
        DBUG_ASSERT(0, "unknown type detected!");
        break;
    case T_void:
        DBUG_ASSERT(0, "parameter type cannot be void!");
        break;
    }

    if (PARAMETERS_DIMENSIONS(arg_node) != NULL)
    {
        printf(" [");
        PARAMETERS_DIMENSIONS(arg_node) = TRAVdo(PARAMETERS_DIMENSIONS(arg_node), arg_info);
        printf("]");
    }
    printf(" %s", PARAMETERS_NAME(arg_node));

    if (PARAMETERS_NEXT(arg_node) != NULL)
    {
        printf(", ");
        PARAMETERS_NEXT(arg_node) = TRAVdo(PARAMETERS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the body of a function. */
node *PRTfunctionbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctionbody");

    /* Traverse child nodes. */
    FUNCTIONBODY_VARDECLARATIONS(arg_node) = TRAVopt(FUNCTIONBODY_VARDECLARATIONS(arg_node), arg_info);
    FUNCTIONBODY_LOCALFUNCTION(arg_node) = TRAVopt(FUNCTIONBODY_LOCALFUNCTION(arg_node), arg_info);
    FUNCTIONBODY_STMTS(arg_node) = TRAVopt(FUNCTIONBODY_STMTS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the variable declarations within a function. */
node *PRTvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTvardeclaration");
    PRTindent(arg_info);

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
        printf("void ");
        break;
    }

    if (VARDECLARATION_DIMENSIONS(arg_node) != NULL)
    {
        printf("[");
        VARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(VARDECLARATION_DIMENSIONS(arg_node), arg_info);
        printf("] ");
    }
    printf("%s", VARDECLARATION_NAME(arg_node));

    if (VARDECLARATION_INIT(arg_node) != NULL)
    {
        printf(" = ");
        if (NODE_TYPE(VARDECLARATION_INIT(arg_node)) == N_arrayexpr)
            printf("[");

        VARDECLARATION_INIT(arg_node) = TRAVdo(VARDECLARATION_INIT(arg_node), arg_info);

        if (NODE_TYPE(VARDECLARATION_INIT(arg_node)) == N_arrayexpr)
            printf("]");

        printf(";\n");
    }
    else
        printf(";\n");

    /* Traverse optional next nodes. */
    VARDECLARATION_NEXT(arg_node) = TRAVopt(VARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the function body. */
node *PRTstmts(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTstmts");

    PRTindent(arg_info);

    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);
    STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Prints the assigning of a variable. */
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

/* Prints the if-node. */
node *PRTifelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTif");

    printf("if (");
    IFELSE_CONDITION(arg_node) = TRAVdo(IFELSE_CONDITION(arg_node), arg_info);

    printf(") \n");
    PRTindent(arg_info);
    printf("{\n");
    INFO_INDENT(arg_info) += 4;

    if (IFELSE_BLOCK(arg_node) != NULL)
    {
        IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);
        INFO_INDENT(arg_info) -= 4;
        PRTindent(arg_info);
        printf("}\n");
    }

    if (IFELSE_ELSE(arg_node) != NULL)
    {
        PRTindent(arg_info);
        printf("else \n");
        PRTindent(arg_info);
        printf("{\n");
        INFO_INDENT(arg_info) += 4;
        IFELSE_ELSE(arg_node) = TRAVdo(IFELSE_ELSE(arg_node), arg_info);
        INFO_INDENT(arg_info) -= 4;

        PRTindent(arg_info);
        printf("}\n");
    }

    DBUG_RETURN(arg_node);
}

/* Prints the while-node. */
node *PRTwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTwhile");
    printf("while (");

    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);

    printf(") \n");
    PRTindent(arg_info);
    printf("{\n");

    if (WHILE_BLOCK(arg_node) != NULL)
    {
        INFO_INDENT(arg_info) += 4;
        WHILE_BLOCK(arg_node) = TRAVopt(WHILE_BLOCK(arg_node), arg_info);
        INFO_INDENT(arg_info) -= 4;
    }

    PRTindent(arg_info);
    printf("}\n");

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
        printf("{\n");
        FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);
        printf("}\n");
    }

    DBUG_RETURN(arg_node);
}

/* Prints the do-while-node. */
node *PRTdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTdowhile");

    printf("do\n");
    PRTindent(arg_info);
    printf("{\n");

    PRTindent(arg_info);
    DOWHILE_BLOCK(arg_node) = TRAVopt(DOWHILE_BLOCK(arg_node), arg_info);
    PRTindent(arg_info);

    printf("}\n");
    PRTindent(arg_info);
    printf("while (");

    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);
    printf(");\n");

    DBUG_RETURN(arg_node);
}

/* Prints the function call statement. */
node *PRTfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfunctioncallstmt");
    printf("%s(", FUNCTIONCALLSTMT_NAME(arg_node));

    FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) = TRAVopt(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);
    printf(");\n");

    DBUG_RETURN(arg_node);
}

/* Prints the node handler for nodeset expressions. */
node *PRTexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTexpressions");

    EXPRESSIONS_EXPR(arg_node) = TRAVopt(EXPRESSIONS_EXPR(arg_node), arg_info);

    if (EXPRESSIONS_NEXT(arg_node) != NULL)
    {
        printf(", ");
        EXPRESSIONS_NEXT(arg_node) = TRAVopt(EXPRESSIONS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the local functions. */
node *PRTlocalfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTlocalfunction");

    PRTindent(arg_info);

    /* Traverse child nodes. */
    LOCALFUNCTION_FUNCTION(arg_node) = TRAVdo(LOCALFUNCTION_FUNCTION(arg_node), arg_info);
    LOCALFUNCTION_NEXT(arg_node) = TRAVopt(LOCALFUNCTION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Print the binop node. */
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

node *PRTternop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTternop");

    TERNOP_CONDITION(arg_node) = TRAVdo(TERNOP_CONDITION(arg_node), arg_info);
    printf(" ? ");
    TERNOP_THEN(arg_node) = TRAVdo(TERNOP_THEN(arg_node), arg_info);
    printf(" : ");
    TERNOP_ELSE(arg_node) = TRAVdo(TERNOP_ELSE(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}
/* Print the float value. */
node *PRTfloat(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTfloat");

    printf("%f", FLOAT_VALUE(arg_node));

    DBUG_RETURN(arg_node);
}

/* Print an integer value. */
node *PRTnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTnum");

    printf("%i", NUM_VALUE(arg_node));

    DBUG_RETURN(arg_node);
}

/* Print a boolean value. */
node *PRTbool(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTbool");

    if (BOOL_VALUE(arg_node))
        printf("true");
    else
        printf("false");

    DBUG_RETURN(arg_node);
}

/* Print a variable name. */
node *PRTvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTvar");

    printf("%s", VAR_NAME(arg_node));
    if (VAR_INDICES(arg_node) != NULL)
    {
        printf("[");
        VAR_INDICES(arg_node) = TRAVdo(VAR_INDICES(arg_node), arg_info);
        printf("]");
    }

    DBUG_RETURN(arg_node);
}

/* Print a variable in assignment possition. */
node *PRTvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTvarlet");

    printf("%s", VARLET_NAME(arg_node));

    if (VARLET_INDICES(arg_node) != NULL)
    {
        printf("[");
        VARLET_INDICES(arg_node) = TRAVdo(VARLET_INDICES(arg_node), arg_info);
        printf("]");
    }

    VARLET_NEXT(arg_node) = TRAVopt(VARLET_NEXT(arg_node), arg_info);

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

    /* Traverse parameter nodes. */
    FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) = TRAVopt(FUNCTIONCALLEXPR_EXPRESSIONS(arg_node), arg_info);
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
    ARRAYEXPR_EXPR(arg_node) = TRAVdo(ARRAYEXPR_EXPR(arg_node), arg_info);

    if (ARRAYEXPR_NEXT(arg_node) != NULL)
    {
        printf(", ");
        ARRAYEXPR_NEXT(arg_node) = TRAVdo(ARRAYEXPR_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the array expression node. */
node *PRTids(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTids");

    printf("%s", IDS_NAME(arg_node));

    if (IDS_NEXT(arg_node) != NULL)
    {
        printf(", ");
        IDS_NEXT(arg_node) = TRAVdo(IDS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

/* Prints the right amount of indentation. */
void PRTindent(info *arg_info)
{
    int indent = INFO_INDENT(arg_info);
    for (int i = 0; i < indent; i++)
        printf(" ");
}

/* Not implemented. */
node *PRTsymboltableentry(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTsymboltableentry");
    DBUG_RETURN(arg_node);
}

/* Not implemented. */
node *PRTsymboltable(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTsymboltable");
    DBUG_RETURN(arg_node);
}

/* Not implemented here. */
node *PRTinstructions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("PRTinstructions");
    DBUG_RETURN(arg_node);
}

/* Prints an error node. */
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