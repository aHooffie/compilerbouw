/*
 * Module: Traversing and looking for symbol table links
 * Prefix: AS
 */

/* 

TO DO (ctrl v van milestone 5)
!! A proper error message must be produced: (no matching declaration/definition) or is ambiguous (multiple matching declarations/definitions).
- Moeten we hier de locatie ook printen? Want dan moeten we misschien de regellijn + kolom ook in de ste opslaan. Lijkt me wel handig eigenlijk.
- Ik heb dit geprobeerd, maar kon niet de juiste lines printen (zie uitgecommend in function & program. )


!! Likewise argument numbers in function calls must match parameter numbers of called function. 
Note that matching types is not done during context analysis but left for a separate type checking
pass.

!! The compilation process shall as far as possible continue in the presence of context errors in order
to report multiple such errors in a single compiler run.

!! Each symbol table entry would feature the name as a character string, its type & >> nesting level <<
(starting with zero for the global context). Symbol table entries can and should be extended by
all information about the variable gathered during the compilation process.

!! Make sure that your compiler appropriately prints the symbol table, for instance as a structured
comment in the beginning of the function body or preceding the entire function definition.

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
#include "lookup_table.h"
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
#define INFO_STACK(n) ((n)->stack)
#define INFO_SIZE(n) ((n)->size)
#define INFO_ERRORS(n) ((n)->errors)

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
    // printf("Program: %i, %i\n", global.line, global.col);

    /* Create a symbol table node for the global scoped variables and functions. */
    node *globals = TBmakeSymboltable(NULL, NULL);
    stackPush(globals, arg_info);
    PROGRAM_SYMBOLTABLE(arg_node) = globals;

    if (INFO_STACK(arg_info) == NULL)
    {
        CTInote("Something went wrong with creating a symboltable in the Program node. \n");
        INFO_ERRORS(arg_info) += 1;
        return NULL;
    }

    /* Continue with traversing in child nodes. */
    PROGRAM_DECLARATIONS(arg_node) = TRAVdo(PROGRAM_DECLARATIONS(arg_node), arg_info);

    /* Remove the linked list at the end of the traversal. */
    stackPop(arg_info);
    if (INFO_SIZE(arg_info) != 0)
    {
        CTInote("Something went wrong. Stack wasn't empty at the end of traversal.\n");
        INFO_ERRORS(arg_info) += 1;
    }

    DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASglobaldec");

    /* Found globaldec. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), GLOBALDEC_NAME(arg_node)) == FALSE)
    {
        CTInote("! Error: Globaldeclaration %s has already been declared.\n", GLOBALDEC_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }
    else
    {
        /* Else, insert the globaldec into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(GLOBALDEC_NAME(arg_node), GLOBALDEC_TYPE(arg_node), global.line, global.col, NULL);
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

    /* Found globaldef. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), GLOBALDEF_NAME(arg_node)) == FALSE)
    {
        CTInote("! Error: Global definition %s has already been declared.\n", GLOBALDEF_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }
    else
    {
        /* Else, insert the globaldef into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(GLOBALDEF_NAME(arg_node), GLOBALDEF_TYPE(arg_node), global.line, global.col, NULL);
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

/* Found nodes that should write or read to a function symbol table entry  */
node *ASfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunction");
    // printf("Function %i, %i\n", global.line, global.col);

    /* Found function. Check if there already is one with the same name in global ST. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), FUNCTION_NAME(arg_node)) == FALSE)
    {
        CTInote("! Error: Function %s has already been declared.\n", FUNCTION_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }
    else
    {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(FUNCTION_NAME(arg_node), FUNCTION_TYPE(arg_node), global.line, global.col, NULL);
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

    if (INFO_STACK(arg_info) == NULL)
    {
        CTInote("Something went wrong with the symboltable in the function %s. \n", FUNCTION_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }

    /* Update function administration. */
    FUNCTION_SYMBOLTABLE(arg_node) = functionSymboltable;

    /* Continue with traversing in child nodes. */
    if (FUNCTION_PARAMETERS(arg_node) != NULL)
        FUNCTION_PARAMETERS(arg_node) = TRAVdo(FUNCTION_PARAMETERS(arg_node), arg_info);
    if (FUNCTION_FUNCTIONBODY(arg_node) != NULL)
        FUNCTION_FUNCTIONBODY(arg_node) = TRAVdo(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    /* Remove the linked list at the end of the traversal. */
    stackPop(arg_info);

    DBUG_RETURN(arg_node);
}

node *ASparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASparameters");

    /* Found parameter. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), PARAMETERS_NAME(arg_node)) == FALSE)
    {
        CTInote("! Error: Parameter %s has already been declared.\n", PARAMETERS_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }
    else
    {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(PARAMETERS_NAME(arg_node), PARAMETERS_TYPE(arg_node), global.line, global.col, NULL);
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));

        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        PARAMETERS_SYMBOLTABLEENTRY(arg_node) = newEntry;
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

    /* Found vardeclaration. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), VARDECLARATION_NAME(arg_node)) == FALSE)
    {
        CTInote("! Error: Vardeclaration %s has already been declared.\n", VARDECLARATION_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }
    else
    {
        /* Else, insert the globaldef into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(VARDECLARATION_NAME(arg_node), VARDECLARATION_TYPE(arg_node), global.line, global.col, NULL);
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
    {
        CTInote("Error: Functioncallstmt %s is nog niet gedeclareerd.\n", FUNCTIONCALLSTMT_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }

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
    {
        CTInote("Error: Functioncallexpr %s is nog niet gedeclareerd.\n", FUNCTIONCALLEXPR_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }

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
    {
        CTInote("! Error. Var %s is nog niet gedeclareerd.\n", VAR_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }

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
    {
        CTInote("! Error. Varlet %s is nog niet gedeclareerd.\n", VARLET_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }

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

    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), IDS_NAME(arg_node)) == FALSE)
    {
        CTInote("! Error: ID %s has already been declared.\n", IDS_NAME(arg_node));
        INFO_ERRORS(arg_info) += 1;
    }
    else
    {
        /* Else, insert the function into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(IDS_NAME(arg_node), T_int, global.line, global.col, NULL);
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
        DBUG_ASSERT(0, "unknown type detected!");
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

    CTInote("Traversing done...\n");

    if (INFO_ERRORS(arg_info) != 0)
    {
        CTIabort("Found %i errors during the context analysis. Aborting the compilation.\n", INFO_ERRORS(arg_info));
    }

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
