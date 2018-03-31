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
    int branchcount; // counter to check what branch voorstuk should be used in labels
};

/* INFO structure macros */
#define INFO_FI(n) ((n)->firstinstruction)
#define INFO_LI(n) ((n)->lastinstruction)
#define INFO_CONSTANTS(n) ((n)->constants)
#define INFO_VARIABLES(n) ((n)->constants)
#define INFO_VC(n) ((n)->variablecount)
#define INFO_CC(n) ((n)->constantcount)
#define INFO_BC(n) ((n)->branchcount)

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
    INFO_BC(result) = 0;

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
        // ADD AND/OR
        case BO_add:
            n = TBmakeInstructions(I_badd, NULL);
            break;
        case BO_mul:
            n = TBmakeInstructions(I_bmul, NULL);
            break;
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info)
{

    // !! TO DO: LAATSTE TWEE CASES

    DBUG_ENTER("GBCbinop");

    // NULL to silence warning..
    node *n = NULL;

    // traverse expression
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    nodetype left = NODE_TYPE(BINOP_LEFT(arg_node));

    printf("node line: %i\n", NODE_LINE(arg_node));

    // find out binop type
    switch(BINOP_OP(arg_node))
    {
        case BO_add:
            if (left == N_num)
                // >>> optimize : met iinc??
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
            else if (left == N_float)
                n = TBmakeInstructions(I_feq, NULL);
            else
                n = TBmakeInstructions(I_beq, NULL);
            break;

        case BO_ne:
            if (left == N_num)
                n = TBmakeInstructions(I_ine, NULL);
            else if (left == N_float)
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
    node *n;

    // traverse expression
    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);

    if (CAST_TYPE(arg_node) == T_int)
        n = TBmakeInstructions(I_f2i, NULL);
    else
        n = TBmakeInstructions(I_i2f, NULL);
    
    /* Add the node to the list of instructions. */
    addNode(n, arg_info); 
    DBUG_RETURN(arg_node);
}

node *GBCvar(node *arg_node, info *arg_info)
{

    DBUG_ENTER("GBCvar");

    node *n;
    int index;

    // store var name
    char *name = STRcpy(VAR_NAME(arg_node));

    // search for index
    for(int i = 0; i < 256; i++)
    {
        if (VARDECLARATION_NAME(INFO_VARIABLES(arg_info)[i]) == name)
        {
            // store index
            index = i;
            break;
        }
    }

    // what if someting went wrong and index was never found?

    // klopt het wel om INFO_VARIABLES(arg_info)[index] te gebruiken????

    if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(arg_node)) == T_int)
        n = TBmakeInstructions(I_iloadc, INFO_VARIABLES(arg_info)[index]);
    else if (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(arg_node)) == T_float)
        n = TBmakeInstructions(I_floadc, INFO_VARIABLES(arg_info)[index]);
    else
        n = TBmakeInstructions(I_bloadc, INFO_VARIABLES(arg_info)[index]);

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

node *GBCnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCnum");
    node *n;

    CTInote("in num!\n");

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
    CTInote("adding node\n");
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

    TRAVpush(TR_gbc);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    printInstructions(arg_info);

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}


