/*
 * Module: Traverse and create / look for declarations in a symbol table.
 * Prefix: AS
 */

/* 

TO DO: Make sure that your compiler appropriately prints the symbol table, for instance as a structured
comment in the beginning of the function body or preceding the entire function denition.

*/

#include "add_symboltables.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"

#include "memory.h"
#include "ctinfo.h"

/* INFO structure */
struct INFO
{
    node *stack;
    node *og;
    int size;
    int errors;
    int FUNCTIONCOUNT;
    int count;
};

/* struct macros */
#define INFO_STACK(n) ((n)->stack)
#define INFO_OG(n) ((n)->og)
#define INFO_SIZE(n) ((n)->size)
#define INFO_ERRORS(n) ((n)->errors)
#define INFO_FUNCTIONCOUNT(n) ((n)->FUNCTIONCOUNT)
#define INFO_COUNT(n) ((n)->count)

/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_STACK(result) = NULL;
    INFO_SIZE(result) = 0;
    INFO_ERRORS(result) = 0;
    INFO_FUNCTIONCOUNT(result) = 0;

    DBUG_RETURN(result);
}
static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/* Found nodes that should write to a global symbol table entry  */
node *ASprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASprogram");

    /* Create a symbol table node for the global scoped variables and functions. */
    node *globals = TBmakeSymboltable(NULL, NULL);
    stackPush(globals, arg_info);
    PROGRAM_SYMBOLTABLE(arg_node) = globals;

    if (stackEmpty(arg_info) == TRUE)
    {
        stError(arg_info, arg_node, "Something went wrong with creating a symboltable when opening the program.", NULL);
        return NULL;
    }

    /* Continue with traversing in child nodes. */
    PROGRAM_DECLARATIONS(arg_node) = TRAVdo(PROGRAM_DECLARATIONS(arg_node), arg_info);

    /* Print the symbol table. */
    CTInote("************************************ \n Global symboltable. Scope level: %i. \n************************************", INFO_SIZE(arg_info) - 1);
    node *entry = SYMBOLTABLE_NEXT(INFO_STACK(arg_info));
    while (entry != NULL)
    {
        CTInote("* Name: %s. Type: %s.", SYMBOLTABLEENTRY_NAME(entry), TypetoString(SYMBOLTABLEENTRY_TYPE(entry)));
        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }
    CTInote("************************************");

    /* Remove the linked list at the end of the traversal. */
    stackPop(arg_info);
    if (stackEmpty(arg_info) == FALSE)
    {
        CTInote("Something went wrong. Stack wasn't empty at the end of traversal.");
        INFO_ERRORS(arg_info) += 1;
    }

    DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASglobaldec");
    char *name = GLOBALDEC_NAME(arg_node);

    /* Found a globaldec. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.\n", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the globaldec into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, GLOBALDEC_TYPE(arg_node), INFO_SIZE(arg_info) - 1, NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        GLOBALDEC_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }
    /* Continue with traversing in optional child nodes. */
    GLOBALDEC_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEC_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASglobaldef");

    char *name = GLOBALDEF_NAME(arg_node);

    /* Found globaldef. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the globaldef into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, GLOBALDEF_TYPE(arg_node), INFO_SIZE(arg_info) - 1, NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        GLOBALDEF_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

    /* Continue with traversing in optional child nodes. */
    GLOBALDEF_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEF_DIMENSIONS(arg_node), arg_info);
    GLOBALDEF_ASSIGN(arg_node) = TRAVopt(GLOBALDEF_ASSIGN(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Put a link between return type and its function. */
node *ASreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASreturn");

    /* Find the original function declaration in the scope above. */
    node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    node *original = travList(SYMBOLTABLE_NEXT(symboltable));

    RETURN_SYMBOLTABLEENTRY(arg_node) = original;

    if (RETURN_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "Return type is not declared", NULL);

    /* Continue with traversing in optional child nodes. */
    RETURN_EXPR(arg_node) = TRAVopt(RETURN_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Found nodes that should write or read to a function symbol table entry  */
node *ASfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunction");
    char *name = FUNCTION_NAME(arg_node);

    /* Found function. Check if there already is one with the same name in global ST. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, FUNCTION_TYPE(arg_node), INFO_SIZE(arg_info) - 1, NULL);
        SYMBOLTABLEENTRY_ORIGINAL(newEntry) = arg_node;
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        FUNCTION_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

    /* Create new symbol table and add it to the stack. */
    node *functionSymboltable = TBmakeSymboltable(NULL, INFO_STACK(arg_info));
    stackPush(functionSymboltable, arg_info);

    if (stackEmpty(arg_info) == TRUE)
        stError(arg_info, arg_node, ": something went wrong with creating a symboltable in this function.", name);

    /* Update function administration. */
    FUNCTION_SYMBOLTABLE(arg_node) = functionSymboltable;

    /* Continue with traversing in child nodes. */
    FUNCTION_PARAMETERS(arg_node) = TRAVopt(FUNCTION_PARAMETERS(arg_node), arg_info);
    FUNCTION_FUNCTIONBODY(arg_node) = TRAVopt(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    /* Print symboltable of current function. */
    CTInote("************************************ \n Function %s's symboltable. Scope level: %i. \n************************************", name, INFO_SIZE(arg_info) - 1);
    node *entry = SYMBOLTABLE_NEXT(INFO_STACK(arg_info));
    while (entry != NULL)
    {
        CTInote("* Name: %s. Type: %s.", SYMBOLTABLEENTRY_NAME(entry), TypetoString(SYMBOLTABLEENTRY_TYPE(entry)));
        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }
    CTInote("************************************");

    /* Remove the linked list at the end of the traversal. */
    stackPop(arg_info);

    DBUG_RETURN(arg_node);
}

node *ASparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASparameters");
    char *name = PARAMETERS_NAME(arg_node);

    /* Found parameter. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, PARAMETERS_TYPE(arg_node), INFO_SIZE(arg_info) - 1, NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        /* Add links to the symbol table entry & to the function definition table entry. */
        node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
        node *original = travList(SYMBOLTABLE_NEXT(symboltable));

        PARAMETERS_PARAMETERSYMBOLTABLEENTRY(arg_node) = newEntry;
        PARAMETERS_FUNCTIONSYMBOLTABLEENTRY(arg_node) = original;
    }

    /* Continue with traversing in child nodes. */
    PARAMETERS_DIMENSIONS(arg_node) = TRAVopt(PARAMETERS_DIMENSIONS(arg_node), arg_info);
    PARAMETERS_NEXT(arg_node) = TRAVopt(PARAMETERS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASexpressions");

    /* Count parameters in fundef. */
    if (INFO_COUNT(arg_info) == 0)
    {
        node *param = FUNCTION_PARAMETERS(INFO_OG(arg_info));
        if (FUNCTION_PARAMETERS(INFO_OG(arg_info)) != NULL)
            INFO_FUNCTIONCOUNT(arg_info) += 1;
        while (PARAMETERS_NEXT(param) != NULL)
        {
            INFO_FUNCTIONCOUNT(arg_info) += 1;
            param = PARAMETERS_NEXT(param);
        }
    }

    if (EXPRESSIONS_EXPR(arg_node) != NULL)
    {
        INFO_COUNT(arg_info) += 1;
        EXPRESSIONS_EXPR(arg_node) = TRAVdo(EXPRESSIONS_EXPR(arg_node), arg_info);
    }

    /* Continue with traversing in optional child nodes. */
    EXPRESSIONS_NEXT(arg_node) = TRAVopt(EXPRESSIONS_NEXT(arg_node), arg_info);

    if (EXPRESSIONS_NEXT(arg_node) == NULL)
    {
        if (INFO_COUNT(arg_info) != INFO_FUNCTIONCOUNT(arg_info))
        {
            stError(arg_info, arg_node, "The amount of parameters didn't match the function definition.", NULL);
            CTInote("> Expected %i, but found %i parameters.", INFO_FUNCTIONCOUNT(arg_info), INFO_COUNT(arg_info));
        }
    }

    INFO_COUNT(arg_info) = 0;
    INFO_FUNCTIONCOUNT(arg_info) = 0;

    DBUG_RETURN(arg_node);
}

node *ASvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvardeclaration");

    char *name = VARDECLARATION_NAME(arg_node);

    /* Found vardeclaration. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the globaldef into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, VARDECLARATION_TYPE(arg_node), INFO_SIZE(arg_info) - 1, NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        VARDECLARATION_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

    /* Continue with traversing in optional child nodes. */
    VARDECLARATION_DIMENSIONS(arg_node) = TRAVopt(VARDECLARATION_DIMENSIONS(arg_node), NULL);
    VARDECLARATION_INIT(arg_node) = TRAVopt(VARDECLARATION_INIT(arg_node), arg_info);
    VARDECLARATION_NEXT(arg_node) = TRAVopt(VARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunctioncallstmt");

    /* Find the original function declaration in the scope above. */
    node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    node *original;

    while (symboltable != NULL)
    {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), FUNCTIONCALLSTMT_NAME(arg_node));

        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else
        {
            FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node) = original;
            INFO_OG(arg_info) = SYMBOLTABLEENTRY_ORIGINAL(original);
            break;
        }
    }

    if (FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", FUNCTIONCALLSTMT_NAME(arg_node));

    /* Continue with traversing in child nodes. */
    if (FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) != NULL)
        FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);
    else
    {
        if (FUNCTION_PARAMETERS(INFO_OG(arg_info)) != NULL)
            stError(arg_info, arg_node, "function call requires parameter.", FUNCTIONCALLSTMT_NAME(arg_node));
    }

    DBUG_RETURN(arg_node);
}

