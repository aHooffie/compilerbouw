#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"
// #include "add_symboltables.h"

#include "memory.h"
#include "ctinfo.h"

extern char *TypetoString(type Type);

/* 
    Milestone 15:
    To do: that it takes advantage of specialised instructions to reduce the
size of the corresponding byte code (OPTIM)


    Milestone 14:
    To do: separate compilation of CiviC modules = extern functions!


    Milestone 13:
    To do: the full function call protocol of the CiviC-VM, but leave
out support for multiple modules.


    Milestone 12: 
    To do standard: vardeclarations, for loop, ternop
    To check: isrg/branch in alle loops goed. 
    To edit: vars, nums, floats in arrays niet dubbel
    To do extensions: functioncallstmt, functioncallexpr
    Done: ifelse while dowhile return assign binop monop var num float cast bool
    
// NB: How many different arrays do we need? export / extern / global etc?

*/

/* INFO structure */
struct INFO
{
    node *firstinstruction; // pointer to the first of the list (for the final printing, to be able to traverse)
    node *lastinstruction;  // pointer to the last of the list (to add a new node to (see function AddNode))
    node *constants[256];   // array of integers and floats (NODES!!! not values!! ) to store / load from
    node *variables[256];   // array of variables (NODES!!! not strings !! ) to store / load from
    node *exportfun[256];   // array of functions (NODES!!! not strings !! ) to export
    node *importvar[256];
    node *variablesexp[256]; // array of variables (NODES!!! not strings !! ) to store / load from
    int constantcount;       // counter to check what indices are filled in the constant array (if you add one, up this with 1)
    int varcount;            // counter to check what indices are filled in the variable array
    int varexpcount;         // counter to check what indices are filled in the variable array
    int exportfuncount;      // counter to check what indices are filled in the constant array (if you add one, up this with 1)
    int importvarcount;
    int branchcount; // counter to check what branch voorstuk should be used in labels

    int size; // FOR TESTING!
    int localvarcount;
};

/* INFO structure macros */
#define INFO_FI(n) ((n)->firstinstruction)
#define INFO_LI(n) ((n)->lastinstruction)
#define INFO_CONSTANTS(n) ((n)->constants)
#define INFO_VARIABLES(n) ((n)->variables)
#define INFO_EXPORTFUN(n) ((n)->exportfun)
#define INFO_IMPORTVAR(n) ((n)->importvar)
#define INFO_VARIABLESEXP(n) ((n)->variablesexp)
#define INFO_VC(n) ((n)->varcount)
#define INFO_VEC(n) ((n)->varexpcount)
#define INFO_CC(n) ((n)->constantcount)
#define INFO_BC(n) ((n)->branchcount)
#define INFO_EFC(n) ((n)->exportfuncount)
#define INFO_IVC(n) ((n)->importvarcount)
#define INFO_LC(n) ((n)->localvarcount)
#define INFO_SIZE(n) ((n)->size)

