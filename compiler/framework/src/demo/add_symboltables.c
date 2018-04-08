/*
 * Module: add_symboltables.c
 * Description: Traverse and create / look for declarations in a symbol table.
 * Prefix: AS
 * Author: Andrea van den Hooff
 * Arrays not implemented.
 */

// NOTE: TURNED PRINTING SYMBOL TABLES OFF IN ASPROGRAM AND ASFUNCTION

#include "add_symboltables.h"

#include "ctinfo.h"
#include "dbug.h"
#include "free.h"
#include "globals.h"
#include "memory.h"
#include "str.h"
#include "tree_basic.h"
#include "traverse.h"
#include "types.h"

/* INFO structure */
struct INFO
{
    node *stack;
    node *original;
    node *functions;
    int parametercount;
    int offsetcount;
    int functionparamcount;
    int globaldeccount;
    int globaldefcount;
    int externfuncount;
    int errors;
    int stacksize;
    int currentscope;
    int forloopcount;
    bool inforloop;
    bool declsleft;
};

/* struct macros */
#define INFO_STACK(n) ((n)->stack)
#define INFO_OG(n) ((n)->original)
#define INFO_PMC(n) ((n)->parametercount)
#define INFO_OSC(n) ((n)->offsetcount)
#define INFO_FC(n) ((n)->functionparamcount)
#define INFO_EFC(n) ((n)->externfuncount)
#define INFO_GDC(n) ((n)->globaldeccount)
#define INFO_GDF(n) ((n)->globaldefcount)
#define INFO_STACKSIZE(n) ((n)->stacksize)
#define INFO_CS(n) ((n)->currentscope)
#define INFO_FLC(n) ((n)->forloopcount)
#define INFO_FUN(n) ((n)->functions)
#define INFO_DECLSLEFT(n) ((n)->declsleft)
#define INFO_ERRORS(n) ((n)->errors)

/* INFO functions */
static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_STACK(result) = NULL;
    INFO_STACKSIZE(result) = 0;
    INFO_ERRORS(result) = 0;
    INFO_FC(result) = 0;
    INFO_EFC(result) = 0;
    INFO_PMC(result) = 0;
    INFO_OSC(result) = 0;
    INFO_GDC(result) = 0;
    INFO_CS(result) = 0;
    INFO_GDF(result) = 0;
    INFO_FLC(result) = 0;

    DBUG_RETURN(result);
}
static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/* Found nodes that should write to a global symbol table entry.  */
node *ASprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASprogram");

    /* Create a symbol table node for the global scoped variables and functions. */
    node *globals = TBmakeSymboltable(NULL, NULL);
    stackPush(globals, arg_info); // Stacksize = 1;
    PROGRAM_SYMBOLTABLE(arg_node) = globals;

    if (stackEmpty(arg_info) == TRUE)
        stError(arg_info, arg_node, "Something went wrong during context analysis.", NULL);

    /* Continue with traversing in child nodes. */
    PROGRAM_DECLARATIONS(arg_node) = TRAVdo(PROGRAM_DECLARATIONS(arg_node), arg_info);

    /* Print the symbol table. */
    // CTInote("************************************ \n Global symboltable. Scope level: %i. \n************************************", INFO_STACKSIZE(arg_info) - 1);
    // node *entry = SYMBOLTABLE_NEXT(INFO_STACK(arg_info));
    // while (entry != NULL)
    // {
    //     CTIwarn("* Name: %s. Type: %s.", SYMBOLTABLEENTRY_NAME(entry), TypetoString(SYMBOLTABLEENTRY_TYPE(entry)));
    //     entry = SYMBOLTABLEENTRY_NEXT(entry);
    // }
    // CTInote("************************************");

    /* Remove the linked list at the end of the traversal. */
    if (INFO_STACKSIZE(arg_info) != 0)
        stackPop(arg_info);
    if (stackEmpty(arg_info) == FALSE)
        stError(arg_info, arg_node, "Something went wrong during context analysis. Stack wasn't empty at the end of traversal.", NULL);

    DBUG_RETURN(arg_node);
}