/* Helper functions */
char *instrToString(instr type)
{
    char *s = NULL;
    switch (type)
    {
    case I_iadd:
        s = "iadd";
        break;
    case I_fadd: { s = "fadd"; break; }
    case I_isub:
        s = "isub";
        break;
    case I_fsub:
        s = "fsub";
        break;
    case I_imul:
        s = "imul";
        break;
    case I_fmul:
        s = "fmul";
        break;
    case I_idiv:
        s = "idiv";
        break;
    case I_fdiv:
        s = "fdiv";
        break;
    case I_irem:
        s = "irem";
        break;
    case I_ineg:
        s = "ineg";
        break;
    case I_fneg:
        s = "fneg";
        break;
    case I_bnot:
        s = "bnot";
        break;
    case I_iinc:
        s = "iinc";
        break;
    case I_iinc_1:
        s = "iinc_1";
        break;
    case I_idec:
        s = "idec";
        break;
    case I_idec_1:
        s = "idec_1";
        break;
    case I_badd:
        s = "badd";
        break;
    case I_bmul:
        s = "bmul";
        break;
    case I_ine:
        s = "ine";
        break;
    case I_ieq:
        s = "ieq";
        break;
    case I_ilt:
        s = "ilt";
        break;
    case I_ile:
        s = "ile";
        break;
    case I_igt:
        s = "igt";
        break;
    case I_ige:
        s = "ige";
        break;
    case I_fne:
        s = "fne";
        break;
    case I_feq:
        s = "feq";
        break;
    case I_flt:
        s = "flt";
        break;
    case I_fle:
        s = "fle";
        break;
    case I_fgt:
        s = "fgt";
        break;
    case I_fge:
        s = "fge";
        break;
    case I_bne:
        s = "bne";
        break;
    case I_beq:
        s = "beq";
        break;
    case I_isr:
        s = "isr";
        break;
    case I_isrn:
        s = "isrn";
        break;
    case I_isrl:
        s = "isrl";
        break;
    case I_isrg:
        s = "isrg";
        break;
    case I_jsr:
        s = "jsr";
        break;
    case I_jsre:
        s = "jsre";
        break;
    case I_esr:
        s = "esr";
        break;
    case I_ireturn:
        s = "ireturn";
        break;
    case I_freturn:
        s = "freturn";
        break;
    case I_breturn:
        s = "breturn";
        break;
    case I_return:
        s = "return";
        break;
    case I_jump:
        s = "jump";
        break;
    case I_branch_t:
        s = "branch_t";
        break;
    case I_branch_f:
        s = "branch_f";
        break;
    case I_iload:
        s = "iload";
        break;
    case I_iload_0:
        s = "iload_0";
        break;
    case I_iload_1:
        s = "iload_1";
        break;
    case I_iload_2:
        s = "iload_2";
        break;
    case I_iload_3:
        s = "iload_3";
        break;
    case I_fload:
        s = "fload";
        break;
    case I_fload_0:
        s = "fload_0";
        break;
    case I_fload_1:
        s = "fload_1";
        break;
    case I_fload_2:
        s = "fload_2";
        break;
    case I_fload_3:
        s = "fload_3";
        break;
    case I_bload:
        s = "bload";
        break;
    case I_bload_0:
        s = "bload_0";
        break;
    case I_bload_1:
        s = "bload_1";
        break;
    case I_bload_2:
        s = "bload_2";
        break;
    case I_bload_3:
        s = "bload_3";
        break;
    case I_iloadn:
        s = "iloadn";
        break;
    case I_floadn:
        s = "floadn";
        break;
    case I_bloadn:
        s = "bloadn";
        break;
    case I_iloadg:
        s = "iloadg";
        break;
    case I_floadg:
        s = "floadg";
        break;
    case I_bloadg:
        s = "bloadg";
        break;
    case I_iloade:
        s = "iloade";
        break;
    case I_floade:
        s = "floade";
        break;
    case I_bloade:
        s = "bloade";
        break;
    case I_iloadc:
        s = "iloadc";
        break;
    case I_floadc:
        s = "floadc";
        break;
    case I_bloadc:
        s = "bloadc";
        break;
    case I_iloadc_0:
        s = "iloadc_0";
        break;
    case I_floadc_0:
        s = "floadc_0";
        break;
    case I_bloadc_t:
        s = "bloadc_t";
        break;
    case I_iloadc_1:
        s = "iloadc_1";
        break;
    case I_floadc_1:
        s = "floadc_1";
        break;
    case I_bloadc_f:
        s = "bloadc_f";
        break;
    case I_iloadc_m1:
        s = "iloadc_m1";
        break;
    case I_istore:
        s = "istore";
        break;
    case I_fstore:
        s = "fstore";
        break;
    case I_bstore:
        s = "bstore";
        break;
    case I_istoren:
        s = "istoren";
        break;
    case I_fstoren:
        s = "fstoren";
        break;
    case I_bstoren:
        s = "bstoren";
        break;
    case I_istoreg:
        s = "istoreg";
        break;
    case I_fstoreg:
        s = "fstoreg";
        break;
    case I_bstoreg:
        s = "bstoreg";
        break;
    case I_istoree:
        s = "istoree";
        break;
    case I_fstoree:
        s = "fstoree";
        break;
    case I_bstoree:
        s = "bstoree";
        break;
    case I_i2f:
        s = "i2f";
        break;
    case I_f2i:
        s = "f2i";
        break;
    case I_ipop:
        s = "ipop";
        break;
    case I_fpop:
        s = "fpop";
        break;
    case I_bpop:
        s = "bpop";
        break;
    case I_ownbranch:
        s = "";
    case I_unknown:
        CTIabort("Unknown instruction type.");
        break;
    default:
        CTIabort("Unknown instruction type.");
        break;
    }
    return s;
}


void printInstructions(info *arg_info)
{
    node *n = INFO_FI(arg_info);

    // TO DO: INDENTATIE + LABELS WITH :

    /* Print all instructions */
    while (n != NULL)
    {
        printf("%s", instrToString(INSTRUCTIONS_INSTR(n)));

        // if (INSTRUCTIONS_OFFSET(n) != NULL)
            printf(" %i", INSTRUCTIONS_OFFSET(n));

        // if (INSTRUCTIONS_ARG(n) != NULL)
            printf(" %s", INSTRUCTIONS_ARG(n));

        printf("\n");

        n = INSTRUCTIONS_NEXT(n);
    }

    /* Print the last instruction. */
    printf("%s", instrToString(INSTRUCTIONS_INSTR(n)));
    // if (INSTRUCTIONS_OFFSET(n) != NULL)
        printf(" %i", INSTRUCTIONS_OFFSET(n));

    // if (INSTRUCTIONS_ARG(n) != NULL)
        printf(" %s", INSTRUCTIONS_ARG(n));

    printf("\n");

    /* Print mogelijk nog andere instructies met een . (voor later) */
}
