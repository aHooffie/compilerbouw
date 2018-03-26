
/*****************************************************************************
 *
 * Module: make_regularexpr
 *
 * Prefix: RE
 *
 * Description:
 *
 * This module implements a traversal of the abstract syntax tree that 
 * turns variable initialisations into regular assignments
 *
 *****************************************************************************/


#include "make_regularexpr.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"

#include "memory.h"
#include "ctinfo.h"



// een variable declaration with initialisation:
// int a = a + 1;
// bedoeling: dit mag in civic als a al eerder declared is; dan neemt ie die waarde over:

// More precisely, the initialisation expression of a variable declaration
// is not in the scope of the declared variable. Technically, a compiler would
// always first traverse the initialisation expression in the existing scope
// before creating a new variable a and updating the current scope accordingly.



/*
 * Traversal functions
 */


node *REglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("REglobaldec");

    DBUG_RETURN(arg_node);
}

node *REglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("REglobaldef");

    DBUG_RETURN(arg_node);
}

node *REvardeclaration(node *arg_node, info *arg_info)
{
	DBUG_ENTER("REvardeclaration");

	node *entry = VARDECLARATION_SYMBOLTABLEENTRY(arg_node);

	while (entry != NULL)
    {        
    	CTInote("in while loop! Name: %s. Scope: %i.", SYMBOLTABLEENTRY_NAME(entry), SYMBOLTABLEENTRY_SCOPE(entry));
        entry = SYMBOLTABLEENTRY_NEXT(entry);
    }


    // // hoe kom ik bij die symboltable....
    // while (SYMBOLTABLE_PREV(arg_node) != NULL)
    // {
    // 	// move to one scope up
    // 	CTInote("DONT CRASH");

    	// if (checkPrevDeclaration(SYMBOLTABLEENTRY_NEXT(arg_node), VARDECLARATION_NAME(arg_node)) == TRUE)
    	// {
    	// 	CTInote("%s has been declared in higher scope!", VARDECLARATION_NAME(arg_node));
    	// }
    // }

	// node *current;

	// while (SYMBOLTABLE_PREV(entry) != NULL)
	// {
	// 	// current = SYMBOLTABLE_PREV(arg_node);
	// 	// check elements in symboltable

	// 	CTInote("In prev ST");
	// }


 //    node *last = SYMBOLTABLE_PREV(arg_node);

	// if (last == NULL)
	// 	return NULL;

 //    while (SYMBOLTABLEENTRY_NEXT(trav) != NULL)
 //        trav = SYMBOLTABLEENTRY_NEXT(trav);

 //    return trav;

    // if (last != NULL)
    	// SYMBOLTABLEENTRY_NEXT(last);

	// SYMBOLTABLEENTRY_SCOPE()

	// VARDECLARATION_INIT();

	// check if declared before in higher scope
	// or check if dec name occurs in init name.. if yes; check higher scopes
	// VARDECLARATION_NAME(arg_node) ....
		// if yes; store and use this variable if it is used in expression


	DBUG_RETURN(arg_node);
}

/*
 * Helper functions
 */

// // check if earlier declaration is found
// node *travList(node *symboltableentry)
// {
// 	node *trav = symboltableentry;
// 	if (trav == NULL)
// 		return NULL;

//     while (SYMBOLTABLEENTRY_NEXT(trav) != NULL)
//         trav = SYMBOLTABLEENTRY_NEXT(trav);

//     return trav;
// }


bool checkPrevDeclaration(node *symboltableentry, char *name)
{
    node *trav = symboltableentry;

    while (trav != NULL)
    {
        if (STReq(SYMBOLTABLEENTRY_NAME(trav), name) == TRUE)
        	// found another initialisation
            return TRUE;
        else
            trav = SYMBOLTABLEENTRY_NEXT(trav);
    }

    // did not find another initialisation
    return FALSE;
}


/*
 * Traversal start function
 */

node *REmakeRegularExpressions(node *syntaxtree)
{
    DBUG_ENTER("REmakeRegularExpressions");

    // info *arg_info;
    // arg_info = MakeInfo();

    TRAVpush(TR_re);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    CTInote("Traversing done...\n");

    // if (INFO_ERRORS(arg_info) != 0)
    //     CTIabort("Found %i errors during the context analysis. Aborting the compilation.\n", INFO_ERRORS(arg_info));

    // arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}