node *ASdeclarations(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASdeclarations");

    DECLARATIONS_DECLARATION(arg_node) = TRAVopt(DECLARATIONS_DECLARATION(arg_node), arg_info);
    DECLARATIONS_NEXT(arg_node) = TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);

    // node *n;

    // if (DECLARATIONS_DECLARATION(arg_node) != NULL)
    // {
    //     /* First go into the decls that aren't functions with function bodies.*/
    //     if ((NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_function &&
    //          FUNCTION_FUNCTIONBODY(DECLARATIONS_DECLARATION(arg_node)) == NULL) ||
    //         NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_globaldef ||
    //         NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_globaldec)
    //     {
    //         DECLARATIONS_DECLARATION(arg_node) = TRAVopt(DECLARATIONS_DECLARATION(arg_node), arg_info);
    //     }

    //     /* Only then go into the function bodies. */
    //     if ((NODE_TYPE(DECLARATIONS_DECLARATION(arg_node)) == N_function) &&
    //         FUNCTION_FUNCTIONBODY(DECLARATIONS_DECLARATION(arg_node)) != NULL)
    //     {
    //         n = travDecls(arg_node, arg_info);
    //         if (INFO_DECLSLEFT(arg_info) == TRUE)
    //         {
    //             DECLARATIONS_NEXT(n) = arg_node;
    //             DECLARATIONS_NEXT(arg_node) = TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    //             DBUG_RETURN(arg_node);
    //         }
    //         else
    //             DECLARATIONS_DECLARATION(arg_node) = TRAVopt(DECLARATIONS_DECLARATION(arg_node), arg_info);
    //     }

    //     DECLARATIONS_NEXT(arg_node) = TRAVopt(DECLARATIONS_NEXT(arg_node), arg_info);
    // }

    DBUG_RETURN(arg_node);
}

node *ASglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASglobaldec");
    CTInote("%s", GLOBALDEC_NAME(arg_node));

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
        node *newEntry = TBmakeSymboltableentry(name, GLOBALDEC_TYPE(arg_node), INFO_STACKSIZE(arg_info) - 1, NULL);
        SYMBOLTABLEENTRY_ORIGINAL(newEntry) = arg_node;
        SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_GDC(arg_info);
        INFO_GDC(arg_info) += 1;

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
    CTInote("%s", GLOBALDEF_NAME(arg_node));

    /* Found globaldef. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the globaldef into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, GLOBALDEF_TYPE(arg_node), INFO_STACKSIZE(arg_info) - 1, NULL);
        SYMBOLTABLEENTRY_ORIGINAL(newEntry) = arg_node;
        SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_GDF(arg_info);
        INFO_GDF(arg_info) += 1;

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

/* Found nodes that should write or read to a function symbol table entry  */
node *ASfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunction");
    node *functionSymboltable;
    char *name = FUNCTION_NAME(arg_node);
    CTInote("%s", FUNCTION_NAME(arg_node));

    /* Found function. Check if there already is one with the same name in current symbol table. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    /* Check if function is extern, stay in current scope */
    else
    {
        /* Make a new symbol table entry. */
        node *newEntry = TBmakeSymboltableentry(name, FUNCTION_TYPE(arg_node), INFO_STACKSIZE(arg_info) - 1, NULL);
        SYMBOLTABLEENTRY_ORIGINAL(newEntry) = arg_node;
        SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_OSC(arg_info);

        /* Add STE to end of the list of current ST. */
        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        FUNCTION_SYMBOLTABLEENTRY(arg_node) = newEntry;

        /* Create new symbol table and add it to the stack. */
        functionSymboltable = TBmakeSymboltable(NULL, INFO_STACK(arg_info));
        if (FUNCTION_ISEXTERN(arg_node) || INFO_STACKSIZE(arg_info) == 1)
        {
            SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_EFC(arg_info);
            INFO_EFC(arg_info) += 1;
            SYMBOLTABLE_SCOPE(functionSymboltable) = 0;
        }
        else
            SYMBOLTABLE_SCOPE(functionSymboltable) = INFO_STACKSIZE(arg_info) - 1;

        stackPush(functionSymboltable, arg_info);

        /* Reset counters for new scope. */
        // INFO_CS(arg_info) = INFO_STACKSIZE(arg_info) - 1;
        INFO_CS(arg_info) = SYMBOLTABLE_SCOPE(functionSymboltable);
        if (!FUNCTION_ISEXTERN(arg_node))
            INFO_OSC(arg_info) = 0;

        if (stackEmpty(arg_info) == TRUE)
            stError(arg_info, arg_node, ": something went wrong with creating a symboltable in this function.", name);

        /* Update function administration. */
        FUNCTION_SYMBOLTABLE(arg_node) = functionSymboltable;

        /* Continue with traversing in child nodes. Set STMTS count on 0, so that we can check if the last stmts will have a return at the end. */
        FUNCTION_PARAMETERS(arg_node) = TRAVopt(FUNCTION_PARAMETERS(arg_node), arg_info);
        FUNCTION_FUNCTIONBODY(arg_node) = TRAVopt(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

        // COUNT FOR LOOPS FOR LATER.
        if (FUNCTION_FUNCTIONBODY(arg_node) != NULL)
        {
            int c = countForLoops(FUNCTION_FUNCTIONBODY(arg_node));
            FUNCTIONBODY_FORLOOPS(FUNCTION_FUNCTIONBODY(arg_node)) = c;
        }

        /* Print symboltable of current function. */
        // CTInote("************************************ \n Function %s's symboltable. Scope level: %i. \n************************************", name, SYMBOLTABLE_SCOPE(functionSymboltable));
        // node *entry = SYMBOLTABLE_NEXT(INFO_STACK(arg_info));
        // while (entry != NULL)
        // {
        //     CTInote("* Name: %s. Type: %s.", SYMBOLTABLEENTRY_NAME(entry), TypetoString(SYMBOLTABLEENTRY_TYPE(entry)));
        //     entry = SYMBOLTABLEENTRY_NEXT(entry);
        // }
        // CTInote("************************************");

        /* Remove the linked list at the end of the traversal. */
        stackPop(arg_info);
        // INFO_CS(arg_info) = INFO_STACKSIZE(arg_info) - 1;
    }

    DBUG_RETURN(arg_node);
}

