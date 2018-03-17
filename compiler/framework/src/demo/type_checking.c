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
#include <stdio.h> // for standard error message
#include <string.h>

#include "add_symboltables.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "lookup_table.h"

#include "memory.h"
#include "ctinfo.h"




/*
 * Traversal functions
 */

node *TCbinop (node *arg_node, info *arg_info)
{
  DBUG_ENTER("TCbinop");

  /*
   * Extremely important:
   *  we must continue to traverse the abstract syntax tree !!
   */

  BINOP_LEFT( arg_node) = TRAVdo( BINOP_LEFT( arg_node), arg_info);
  BINOP_RIGHT( arg_node) = TRAVdo( BINOP_RIGHT( arg_node), arg_info);

  if (NODE_TYPE(BINOP_LEFT(arg_node)) != NODE_TYPE(BINOP_RIGHT(arg_node)))
  {
  	// ERROR
  	fprintf(stderr, "Types dont match :-(\n");
  }

 //  if (BINOP_OP( arg_node) == BO_sub) {
 //    if ((NODE_TYPE( BINOP_LEFT( arg_node)) == N_var)
	// && (NODE_TYPE( BINOP_RIGHT( arg_node)) == N_var)
	// && STReq( VAR_NAME( BINOP_LEFT( arg_node)), VAR_NAME( BINOP_RIGHT( arg_node)))) {
 //      arg_node = FREEdoFreeTree( arg_node);
 //      arg_node = TBmakeNum( 0);
 //    }
 //    else if  ((NODE_TYPE( BINOP_LEFT( arg_node)) == N_num)
	//       && (NODE_TYPE( BINOP_RIGHT( arg_node)) == N_num)
	//       && (NUM_VALUE( BINOP_LEFT( arg_node)) == NUM_VALUE( BINOP_RIGHT( arg_node)))) {
 //      arg_node = FREEdoFreeTree( arg_node);
 //      arg_node = TBmakeNum( 0);
 //    }
 //  }

  DBUG_RETURN( arg_node);
}



/*
 * Traversal start function
 */

node *TCdoTypeChecking(node *syntaxtree)
{
    DBUG_ENTER("TCdoTypeChecking");

    // info *arg_info;
    // arg_info = MakeInfo();

    TRAVpush(TR_tc); 						// Push traversal "tc" as defined in ast.xml
    syntaxtree = TRAVdo(syntaxtree, NULL);  // Initiate ast traversal
    TRAVpop();								// Pop current traversal

    CTInote("Traversing for TC done...\n");

    // arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}