#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"

#include "memory.h"
#include "ctinfo.h"

/* 

    Milestone 12: Assembly Code Generation for Expressions and Statements
    Implement a code generator that transforms your internal representation into a 
    at sequence of CiviC-VM assembly instructions, pseudo instructions and labels. For this milestone leave out the
    function call interface and restrict yourself to the body of the main function.

    Statements: ifelse for while dowhile return assign (functioncallstmt)
    Expressions: ternop binop monop var num float cast bool (functioncallexpr arrayexpr)

*/

/* INFO structure */
struct INFO
{
    node *firstinstruction; // pointer to the first of the list (for the final printing, to be able to traverse)
    node *lastinstruction; // pointer to the last of the list (to add a new node to (see function AddNode))
    node *constants[256]; // array of integers and floats (NODES!!! not values!! ) to store / load from
    node *variables[256]; // array of variables (NODES!!! not strings !! ) to store / load from
    int constantcount; // counter to check what indices are filled in the constant array (if you add one, up this with 1)
    int variablecount; // counter to check what indices are filled in the variable array
};

/* INFO structure macros */
#define INFO_FI(n) ((n)->firstinstruction)
#define INFO_LI(n) ((n)->lastinstruction)
#define INFO_CONSTANTS(n) ((n)->constants)
#define INFO_VARIABLES(n) ((n)->constants)
#define INFO_VC(n) ((n)->variablecount)
#define INFO_CC(n) ((n)->constantcount)