/* Create new variables found in parameters. */
node *ASparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASparameters");

    char *name = PARAMETERS_NAME(arg_node);

    // WAT DOEN WE HIER MEE??
    PARAMETERS_SCOPE(arg_node) = INFO_STACKSIZE(arg_info);

    /* Found parameter. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the parameter into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, PARAMETERS_TYPE(arg_node), INFO_STACKSIZE(arg_info) - 1, NULL);
        SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_OSC(arg_info);
        SYMBOLTABLEENTRY_ORIGINAL(newEntry) = arg_node;
        INFO_OSC(arg_info) += 1;

        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        node *symboltable;
        /* Add links to the symbol table entry & to the function definition table entry. */
        if (INFO_STACKSIZE(arg_info) == 1)
            symboltable = INFO_STACK(arg_info);
        else
            symboltable = SYMBOLTABLE_PREV(INFO_STACK(arg_info));

        node *original = travList(SYMBOLTABLE_NEXT(symboltable));
        PARAMETERS_PARAMETERSYMBOLTABLEENTRY(arg_node) = newEntry;
        PARAMETERS_FUNCTIONSYMBOLTABLEENTRY(arg_node) = original;
    }

    /* Continue with traversing in child nodes. */
    PARAMETERS_DIMENSIONS(arg_node) = TRAVopt(PARAMETERS_DIMENSIONS(arg_node), arg_info);
    PARAMETERS_NEXT(arg_node) = TRAVopt(PARAMETERS_NEXT(arg_node), arg_info);

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

/* Create new entries for variables found in parameters. */
node *ASexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASexpressions");

    /* Count parameters in fundef. */
    if (INFO_PMC(arg_info) == 0)
    {
        node *param = FUNCTION_PARAMETERS(INFO_OG(arg_info));
        if (FUNCTION_PARAMETERS(INFO_OG(arg_info)) != NULL)
            INFO_FC(arg_info) += 1;
        while (PARAMETERS_NEXT(param) != NULL)
        {
            INFO_FC(arg_info) += 1;
            param = PARAMETERS_NEXT(param);
        }
    }

    /* Counts parameters in functioncall. */
    if (EXPRESSIONS_EXPR(arg_node) != NULL)
    {
        INFO_PMC(arg_info) += 1;
        EXPRESSIONS_EXPR(arg_node) = TRAVdo(EXPRESSIONS_EXPR(arg_node), arg_info);
    }

    /* Continue with traversing in optional child nodes. */
    EXPRESSIONS_NEXT(arg_node) = TRAVopt(EXPRESSIONS_NEXT(arg_node), arg_info);

    if (EXPRESSIONS_NEXT(arg_node) == NULL)
    {
        if (INFO_PMC(arg_info) != INFO_FC(arg_info))
        {
            stError(arg_info, arg_node, "The amount of parameters didn't match the function definition.", NULL);
            CTIwarn("> Expected %i, but found %i parameters.", INFO_FC(arg_info), INFO_PMC(arg_info));
        }
    }

    /* Reset counters. */
    INFO_PMC(arg_info) = 0;
    INFO_FC(arg_info) = 0;

    DBUG_RETURN(arg_node);
}

