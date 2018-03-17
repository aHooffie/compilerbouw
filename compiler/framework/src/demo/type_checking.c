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

#include <stdio.h> // for standard error message?? eventueel niet meer nodig
#include <string.h>

#include "add_symboltables.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "lookup_table.h"
#include "add_symboltables.h"

#include "memory.h"
#include "ctinfo.h"

/*
 * INFO structure
 */

struct INFO
{
    int errors;
};


/*
 * INFO macros
 */

#define INFO_ERRORS(n) ((n)->errors)


/*
 * INFO functions
 */

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


/*
 * Traversal functions
 */


// !! NOG EVEN LETTEN OP VOID !!

node *TCbinop (node *arg_node, info *arg_info)
{
	DBUG_ENTER("TCbinop");

	BINOP_LEFT( arg_node) = TRAVdo( BINOP_LEFT( arg_node), arg_info);
	BINOP_RIGHT( arg_node) = TRAVdo( BINOP_RIGHT( arg_node), arg_info);

	// check if modulo
	if (BINOP_OP( arg_node) == BO_mod)
	{
		// then only int
		if (NODE_TYPE(BINOP_LEFT(arg_node)) != T_int || NODE_TYPE(BINOP_RIGHT(arg_node)) != T_int)
	  	{
	  		CTInote("Modulo can only be performed on two integers.\n");
	  		INFO_ERRORS(arg_info) += 1;
	  	}
	}

	else if (NODE_TYPE(BINOP_LEFT(arg_node)) =! NODE_TYPE(BINOP_RIGHT(arg_node)))
	{
		CTInote("Type error: types dont match :-(\n");

		INFO_ERRORS(arg_info) += 1;
	}

	DBUG_RETURN( arg_node);
}


node *TCparameters (node *arg_node, info *arg_info)
{
	DBUG_ENTER("TCparameters");

	node *entry = PARAMETERS_SYMBOLTABLEENTRY( arg_node);

	DBUG_RETURN( arg_node);
}


/*
 * Traversal start function
 */

node *TCdoTypeChecking(node *syntaxtree)
{
    DBUG_ENTER("TCdoTypeChecking");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_tc); 							// push traversal "tc" as defined in ast.xml
    syntaxtree = TRAVdo(syntaxtree, arg_info);  // initiate ast traversal
    TRAVpop();									// pop current traversal

    CTInote("Traversing for TC done...\n");

    if (INFO_ERRORS(arg_info) != 0)
    {
        CTIabort("Found %i error(s) during type checking. Aborting the compilation.\n", INFO_ERRORS(arg_info));
    }

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}