/* INFO functions */
static info *MakeInfo(void)
{
    DBUG_ENTER("MakeInfo");

    info *result;
    result = (info *)MEMmalloc(sizeof(info));
    INFO_FI(result) = NULL;
    INFO_LI(result) = NULL;
    INFO_VC(result) = 0;
    INFO_VEC(result) = 0;
    INFO_CC(result) = 0;
    INFO_BC(result) = 0;
    INFO_EFC(result) = 0;
    INFO_IVC(result) = 0;
    INFO_LC(result) = 0;
    INFO_SIZE(result) = 0;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

/* Globaldef: also exported! */
// Does this require a different array?
node *GBCglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCglobaldef");

    // /* Traverse into array subtree - not implemented. */
    // if (GLOBALDEF_DIMENSIONS(arg_node) != NULL)
    //     GLOBALDEF_DIMENSIONS(arg_node) = TRAVdo(GLOBALDEF_DIMENSIONS(arg_node), arg_info);

    // /* Add variable to variable array

    // -- possible new global array ? */
    // INFO_VARIABLES(arg_info)[INFO_VC(arg_info)] = arg_node;
    // INFO_VC(arg_info) += 1;

    //  // Check if variable should be exported, add it to that array too.
    // if (GLOBALDEF_ISEXPORT(arg_node) == TRUE)
    // {
    //     INFO_VARIABLESEXP(arg_info)[INFO_VEC(arg_info)] = arg_node;
    //     INFO_VEC(arg_info) += 1;
    // }

    // /* Traverse into assigning subtree. */
    // if (GLOBALDEF_ASSIGN(arg_node) != NULL)
    //     GLOBALDEF_ASSIGN(arg_node) = TRAVdo(GLOBALDEF_ASSIGN(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

// NB: EXTERN!!!!!!!! HIER MOET IETS MEE?
// ja denk wel: aparte instructies voor (zoals iloade)
node *GBCglobaldec(node *arg_node, info *arg_info)
{
    // NOG NIET GETEST

    DBUG_ENTER("GBCglobaldec");
    // node *n;

    /* Traverse into array subtree - not implemented. */
    GLOBALDEC_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEC_DIMENSIONS(arg_node), arg_info);

    /* Add variable to variable array. */
    INFO_VARIABLES(arg_info)
    [INFO_VC(arg_info)] = arg_node;
    INFO_IMPORTVAR(arg_info)
    [INFO_IVC(arg_info)] = arg_node;

    // if (GLOBALDEC_TYPE(arg_node) == T_int)
    //     n = TBmakeInstructions(I_iloadg, NULL);
    // else if (GLOBALDEC_TYPE(arg_node) == T_float)
    //     n = TBmakeInstructions(I_floadg, NULL);
    // else
    //     n = TBmakeInstructions(I_bloadg, NULL);

    // INSTRUCTIONS_OFFSET(n) = INFO_VC(arg_info);

    INFO_VC(arg_info) += 1;
    INFO_IVC(arg_info) += 1;
    // addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

// FUNC EXPORT???!!!!
node *GBCfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCfunction");
    node *n;

    /* Add the labelname to the linked list.*/
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARG(n) = FUNCTION_NAME(arg_node);
    addNode(n, arg_info);

    /* Create esr instruction. */
    n = TBmakeInstructions(I_esr, NULL);
    addNode(n, arg_info);

    /* Traverse into child nodes. */
    FUNCTION_PARAMETERS(arg_node) = TRAVopt(FUNCTION_PARAMETERS(arg_node), arg_info);
    FUNCTION_FUNCTIONBODY(arg_node) = TRAVopt(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    /* Add offset to esr, reset variablecount for next function. */
    INSTRUCTIONS_OFFSET(n) = INFO_LC(arg_info);
    INFO_LC(arg_info) = 0;

    /* Check if function needs to be exported. */
    if (FUNCTION_ISEXPORT(arg_node) == TRUE)
    {
        INFO_EXPORTFUN(arg_info)
        [INFO_EFC(arg_info)] = arg_node;
        INFO_EFC(arg_info) += 1;
    }

    DBUG_RETURN(arg_node);
}

node *GBCparameters(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCparameters");

    INFO_VARIABLES(arg_info)
    [INFO_VC(arg_info)] = arg_node;
    INFO_VC(arg_info) += 1;

    PARAMETERS_NEXT(arg_node) = TRAVopt(PARAMETERS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCvardeclaration");

    INFO_LC(arg_info) += 1;

    /* Add variable to variable array. */
    INFO_VARIABLES(arg_info)
    [INFO_VC(arg_info)] = arg_node;
    INFO_VC(arg_info) += 1;

    VARDECLARATION_NEXT(arg_node) = TRAVopt(VARDECLARATION_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* Statements */
/* Ifelse */
node *GBCifelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCifelse");
    node *n;
    char *otherwise;
    char *end;
    char str[12];

    /* Traverse into if condition. */
    IFELSE_CONDITION(arg_node) = TRAVdo(IFELSE_CONDITION(arg_node), arg_info);
    INFO_BC(arg_info) += 1;

    /* Check if there is an else block of statements. */
    if (IFELSE_ELSE(arg_node) != NULL)
    {
        sprintf(str, "%d", INFO_BC(arg_info));
        otherwise = STRcat(str, "_else");

        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_branch_f, NULL);
        INSTRUCTIONS_ARG(n) = otherwise;
        addNode(n, arg_info);

        /* Traverse into if block. */
        IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);

        /* Create jump instruction. */
        INFO_BC(arg_info) += 1;
        sprintf(str, "%d", INFO_BC(arg_info));
        end = STRcat(str, "_end");

        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_jump, NULL);
        INSTRUCTIONS_ARG(n) = end;
        addNode(n, arg_info);

        /* Create the else label as instruction. */
        n = TBmakeInstructions(I_ownbranch, NULL);
        INSTRUCTIONS_ARG(n) = otherwise;
        addNode(n, arg_info);

        /* Traverse into else statements. */
        IFELSE_ELSE(arg_node) = TRAVdo(IFELSE_ELSE(arg_node), arg_info);
    }
    else
    {
        sprintf(str, "%d", INFO_BC(arg_info));
        end = STRcat(str, "_end");

        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_branch_f, NULL);
        INSTRUCTIONS_ARG(n) = end;
        addNode(n, arg_info);

        /* Traverse into block of statements. */
        IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);
    }

    /* Create the end label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARG(n) = end;
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

/* For - werkt als enige nog niet - uitgecomment! */
node *GBCfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCfor");
    // int step = 1;
    // node *n;

    // /* Check if the step size is custom. */
    // if (FOR_STEP(arg_node) != NULL)
    // {
    //     FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);

    //     if (NODE_TYPE(FOR_STEP(arg_node)) == N_num)
    //     {
    //         step = NUM_VALUE(FOR_STEP(arg_node));
    //         if (step == 0)
    //             CTIabort("Step size of a for loop cannot be 0.");
    //     }

    //     /* For-loop can be a negative integer. */
    //     else if (NODE_TYPE(FOR_STEP(arg_node)) == N_monop)
    //     {
    //         if (MONOP_OP(FOR_STEP(arg_node)) == MO_neg && NODE_TYPE(MONOP_EXPR(FOR_STEP(arg_node))) == N_num)
    //         {
    //             step = NUM_VALUE(MONOP_EXPR(FOR_STEP(arg_node))) * -1;
    //             if (step == 0.0)
    //                 CTIabort("Step size of a for loop cannot be 0.");
    //         }
    //     }
    // }

    // /* Create the starting label for a branch (1_while, 2_end etc. ) */
    // INFO_BC(arg_info) += 1;
    // char str[12];
    // char *start;

    // sprintf(str, "%d", INFO_BC(arg_info));
    // start = STRcat(str, "_while(FOR)");

    // /* Add the label as instruction. */
    // n = TBmakeInstructions(I_ownbranch, NULL);
    // INSTRUCTIONS_ARG(n) = start;
    // addNode(n, arg_info);

    // // SOMEHOW create while condition with
    // // TODO
    // FOR_START(arg_node) = TRAVdo(FOR_START(arg_node), arg_info);
    // FOR_STOP(arg_node) = TRAVdo(FOR_STOP(arg_node), arg_info);

    // /* Create comparison instruction. */
    // if (step < 0)
    // {
    //     n = TBmakeInstructions(I_igt, NULL);
    //     addNode(n, arg_info);
    // }
    // else
    // {
    //     n = TBmakeInstructions(I_ilt, NULL);
    //     addNode(n, arg_info);
    // }

    // // instruction to branch_f endlabel here
    // // isrg

    // /* Traverse through block. */
    // FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);

    // /* Choose right increment instruction */
    // if (step == 1)
    // {
    //     n = TBmakeInstructions(I_iinc_1, NULL);
    //     // ADD OFFSET OF VAR TO INCREASE WITH 1
    //     addNode(n, arg_info);
    // }
    // else if (step == -1)
    // {
    //     n = TBmakeInstructions(I_idec_1, NULL);
    //     // ADD OFFSET OF VAR TO INCREASE WITH 1
    //     addNode(n, arg_info);
    // }
    // else
    // {
    //     // TO DO
    // }

    // /* Create the jump label. */
    // n = TBmakeInstructions(I_jump, NULL);
    // INSTRUCTIONS_ARG(n) = start;
    // addNode(n, arg_info);

    // char *end = "TODO";
    // /* Add the ending label as instruction. */
    // n = TBmakeInstructions(I_ownbranch, NULL);
    // INSTRUCTIONS_ARG(n) = end;
    // addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

