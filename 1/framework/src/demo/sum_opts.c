/*
 * Andrea van den Hooff
 * 10439080
 * Premaster Software Engineering
 * 
 * Module: sum_opts
 * Prefix: SO
 * Description:
 * This module implements a traversal of the abstract syntax tree that 
 * counts all arithmetic operators and prints the result at the end of the traversal.
 */

#include "sum_opts.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"

#include "memory.h"
#include "ctinfo.h"

/* INFO structure. */
struct INFO
{
	int SumAdd;
	int SumSub;
	int SumMul;
	int SumDiv;
	int SumMod;
};

/* INFO macros. */
#define INFO_SUMADD(n) ((n)->SumAdd)
#define INFO_SUMSUB(n) ((n)->SumSub)
#define INFO_SUMMUL(n) ((n)->SumMul)
#define INFO_SUMDIV(n) ((n)->SumDiv)
#define INFO_SUMMOD(n) ((n)->SumMod)

/* Create the info struct. */
static info *MakeInfo(void)
{
	info *result;
	DBUG_ENTER("MakeInfo");
	result = (info *)MEMmalloc(sizeof(info));

	INFO_SUMADD(result) = 0;
	INFO_SUMSUB(result) = 0;
	INFO_SUMMUL(result) = 0;
	INFO_SUMDIV(result) = 0;
	INFO_SUMMOD(result) = 0;

	DBUG_RETURN(result);
}

/* Free the info struct. */
static info *FreeInfo(info *info)
{
	DBUG_ENTER("FreeInfo");
	info = MEMfree(info);
	DBUG_RETURN(info);
}

/* Create the module. */
static node *MakeModule(node *syntaxtree)
{
	node *counters;
	DBUG_ENTER("MakeModule");
	counters = TBmakeModule(0, 0, 0, 0, 0, syntaxtree);

	DBUG_RETURN(counters);
}

/*  Update the values of module. */
node *SOmodule(node *arg_node, info *arg_info)
{
	DBUG_ENTER("SOmodule");

	MODULE_STMTS(arg_node) = TRAVdo(MODULE_STMTS(arg_node), arg_info);

	MODULE_ADDITION(arg_node) = INFO_SUMADD(arg_info);
	MODULE_SUBTRACTION(arg_node) = INFO_SUMSUB(arg_info);
	MODULE_MULTIPLICATION(arg_node) = INFO_SUMMUL(arg_info);
	MODULE_DIVISION(arg_node) = INFO_SUMDIV(arg_info);
	MODULE_MODULO(arg_node) = INFO_SUMMOD(arg_info);

	DBUG_RETURN(arg_node);
}

/* Traversal functions. */
node *SObinop(node *arg_node, info *arg_info)
{
	DBUG_ENTER("SObinop");

	BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
	BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

	/* Look up type. */
	switch (BINOP_OP(arg_node))
	{
	case BO_add:
		INFO_SUMADD(arg_info) += 1;
		break;
	case BO_sub:
		INFO_SUMSUB(arg_info) += 1;
		break;
	case BO_mul:
		INFO_SUMMUL(arg_info) += 1;
		break;
	case BO_div:
		INFO_SUMDIV(arg_info) += 1;
		break;
	case BO_mod:
		INFO_SUMMOD(arg_info) += 1;
		break;
	default:
		break;
	}

	DBUG_RETURN(arg_node);
}

/* Start the traversal. */
node *SOdoSumOpts(node *syntaxtree)
{
	info *arg_info;
	node *arg_module;
	DBUG_ENTER("SOdoSumOpts");

	arg_info = MakeInfo();
	arg_module = MakeModule(syntaxtree);

	TRAVpush(TR_so);
	syntaxtree = TRAVdo(arg_module, arg_info);
	TRAVpop();

	CTInote("Total additions: %d", MODULE_ADDITION(syntaxtree));
	CTInote("Total subtractions: %d", MODULE_SUBTRACTION(syntaxtree));
	CTInote("Total multiplications: %d", MODULE_MULTIPLICATION(syntaxtree));
	CTInote("Total divisions: %d", MODULE_DIVISION(syntaxtree));
	CTInote("Total modulos: %d", MODULE_MODULO(syntaxtree));

	arg_info = FreeInfo(arg_info);
	DBUG_RETURN(syntaxtree);
}
