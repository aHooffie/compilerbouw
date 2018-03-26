/*
 * Module: Traversing and looking for symbol table links
 * Prefix: AS
 */

/* 

TO DO (ctrl v van milestone 5)

!! Likewise argument numbers in function calls must match parameter numbers of called function. 
Note that matching types is not done during context analysis but left for a separate type checking
pass.
- ??????

!! Context analysis disambiguates equally named symbols according to the scoping rules of the
language, both variables and functions. For documentation as well as debugging purposes this
disambiguation should also be visualised when displaying the abstract syntax tree after context
analysis. This could, for example, be achieved by consistent renaming of identiers incorporating
a suitable representation of the scope level into the variable name. Alternatively, you could also
simply print the scope information together with the variable name when visualising the abstract
syntax tree.

!! Last but not least, remove the declaration part from for-loop induction variables and create corre-
sponding local variable declarations on the level of the (innermost) function denition. Beware of
nested for-loops using the same induction variable and occurrences of identically named variables
outside the scope of the corresponding for-loop. Like explained above, context disambiguation
and possibly a systematic renaming of for-loop induction variables is needed.

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
    int size;
    int errors;
};

/* struct macros */
#define INFO_STACK(n)   ((n)->stack)
#define INFO_SIZE(n)    ((n)->size)
#define INFO_ERRORS(n)  ((n)->errors)

/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));
    INFO_STACK(result) = NULL;
    INFO_SIZE(result) = 0;
    INFO_ERRORS(result) = 0;

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
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE) {
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
    /* Continue with traversing in child nodes. */
    if (GLOBALDEC_DIMENSIONS(arg_node) != NULL)
        GLOBALDEC_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEC_DIMENSIONS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASglobaldef");

    char *name = GLOBALDEF_NAME(arg_node);

    /* Found globaldef. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE) {
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

    /* Continue with traversing in child nodes. */
    if (GLOBALDEF_DIMENSIONS(arg_node) != NULL)
        GLOBALDEF_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEF_DIMENSIONS(arg_node), arg_info);
    if (GLOBALDEF_ASSIGN(arg_node) != NULL)
        GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);

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

    /* Continue with traversing in child nodes. */
    if (RETURN_EXPR(arg_node) != NULL)
        RETURN_EXPR(arg_node) = TRAVdo(RETURN_EXPR(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Found nodes that should write or read to a function symbol table entry  */
node *ASfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunction");
    char *name = FUNCTION_NAME(arg_node);

    /* Found function. Check if there already is one with the same name in global ST. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE) {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, FUNCTION_TYPE(arg_node), INFO_SIZE(arg_info) - 1, NULL);
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
    if (FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    if (FUNCTION_FUNCTIONBODY(arg_node) != NULL)
        FUNCTION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    /* Print symboltable of current function. */
    CTInote("************************************ \n Function %s symboltable. Scope level: %i. \n************************************", name, INFO_SIZE(arg_info) - 1);
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
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE) {
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
    if (PARAMETERS_DIMENSIONS(arg_node) != NULL)
        PARAMETERS_DIMENSIONS(arg_node) = TRAVdo(PARAMETERS_DIMENSIONS(arg_node), arg_info);

    if (PARAMETERS_NEXT(arg_node) != NULL)
        PARAMETERS_NEXT(arg_node) = TRAVdo(PARAMETERS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvardeclaration");

    char *name = VARDECLARATION_NAME(arg_node);

    /* Found vardeclaration. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE) {
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

    /* Continue with traversing in child nodes. */
    if (VARDECLARATION_DIMENSIONS(arg_node) != NULL)
        VARDECLARATION_DIMENSIONS(arg_node) = TRAVdo(VARDECLARATION_DIMENSIONS(arg_node), NULL);
    if (VARDECLARATION_INIT(arg_node) != NULL)
        VARDECLARATION_INIT(arg_node) = TRAVdo(VARDECLARATION_INIT(arg_node), arg_info);
    if (VARDECLARATION_NEXT(arg_node) != NULL)
        VARDECLARATION_NEXT(arg_node) = TRAVdo(VARDECLARATION_NEXT(arg_node), arg_info);

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
            break;
        }
    }

    if (FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", FUNCTIONCALLSTMT_NAME(arg_node));

    /* Continue with traversing in child nodes. */
    if (FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) != NULL)
        FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);

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
            break;
        }
    }

    if (FUNCTIONCALLEXPR_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", FUNCTIONCALLEXPR_NAME(arg_node));

    /* Continue with traversing in child nodes. */
    if (FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) != NULL)
        FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS(arg_node), arg_info);

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

    /* Continue with traversing in child nodes. */
    if (VAR_INDICES(arg_node) != NULL)
        VAR_INDICES(arg_node) = TRAVdo(VAR_INDICES(arg_node), arg_info);

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

    /* Continue with traversing in child nodes. */
    if (VARLET_INDICES(arg_node) != NULL)
        VARLET_INDICES(arg_node) = TRAVdo(VARLET_INDICES(arg_node), arg_info);
    if (VARLET_NEXT(arg_node) != NULL)
        VARLET_NEXT(arg_node) = TRAVdo(VARLET_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *ASids(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASids");

    char *name = IDS_NAME(arg_node);

    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE) {
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

    if (IDS_NEXT(arg_node) != NULL)
        IDS_NEXT(arg_node) = TRAVdo(IDS_NEXT(arg_node), arg_info);

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

    CTInote("Traversing done...");

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
        else {
            CTInote("> Original declaration was on line %i.", NODE_LINE(original));
            break;
        }
    }
}