node *ASfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunctioncallexpr");

    /* Find the original function declaration in the scope above. */
    node *symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    node *original;

    while (symboltable != NULL)
    {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), FUNCTIONCALLEXPR_NAME(arg_node));

        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else
        {
            FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node) = original;
            INFO_OG(arg_info) = SYMBOLTABLEENTRY_ORIGINAL(original);
            break;
        }
    }

    if (FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", FUNCTIONCALLEXPR_NAME(arg_node));

    /* Continue with traversing in child nodes. */
    if (FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) != NULL)
        FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS(arg_node), arg_info);
    else
    {
        if (FUNCTION_PARAMETERS(INFO_OG(arg_info)) != NULL)
            stError(arg_info, arg_node, "function call requires parameter.", FUNCTIONCALLSTMT_NAME(arg_node));
    }

    DBUG_RETURN(arg_node);
}

/* 

TO DO (ctrl v van milestone 5)

!! Remove the declaration part from for-loop induction variables.
create corresponding local variable declarations on the level of the (innermost) function definition.
Beware of nested for-loops using the same induction variable and occurrences of identically named variables
outside the scope of the corresponding for-loop. Like explained above, context disambiguation
and possibly a systematic renaming of for-loop induction variables is needed.

*/
node *ASfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfor");
    DBUG_RETURN(arg_node);
}