/* Create new entries for variables found in vardeclarations. */
node *ASvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvardeclaration");

    char *name = VARDECLARATION_NAME(arg_node);

    /* Add scope of current vardeclaration for codegeneration. */
    VARDECLARATION_SCOPE(arg_node) = INFO_STACKSIZE(arg_info) - 1;

    /* Found vardeclaration. Check if there already is one with the same name. */
    if (checkDuplicates(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)), name) == FALSE)
    {
        stError(arg_info, arg_node, "has already been declared.", name);
        printLine(arg_info, name);
    }
    else
    {
        /* Else, insert the vardeclaration into the symbol table linked list at the end. */
        node *newEntry = TBmakeSymboltableentry(name, VARDECLARATION_TYPE(arg_node), INFO_STACKSIZE(arg_info) - 1, NULL);
        SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_OSC(arg_info);
        SYMBOLTABLEENTRY_ORIGINAL(newEntry) = arg_node;
        INFO_OSC(arg_info) += 1;

        node *last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
        if (last == NULL)
            SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
        else
            SYMBOLTABLEENTRY_NEXT(last) = newEntry;

        VARDECLARATION_SYMBOLTABLEENTRY(arg_node) = newEntry;
    }

    /* Continue with traversing in optional child nodes. */
    if (VARDECLARATION_DIMENSIONS(arg_node) != NULL)
        stError(arg_info, arg_node, ": arrays have not been implemented.", name);

    VARDECLARATION_INIT(arg_node) = TRAVopt(VARDECLARATION_INIT(arg_node), arg_info);
    VARDECLARATION_NEXT(arg_node) = TRAVopt(VARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Find the original function declaration and put a link between it and the functioncall. */
node *ASfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunctioncallstmt");

    /* Add current scope for codegeneration later. */
    FUNCTIONCALLSTMT_SCOPE(arg_node) = INFO_STACKSIZE(arg_info);

    /* Find the original function declaration in the scopes above. */
    node *symboltable = INFO_STACK(arg_info);
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
    else
    {

        /* Continue with traversing in child nodes. */
        if (FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) != NULL)
            FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);
        else
        {
            if (FUNCTION_PARAMETERS(INFO_OG(arg_info)) != NULL)
                stError(arg_info, arg_node, "function call requires parameter.", FUNCTIONCALLSTMT_NAME(arg_node));
        }
    }

    DBUG_RETURN(arg_node);
}

/* Find the original function declaration and put a link between it and the functioncall. */
node *ASfunctioncallexpr(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfunctioncallexpr");

    /* Add current scope for codegeneration later. */
    FUNCTIONCALLEXPR_SCOPE(arg_node) = INFO_STACKSIZE(arg_info);

    /* Find the original function declaration in the scope above. */
    node *symboltable = INFO_STACK(arg_info);
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
    else
    {
        /* Continue with traversing in child nodes. */
        if (FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) != NULL)
            FUNCTIONCALLEXPR_EXPRESSIONS(arg_node) = TRAVdo(FUNCTIONCALLEXPR_EXPRESSIONS(arg_node), arg_info);
        else
        {
            if (FUNCTION_PARAMETERS(INFO_OG(arg_info)) != NULL)
                stError(arg_info, arg_node, "function call requires parameter.", FUNCTIONCALLEXPR_NAME(arg_node));
        }
    }

    DBUG_RETURN(arg_node);
}