/* While */
// Check isrg / branching
node *GBCwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCwhile");
    node *n;
    char *start;
    char *end;
    char str[12];

    /* Create the starting label for a branch (1_while, 2_end etc. ) */
    INFO_BC(arg_info) += 1;
    sprintf(str, "%d", INFO_BC(arg_info));
    start = STRcat(str, "_while");

    /* Add the label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARG(n) = start;
    addNode(n, arg_info);

    /* Traverse into condition. */
    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);

    /* Create the ending label for the branch. */
    INFO_BC(arg_info) += 1;
    sprintf(str, "%d", INFO_BC(arg_info));
    end = STRcat(str, "_end");

    /* Add the branch_f labelname to the linked list.*/
    n = TBmakeInstructions(I_branch_f, NULL);
    INSTRUCTIONS_ARG(n) = end;
    addNode(n, arg_info);

    // Instruction for isrg here?
    n = TBmakeInstructions(I_isrg, NULL);
    addNode(n, arg_info);

    /* Traverse into block of statements. */
    WHILE_BLOCK(arg_node) = TRAVdo(WHILE_BLOCK(arg_node), arg_info);

    /* Create the jump label. */
    n = TBmakeInstructions(I_jump, NULL);
    INSTRUCTIONS_ARG(n) = end;
    addNode(n, arg_info);

    /* Add the ending label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARG(n) = end;
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

/* Dowhile node. */
// Check isrg / branching
node *GBCdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCdowhile");
    char *label;
    char str[12];
    node *n;

    /* Create the starting label for a branch (1_while, 2_end etc. ) */
    INFO_BC(arg_info) += 1;
    sprintf(str, "%d", INFO_BC(arg_info));
    label = STRcat(str, "_dowhile");

    /* Add the label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARG(n) = label;
    addNode(n, arg_info);

    /* Traverse into statements. */
    if (DOWHILE_BLOCK(arg_node) != NULL)
        DOWHILE_BLOCK(arg_node) = TRAVdo(DOWHILE_BLOCK(arg_node), arg_info);

    /* Traverse into condition. */
    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);

    /* Add the label as instruction. */
    n = TBmakeInstructions(I_branch_t, NULL);
    INSTRUCTIONS_ARG(n) = label;
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