/* Found nodes that should read from a symbol table entry  */
node *ASvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvar");

    /* Find the original function declaration in the scope above. */
    node *symboltable = INFO_STACK(arg_info);
    node *original;

    while (symboltable != NULL)
    {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), VAR_NAME(arg_node));

        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else
        {
            VAR_SYMBOLTABLEENTRY(arg_node) = original;
            break;
        }
    }

    if (VAR_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", VAR_NAME(arg_node));

    /* Continue with traversing in optional child nodes. */
    VAR_INDICES(arg_node) = TRAVopt(VAR_INDICES(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvarlet");

    /* Find the original function declaration in the scope above. */
    node *symboltable = INFO_STACK(arg_info);
    node *original;

    while (symboltable != NULL)
    {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), VARLET_NAME(arg_node));

        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else
        {
            VARLET_SYMBOLTABLEENTRY(arg_node) = original;
            break;
        }
    }

    if (VARLET_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", VARLET_NAME(arg_node));

    /* Continue with traversing in optional child nodes. */
    VARLET_INDICES(arg_node) = TRAVopt(VARLET_INDICES(arg_node), arg_info);
    VARLET_NEXT(arg_node) = TRAVopt(VARLET_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASids(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASids");

    char *name = IDS_NAME(arg_node);

    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, T_int, INFO_SIZE(arg_info) - 1, NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        IDS_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

    IDS_NEXT(arg_node) = TRAVopt(IDS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Stack functions. */
void stackPush(node *symboltable, info *arg_info)
{
    INFO_STACK(arg_info) = symboltable;
    INFO_SIZE(arg_info) += 1;
    return;
}

void stackPop(info *arg_info)
{
    INFO_STACK(arg_info) = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    INFO_SIZE(arg_info) -= 1;
    return;
}

bool stackEmpty(info *arg_info)
{
    if (INFO_STACK(arg_info) == NULL && INFO_SIZE(arg_info) == 0)
        return TRUE;
    else
        return FALSE;
}

/* Returns a string of the type. Unnecessary now, might be useful later? */
char *TypetoString(type Type)
{
    char *typeString;
    switch (Type)
    {
    case T_int:
        typeString = "int";
        break;
    case T_float:
        typeString = "float";
        break;
    case T_bool:
        typeString = "bool";
        break;
    case T_void:
        typeString = "void";
        break;
    case T_unknown:
        DBUG_ASSERT(0, "trying to print unknown type!");
        break;
    }

    return typeString;
}
/* Functions to traverse to symbol table entry linked lists. */
node *travList(node *symboltableentry)
{
    node *trav = symboltableentry;
    if (trav == NULL)
        return NULL;

    while (SYMBOLTABLEENTRY_NEXT(trav) != NULL)
        trav = SYMBOLTABLEENTRY_NEXT(trav);

    return trav;
}

node *findOriginal(node *symboltableentry, char *name)
{
    node *trav = symboltableentry;

    while (trav != NULL)
    {
        if (STReq(SYMBOLTABLEENTRY_NAME(trav), name) == TRUE)
            return trav;
        else
            trav = SYMBOLTABLEENTRY_NEXT(trav);
    }

    return trav;
}

bool checkDuplicates(node *symboltableentry, char *name)
{
    node *trav = symboltableentry;

    while (trav != NULL)
    {
        if (STReq(SYMBOLTABLEENTRY_NAME(trav), name) == TRUE)
            return FALSE;
        else
            trav = SYMBOLTABLEENTRY_NEXT(trav);
    }

    return TRUE;
}

/* Prints the current symboltable. */
void printSymboltable(node *symboltableentry)
{
    node *trav = symboltableentry;

    while (trav != NULL)
    {
        printf("%s \n", SYMBOLTABLEENTRY_NAME(trav));
        trav = SYMBOLTABLEENTRY_NEXT(trav);
    }
}

/* Traversal start function */
node *ASdoAddSymbolTables(node *syntaxtree)
{
    DBUG_ENTER("ASdoAddSymboltables");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_as);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    if (INFO_ERRORS(arg_info) != 0)
        CTIabort("Found %i errors during the context analysis. Aborting the compilation.", INFO_ERRORS(arg_info));

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}

/* Function to call when a symboltable error arises. */
void stError(info *arg_info, node *arg_node, char *message, char *name)
{
    if (name == NULL)
        CTInote("Error on line %i, col %i: %s", NODE_LINE(arg_node), NODE_COL(arg_node), message);
    else
        CTInote("Error on line %i, col %i: %s %s", NODE_LINE(arg_node), NODE_COL(arg_node), name, message);

    INFO_ERRORS(arg_info) += 1;
}

void printLine(info *arg_info, char *name)
{
    node *symboltable = INFO_STACK(arg_info);
    node *original;
    while (symboltable != NULL)
    {
        original = findOriginal(SYMBOLTABLE_NEXT(symboltable), name);
        if (original == NULL)
            symboltable = SYMBOLTABLE_PREV(symboltable);
        else
        {
            CTInote("> Original declaration was on line %i.", NODE_LINE(original));
            break;
        }
    }
}