node *ASfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASfor");
    node *newVardecl, *newEntry, *newVarlet, *last;
    char *name, *label;

    /* Add a link to the last symboltable + current offset for code generation. */
    FOR_SYMBOLTABLE(arg_node) = INFO_STACK(arg_info);

    /* Create a new vardeclaration for the initvar. */
    label = STRitoa(INFO_OSC(arg_info));
    name = STRcatn(3, FOR_INITVAR(arg_node), "_", label); // i_3 bijv.
    FOR_INITVAR(arg_node) = name;

    /* Create a symbol table entry for the initvar. */
    newEntry = TBmakeSymboltableentry(name, T_int, INFO_STACKSIZE(arg_info) - 1, NULL);
    SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_OSC(arg_info);
    SYMBOLTABLEENTRY_ORIGINAL(newEntry) = newEntry; // EASTER EGG
    INFO_OSC(arg_info) += 1;

    /* Add STE to function's symboltable.    */
    last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
    if (last == NULL)
        SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
    else
        SYMBOLTABLEENTRY_NEXT(last) = newEntry;

    /* Create var declaration for stopvar. */
    label = STRitoa(INFO_OSC(arg_info));
    name = STRcat("stop_", label); // stop_4 bijv.

    /* Create a symbol table entry for the stopvar. */
    newEntry = TBmakeSymboltableentry(name, T_int, INFO_STACKSIZE(arg_info) - 1, NULL);
    SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_OSC(arg_info);
    SYMBOLTABLEENTRY_ORIGINAL(newEntry) = newEntry;
    INFO_OSC(arg_info) += 1;

    /* Add STE to function's symboltable.    */
    last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
    if (last == NULL)
        SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
    else
        SYMBOLTABLEENTRY_NEXT(last) = newEntry;

    /* Create entry for stepvar. */
    label = STRitoa(INFO_OSC(arg_info));
    name = STRcat("step_", label); // step_4 bijv.

    /* Create a symbol table entry for the stepvar. */
    newEntry = TBmakeSymboltableentry(name, T_int, INFO_STACKSIZE(arg_info) - 1, NULL);
    SYMBOLTABLEENTRY_OFFSET(newEntry) = INFO_OSC(arg_info);
    SYMBOLTABLEENTRY_ORIGINAL(newEntry) = newEntry;
    INFO_OSC(arg_info) += 1;

    /* Add STE to function's symboltable.    */
    last = travList(SYMBOLTABLE_NEXT(INFO_STACK(arg_info)));
    if (last == NULL)
        SYMBOLTABLE_NEXT(INFO_STACK(arg_info)) = newEntry;
    else
        SYMBOLTABLEENTRY_NEXT(last) = newEntry;

    /* Traverse into child nodes. */
    FOR_START(arg_node) = TRAVdo(FOR_START(arg_node), arg_info);
    FOR_STOP(arg_node) = TRAVdo(FOR_STOP(arg_node), arg_info);
    FOR_STEP(arg_node) = TRAVopt(FOR_STEP(arg_node), arg_info);

    INFO_FLC(arg_info) += 3;
    FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);
    INFO_FLC(arg_info) -= 3;

    DBUG_RETURN(arg_node);
}

/* Found nodes that should read from a symbol table entry  */
node *ASvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvar");

    /* Save the current scope for later in code generation. */
    VAR_SCOPE(arg_node) = INFO_STACKSIZE(arg_info) - 1;

    /* Find the original function declaration in the scope above. */
    node *symboltable = INFO_STACK(arg_info);
    node *original;
    bool found = FALSE;

    if (INFO_FLC(arg_info) != 0 && symboltable != NULL)
    {
        char *name = VAR_NAME(arg_node);
        char *s;
        for (int i = 3; i <= INFO_FLC(arg_info); i += 3)
        {
            s = STRitoa(INFO_OSC(arg_info) - i);
            name = STRcatn(3, VAR_NAME(arg_node), "_", s);
            original = findOriginal(SYMBOLTABLE_NEXT(symboltable), name);
            if (original != NULL)
            {
                VAR_SYMBOLTABLEENTRY(arg_node) = original;
                VAR_NAME(arg_node) = name;
                found = TRUE;
                break;
            }
        }
    }
    if (found == FALSE)
    {
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
    }

    // CTInote("VAR %s on line %i. Current scope: %i Stack size: %i, ", VAR_NAME(arg_node), NODE_LINE(arg_node), VAR_SCOPE(arg_node), INFO_STACKSIZE(arg_info));

    if (VAR_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", VAR_NAME(arg_node));

    /* Arrays are not implemented. */
    if (VAR_INDICES(arg_node) != NULL)
        stError(arg_info, arg_node, ": arrays have not been implemented.", VAR_NAME(arg_node));

    DBUG_RETURN(arg_node);
}