/* Return node. */
node *GBCreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCreturn");
    node *n;

    type t = SYMBOLTABLEENTRY_TYPE(RETURN_SYMBOLTABLEENTRY(arg_node));

    /* Traverse into optional expression to return. */
    RETURN_EXPR(arg_node) = TRAVopt(RETURN_EXPR(arg_node), arg_info);

    /* Create return instruction, according to expression type. */
    if (t == T_int)
        n = TBmakeInstructions(I_ireturn, NULL);
    else if (t == T_float)
        n = TBmakeInstructions(I_freturn, NULL);
    else if (t == T_bool)
        n = TBmakeInstructions(I_breturn, NULL);
    else
        n = TBmakeInstructions(I_return, NULL);

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

/* Expressions */
node *GBCternop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCternop");

    node *n;

    /* Traverse expression */
    TERNOP_CONDITION(arg_node) = TRAVdo(TERNOP_CONDITION(arg_node), arg_info);
    TERNOP_THEN(arg_node) = TRAVdo(TERNOP_THEN(arg_node), arg_info);
    TERNOP_ELSE(arg_node) = TRAVdo(TERNOP_ELSE(arg_node), arg_info);

    switch (TERNOP_OP(arg_node))
    {
    // !!!!! KLOPT DIT? NALOPEN
    // ADD AND/OR
    case BO_add:
        n = TBmakeInstructions(I_badd, NULL);
        break;
    case BO_mul:
        n = TBmakeInstructions(I_bmul, NULL);
        break;
    default:
        n = NULL;
        CTInote("TO DO TERNOP");
        break;
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCbinop");
    node *n;

    /* Traverse expressions */
    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

    nodetype left = NODE_TYPE(BINOP_LEFT(arg_node));

    switch (BINOP_OP(arg_node))
    {
    case BO_add:
        if (left == N_num)
            // TO DO > optimize : met iinc??
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

    default:
        n = NULL;
        CTIabort("Unknown binop type.");
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

node *GBCmonop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCmonop");
    node *n;

    /* Traverse expression. */
    MONOP_EXPR(arg_node) = TRAVdo(MONOP_EXPR(arg_node), arg_info);
    nodetype type = NODE_TYPE(MONOP_EXPR(arg_node));

    switch (MONOP_OP(arg_node))
    {
    case MO_neg:
        if (type == N_num)
            n = TBmakeInstructions(I_ineg, NULL);
        else
            n = TBmakeInstructions(I_fneg, NULL);
        break;
    case MO_not:
        n = TBmakeInstructions(I_bnot, NULL);
        break;
    default:
        n = NULL;
        CTIabort("Unknown monop type");
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

node *GBCcast(node *arg_node, info *arg_info)
{
    // !! WERKT NIETTTTTTT AAAAAHHH !!

    DBUG_ENTER("GBCcast");
    node *n;

    /* Traverse expression */
    CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);

    if (CAST_TYPE(arg_node) == T_int)
        n = TBmakeInstructions(I_f2i, NULL);
    else
        n = TBmakeInstructions(I_i2f, NULL);

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

/* WERKT */
node *GBCvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCvar");

    node *n;
    nodetype nt;
    int i;
    bool foundVardec = FALSE;

    for (i = 0; i < INFO_VC(arg_info); i++)
    {
        if (NODE_TYPE(INFO_VARIABLES(arg_info)[i]) == N_vardeclaration)
        {
            if (STReq(VAR_NAME(arg_node), VARDECLARATION_NAME(INFO_VARIABLES(arg_info)[i])) == TRUE)
            {
                foundVardec = TRUE;
                nt = N_vardeclaration;
                break;
            }
        }
        else if (NODE_TYPE(INFO_VARIABLES(arg_info)[i]) == N_parameters)
        {
            // is param
            if (STReq(VAR_NAME(arg_node), PARAMETERS_NAME(INFO_VARIABLES(arg_info)[i])) == TRUE)
            {
                foundVardec = TRUE;
                nt = N_parameters;
                break;
            }
        }
        else
        {
            // is global
            if (STReq(VAR_NAME(arg_node), GLOBALDEC_NAME(INFO_VARIABLES(arg_info)[i])) == TRUE)
            {
                foundVardec = TRUE;
                nt = N_globaldec;
                break;
            }
        }
    }

    // NETTER OPLOSSENNNN
    if (INFO_VC(arg_info) == 0)
        nt = N_num;

    /* If var wasn't found. */
    if (foundVardec == FALSE)
        CTIabort("Error during code generation, line %i", NODE_LINE(arg_node));

    /* Load var from array. */
    type t = SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(arg_node));

    switch (nt)
    {
    case N_vardeclaration:
    case N_parameters:
        if (t == T_int)
            n = TBmakeInstructions(I_iloadc, NULL);
        else if (t == T_float)
            n = TBmakeInstructions(I_floadc, NULL);
        else
            n = TBmakeInstructions(I_bloadc, NULL);
        break;

    case N_globaldec:
        if (t == T_int)
            n = TBmakeInstructions(I_iloade, NULL);
        else if (t == T_float)
            n = TBmakeInstructions(I_floade, NULL);
        else
            n = TBmakeInstructions(I_bloade, NULL);
        break;

    case N_globaldef:
        if (t == T_int)
            n = TBmakeInstructions(I_iloadg, NULL);
        else if (t == T_float)
            n = TBmakeInstructions(I_floadg, NULL);
        else
            n = TBmakeInstructions(I_bloadg, NULL);
        break;

    default:
        n = NULL;
    }

    INSTRUCTIONS_OFFSET(n) = i;

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

/* WERKT */
node *GBCvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCvarlet");

    node *n;
    nodetype nt;
    int i;
    bool foundVardec = FALSE;

    for (i = 0; i < INFO_VC(arg_info); i++)
    {
        if (NODE_TYPE(INFO_VARIABLES(arg_info)[i]) == N_vardeclaration)
        {
            if (STReq(VARLET_NAME(arg_node), VARDECLARATION_NAME(INFO_VARIABLES(arg_info)[i])) == TRUE)
            {
                foundVardec = TRUE;
                nt = N_vardeclaration;
                break;
            }
        }
        else if (NODE_TYPE(INFO_VARIABLES(arg_info)[i]) == N_parameters)
        {
            // is param
            if (STReq(VARLET_NAME(arg_node), PARAMETERS_NAME(INFO_VARIABLES(arg_info)[i])) == TRUE)
            {
                foundVardec = TRUE;
                nt = N_parameters;
                break;
            }
        }
        else
        {
            // is global
            if (STReq(VARLET_NAME(arg_node), GLOBALDEC_NAME(INFO_VARIABLES(arg_info)[i])) == TRUE)
            {
                foundVardec = TRUE;
                nt = N_globaldec;
                break;
            }
        }
    }

    // NETTER OPLOSSENNNN
    if (INFO_VC(arg_info) == 0)
        nt = N_num;

    /* If varlet wasn't found. */
    if (foundVardec == FALSE)
        CTIabort("Error during VARLET code generation, line %i", NODE_LINE(arg_node));

    /* Load var from array. */
    type t = SYMBOLTABLEENTRY_TYPE(VARLET_SYMBOLTABLEENTRY(arg_node));

    switch (nt)
    {
    case N_vardeclaration:
    case N_parameters:
        if (t == T_int)
            n = TBmakeInstructions(I_istore, NULL);
        else if (t == T_float)
            n = TBmakeInstructions(I_fstore, NULL);
        else
            n = TBmakeInstructions(I_bstore, NULL);
        break;

    case N_globaldec:
        if (t == T_int)
            n = TBmakeInstructions(I_istoree, NULL);
        else if (t == T_float)
            n = TBmakeInstructions(I_fstoree, NULL);
        else
            n = TBmakeInstructions(I_bstoree, NULL);
        break;

    case N_globaldef:
        if (t == T_int)
            n = TBmakeInstructions(I_istoreg, NULL);
        else if (t == T_float)
            n = TBmakeInstructions(I_fstoreg, NULL);
        else
            n = TBmakeInstructions(I_bstoreg, NULL);
        break;

    default:
        n = NULL;
    }

    /* store local variable with offset. */
    INSTRUCTIONS_OFFSET(n) = i;

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

    /* Continue with other varlets. */
    VARLET_NEXT(arg_node) = TRAVopt(VARLET_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

/* WERKT */
node *GBCnum(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCnum");
    node *n;
    bool foundDouble = FALSE;

    int i;
    for (i = 0; i < INFO_CC(arg_info); i++)
    {
        if (NODE_TYPE(INFO_CONSTANTS(arg_info)[i]) == N_num && NUM_VALUE(arg_node) == NUM_VALUE(INFO_CONSTANTS(arg_info)[i]))
        {
            foundDouble = TRUE;
            break;
        }
    }

    /* If the value of the integer is either 0 or 1, create a basic instruction, otherwise a custom instruction. */
    if (NUM_VALUE(arg_node) == 0)
        n = TBmakeInstructions(I_iloadc_0, NULL);
    else if (NUM_VALUE(arg_node) == 1)
        n = TBmakeInstructions(I_iloadc_1, NULL);
    else
    {
        n = TBmakeInstructions(I_iloadc, NULL);

        /* Add the indices to the right place in the array to the instruction. */
        if (foundDouble == FALSE)
        {
            INFO_CONSTANTS(arg_info)
            [INFO_CC(arg_info)] = arg_node;
            INSTRUCTIONS_OFFSET(n) = INFO_CC(arg_info);
            INFO_CC(arg_info) += 1;
        }
        else
            INSTRUCTIONS_OFFSET(n) = i;
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

/* WERKT */
node *GBCfloat(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCfloat");
    node *n;
    int i;
    bool foundDouble = FALSE;

    for (i = 0; i < INFO_CC(arg_info); i++)
    {
        if (NODE_TYPE(INFO_CONSTANTS(arg_info)[i]) == N_float && FLOAT_VALUE(arg_node) == FLOAT_VALUE(INFO_CONSTANTS(arg_info)[i]))
        {
            foundDouble = TRUE;
            break;
        }
    }

    /* If the value of the integer is either 0 or 1, create a basic instruction, otherwise a custom instruction. */
    if (FLOAT_VALUE(arg_node) == 0.0)
        n = TBmakeInstructions(I_fload_0, NULL);
    else if (FLOAT_VALUE(arg_node) == 1.0)
        n = TBmakeInstructions(I_fload_1, NULL);
    else
    {

        n = TBmakeInstructions(I_floadc, NULL);

        /* Add the indices to the right place in the array to the instruction. */
        if (foundDouble == FALSE)
        {
            INFO_CONSTANTS(arg_info)
            [INFO_CC(arg_info)] = arg_node;
            INSTRUCTIONS_OFFSET(n) = INFO_CC(arg_info);
            INFO_CC(arg_info) += 1;
        }
        else
            INSTRUCTIONS_OFFSET(n) = i;
    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

/* WERKT */
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

/* If the list is empty, add the new node as both head and tail.
    Otherwise, add it to the end and update the previous last node. */
void addNode(node *arg_node, info *arg_info)
{
    INFO_SIZE(arg_info) += 1;

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
    case I_fadd:
        s = "fadd";
        break;
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
        break;
    case I_unknown:
        CTInote("Unknown instruction type.");
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
    while (INSTRUCTIONS_NEXT(n) != NULL)
    {
        if (INSTRUCTIONS_INSTR(n) != I_ownbranch)
            printf("    ");

        printf("%s", instrToString(INSTRUCTIONS_INSTR(n)));

        if (INSTRUCTIONS_OFFSET(n) != NULL)
            printf(" %i", INSTRUCTIONS_OFFSET(n));
        // WHAT IF OFFSET = 0? WARNING!

        if (INSTRUCTIONS_ARG(n) != NULL)
            printf(" %s", INSTRUCTIONS_ARG(n)); // spatie te veel?

        if (INSTRUCTIONS_INSTR(n) == I_ownbranch)
            printf(":");

        printf("\n");

        n = INSTRUCTIONS_NEXT(n);
    }

    /* Print the last instruction. */
    if (INSTRUCTIONS_INSTR(n) != I_ownbranch)
        printf("    ");

    printf("%s", instrToString(INSTRUCTIONS_INSTR(n)));
    if (INSTRUCTIONS_OFFSET(n) != 0)
        printf(" %i", INSTRUCTIONS_OFFSET(n));

    if (INSTRUCTIONS_ARG(n) != NULL)
        printf(" %s", INSTRUCTIONS_ARG(n));

    printf("\n");

    /* Add constants */
    char *consttype;
    int numval;
    float floatval;

    // DUBBELE CODE MET PRINT DOOR %i EN %f

    for (int i = 0; i < INFO_CC(arg_info); i++)
    {
        if (NODE_TYPE(INFO_CONSTANTS(arg_info)[i]) == N_num)
        {
            consttype = TypetoString(T_int);
            numval = NUM_VALUE(INFO_CONSTANTS(arg_info)[i]);
            printf(".const %s %i \n", consttype, numval);
        }
        else
        {
            // float
            consttype = TypetoString(T_float);
            floatval = FLOAT_VALUE(INFO_CONSTANTS(arg_info)[i]);
            printf(".const %s %f \n", consttype, floatval);
        }

        // printf(".const %s %i \n", consttype, constvalue);
    }

    /* Add exports */
    char *returntype;
    char *paramtype;

    for (int i = 0; i < INFO_EFC(arg_info); i++)
    {
        returntype = TypetoString(FUNCTION_TYPE(INFO_EXPORTFUN(arg_info)[i]));
        printf(".exportfun \"%s\" %s ", FUNCTION_NAME(INFO_EXPORTFUN(arg_info)[i]), returntype);

        node *param = FUNCTION_PARAMETERS(INFO_EXPORTFUN(arg_info)[i]);

        while (param != NULL)
        {
            paramtype = TypetoString(PARAMETERS_TYPE(param));
            printf("%s ", paramtype);
            param = PARAMETERS_NEXT(param);
        }

        printf("%s\n", FUNCTION_NAME(INFO_EXPORTFUN(arg_info)[i]));
    }

    /* Add imports */
    char *globaltype;

    for (int i = 0; i < INFO_IVC(arg_info); i++)
    {
        globaltype = TypetoString(GLOBALDEC_TYPE(INFO_IMPORTVAR(arg_info)[i]));
        printf(".importvar \"%s\" %s\n", GLOBALDEC_NAME(INFO_IMPORTVAR(arg_info)[i]), globaltype);
    }
}
