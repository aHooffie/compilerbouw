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
    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCbinop");
    DBUG_RETURN(arg_node);
}

node *GBCmonop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCmonop");
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