node *ASvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("ASvarlet");

    /* Save the current scope for later in code generation. */
    VARLET_SCOPE(arg_node) = INFO_STACKSIZE(arg_info) - 1;

    /* Find the original vardeclaration in the scope above. */
    node *symboltable = INFO_STACK(arg_info);
    node *original;
    bool found = FALSE;

    /* Check if it has been declared in a for loop. */
    if (INFO_FLC(arg_info) != 0 && symboltable != NULL)
    {
        char *name = VARLET_NAME(arg_node);
        char *s;
        for (int i = 3; i <= INFO_FLC(arg_info); i += 3)
        {
            s = STRitoa(INFO_OSC(arg_info) - i);
            name = STRcatn(3, VARLET_NAME(arg_node), "_", s);
            original = findOriginal(SYMBOLTABLE_NEXT(symboltable), name);
            if (original != NULL)
            {
                VARLET_SYMBOLTABLEENTRY(arg_node) = original;
                found = TRUE;
                // VARLET_NAME(arg_node) = name;
                CTIabort("Cannot assign to induction variable %s.", VARLET_NAME(arg_node));
                break;
            }
        }
    }
    if (found == FALSE)
    {
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
    }

    // CTInote("VARLET %s on line %i. Current scope: %i Stack size: %i, ", VARLET_NAME(arg_node), NODE_LINE(arg_node), VARLET_SCOPE(arg_node), INFO_STACKSIZE(arg_info));

    if (VARLET_SYMBOLTABLEENTRY(arg_node) == NULL)
        stError(arg_info, arg_node, "has not been declared yet.", VARLET_NAME(arg_node));

    /* Continue with traversing in optional child nodes. */
    VARLET_NEXT(arg_node) = TRAVopt(VARLET_NEXT(arg_node), arg_info);

    if (VARLET_INDICES(arg_node) != NULL)
        stError(arg_info, arg_node, ": arrays have not been implemented.", VARLET_NAME(arg_node));

    DBUG_RETURN(arg_node);
}

/* Stack functions. */
void stackPush(node *symboltable, info *arg_info)
{
    INFO_STACK(arg_info) = symboltable;
    INFO_STACKSIZE(arg_info) += 1;
    return;
}

void stackPop(info *arg_info)
{
    INFO_STACK(arg_info) = SYMBOLTABLE_PREV(INFO_STACK(arg_info));
    INFO_STACKSIZE(arg_info) -= 1;
    return;
}

bool stackEmpty(info *arg_info)
{
    if (INFO_STACK(arg_info) == NULL && INFO_STACKSIZE(arg_info) == 0)
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
        CTIwarn("Error on line %i, col %i: %s", NODE_LINE(arg_node), NODE_COL(arg_node), message);
    else
        CTIwarn("Error on line %i, col %i: %s %s", NODE_LINE(arg_node), NODE_COL(arg_node), name, message);

    INFO_ERRORS(arg_info) += 1;
}

node *travDecls(node *arg_node, info *arg_info)
{
    node *n = arg_node;
    INFO_DECLSLEFT(arg_info) = FALSE;

    if (n != NULL)
    {
        if ((NODE_TYPE(DECLARATIONS_DECLARATION(n)) == N_function &&
             FUNCTION_FUNCTIONBODY(DECLARATIONS_DECLARATION(n)) == NULL) ||
            NODE_TYPE(DECLARATIONS_DECLARATION(n)) == N_globaldef ||
            NODE_TYPE(DECLARATIONS_DECLARATION(n)) == N_globaldec)
            INFO_DECLSLEFT(arg_info) = TRUE;

        while (DECLARATIONS_NEXT(n) != NULL)
        {
            CTInote("HIER");
            if ((NODE_TYPE(DECLARATIONS_DECLARATION(n)) == N_function &&
                 FUNCTION_FUNCTIONBODY(DECLARATIONS_DECLARATION(n)) == NULL) ||
                NODE_TYPE(DECLARATIONS_DECLARATION(n)) == N_globaldef ||
                NODE_TYPE(DECLARATIONS_DECLARATION(n)) == N_globaldec)
                INFO_DECLSLEFT(arg_info) = TRUE;
            n = DECLARATIONS_NEXT(n);
        }
    }

    return n;
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
            CTIwarn("> Original declaration was on line %i.", NODE_LINE(original));
            break;
        }
    }
}

int countForLoops(node *arg_node)
{
    if (FUNCTIONBODY_STMTS(arg_node) != NULL)
    {
        int count = 0;
        node *n;
        n = FUNCTIONBODY_STMTS(arg_node);

        while (STMTS_NEXT(n) != NULL)
        {
            if (NODE_TYPE(STMTS_STMT(n)) == N_for)
                count += 1;

            n = STMTS_NEXT(n);
        }

        if (NODE_TYPE(STMTS_STMT(n)) == N_for)
            return count + 1;
        else
            return count;
    }
    else
        return 0;
}