/* INFO functions */
static info *MakeInfo(void)
{
    DBUG_ENTER("MakeInfo");

    info *result;
    result = (info *)MEMmalloc(sizeof(info));
    INFO_FI(result) = NULL;
    INFO_LI(result) = NULL;
    INFO_VC(result) = 0;
    INFO_CC(result) = 0;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/* Expressions */
node *GBCternop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCternop");

    node *n;

    // traverse expression
    TERNOP_CONDITION(arg_node) = TRAVdo(TERNOP_CONDITION(arg_node), arg_info);
    TERNOP_THEN(arg_node) = TRAVdo(TERNOP_THEN(arg_node), arg_info);
    TERNOP_ELSE(arg_node) = TRAVdo(TERNOP_ELSE(arg_node), arg_info);

    switch(TERNOP_OP(arg_node))
    {

        // !!!!! KLOPT DIT? NALOPEN
        case BO_add:
            n = TBmakeInstructions(I_badd, NULL);
        case BO_mul:
            n = TBmakeInstructions(I_bmul, NULL);
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info)
{

    // !! TO DO: LAATSTE TWEE CASES
    
    DBUG_ENTER("GBCbinop");

    node *n;

    // traverse expression
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    nodetype left = NODE_TYPE(BINOP_LEFT(arg_node));

    // find out what binop
    switch(BINOP_OP(arg_node))
    {
        case BO_add:
            if (left == N_num)
                n = TBmakeInstructions(I_iadd, NULL);
            else
                n = TBmakeInstructions(I_fadd, NULL);
            break;

        case BO_sub:
            if (left == N_num)
                n = TBmakeInstructions(I_isub, NULL);
            else
                n = TBmakeInstructions(I_fsub, NULL);
            break;

        case BO_mul:
            if (left == N_num)
                n = TBmakeInstructions(I_imul, NULL);
            else
                n = TBmakeInstructions(I_fmul, NULL);
            break;

        case BO_div:
            if (left == N_num)
                n = TBmakeInstructions(I_idiv, NULL);
            else
                n = TBmakeInstructions(I_fdiv, NULL);
            break;

        case BO_mod:
            n = TBmakeInstructions(I_irem, NULL);
            break;

        case BO_lt:
            if (left == N_num)
                n = TBmakeInstructions(I_ilt, NULL);
            else
                n = TBmakeInstructions(I_flt, NULL);
            break;

        case BO_le:
            if (left == N_num)
                n = TBmakeInstructions(I_ile, NULL);
            else
                n = TBmakeInstructions(I_fle, NULL);
            break;

        case BO_gt:
            if (left == N_num)
                n = TBmakeInstructions(I_igt, NULL);
            else
                n = TBmakeInstructions(I_fgt, NULL);
            break;

        case BO_ge:
            if (left == N_num)
                n = TBmakeInstructions(I_ige, NULL);
            else
                n = TBmakeInstructions(I_fge, NULL);
            break;

        case BO_eq:
            if (left == N_num)
                n = TBmakeInstructions(I_ieq, NULL);
            else if (left == N_FLOAT)
                n = TBmakeInstructions(I_feq, NULL);
            else
                n = TBmakeInstructions(I_beq, NULL);
            break;

        case BO_ne:
            if (left == N_num)
                n = TBmakeInstructions(I_ine, NULL);
            else if (left == N_FLOAT)
                n = TBmakeInstructions(I_fne, NULL);
            else
                n = TBmakeInstructions(I_bne, NULL);
            break;

        case BO_and:
            break;

        case BO_or:
            break;

    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCmonop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCmonop");

    node *n;

    // traverse expression
    MONOP_EXPR(arg_node) = TRAVdo(MONOP_EXPR(arg_node), arg_info);
    nodetype type = NODE_TYPE(MONOP_EXPR(arg_node));

    switch (MONOP_OP(arg_node))
    {
        case MO_neg:
            if (type == N_num)
                n = TBmakeInstructions(I_ineg, NULL);
            else
                n = TBmakeInstructions(I_fneg, NULL);

        case MO_not:
            n = TBmakeInstructions(I_bnot, NULL);
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCcast(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCcast");
    DBUG_RETURN(arg_node);
}

node *GBCvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCvar");
    // find var in table
    // makeinstruction (iloadc, index)
    DBUG_RETURN(arg_node);
}

node *GBCnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCnum");
    node *n;

    /* If the value of the integer is either 0 or 1, create a basic instruction, otherwise a custom instruction. */
    if (NUM_VALUE(arg_node) == 0)
        n = TBmakeInstructions(I_iloadc_0, NULL);
    else if (NUM_VALUE(arg_node) == 1)
        n = TBmakeInstructions(I_iloadc_1, NULL);
    else {
        INFO_CONSTANTS(arg_info)[INFO_CC(arg_info)] = arg_node;
        n = TBmakeInstructions(I_iloadc, NULL);

        /* Add the indices to the right place in the array to the instruction. */
        INSTRUCTIONS_INSTR(n) = INFO_CC(arg_info);
        INFO_CC(arg_info) += 1;
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

node *GBCfloat(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCfloat");
    node *n;

    /* If the value of the integer is either 0 or 1, create a basic instruction, otherwise a custom instruction. */
    if (FLOAT_VALUE(arg_node) == 0.0)
        n = TBmakeInstructions(I_fload_0, NULL);
    else if (FLOAT_VALUE(arg_node) == 1.0)
        n = TBmakeInstructions(I_fload_1, NULL);
    else
    {
        INFO_CONSTANTS(arg_info)[INFO_CC(arg_info)] = arg_node;
        n = TBmakeInstructions(I_floadc, NULL);

        /* Add the indices to the right place in the array to the instruction. */
        INSTRUCTIONS_INSTR(n) = INFO_CC(arg_info);
        INFO_CC(arg_info) += 1;
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

node *GBCbool(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCbool");
    node *n;

    if (BOOL_VALUE(arg_node) == TRUE)
        n = TBmakeInstructions(I_bloadc_t, NULL);
    else
        n = TBmakeInstructions(I_bloadc_f, NULL);

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

/* If the list is empty, add the new node as both head and tail. Otherwise, add it to the end and update the previous last node. */
void addNode(node *arg_node, info *arg_info)
{
    if (INFO_LI(arg_info) == NULL)
    {
        INFO_FI(arg_info) = arg_node;
        INFO_LI(arg_info) = arg_node;
    }
    else
    {
        INSTRUCTIONS_NEXT(INFO_LI(arg_info)) = arg_node;
        INFO_LI(arg_info) = arg_node;
    }
}

/* Traversal start function */
node *GBCdoGenByteCode(node *syntaxtree)
{
    DBUG_ENTER("GBCdoGenByteCode");

    info *arg_info;
    arg_info = MakeInfo();

    TRAVpush(TR_as);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
