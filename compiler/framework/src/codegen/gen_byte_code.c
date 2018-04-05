/*
 * Module: Generating code.
 * Prefix: GBC
 * Author: Aynel Gul & Andrea van den Hooff
 * Arrays not implemented.
 */

#include "gen_byte_code.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "globals.h"

#include "memory.h"
#include "ctinfo.h"

// >>>>>>
// HEADER VOOR MAX_INT???
// <<<<<<

// ILOAD 2 en 3 IMPLEMENTEREN?

// LOCAL FUNCTIONS CALL ILOADN

extern char *TypetoString(type Type);
extern char *nodetypetoString(node *arg_node);
extern void typeError(info *arg_info, node *arg_node, char *message);

/* INFO structure */
struct INFO
{
    node *firstinstruction;
    node *lastinstruction;
    node *constants[256];
    node *exportfun[256];
    node *exportvar[256];
    node *importfun[256];
    node *importvar[256];
    node *global[256];
    int constantcount;
    int varcount;
    int exportfuncount;
    int exportvarcount;
    int importvarcount;
    int globalcount;
    int localvarcount; // counter to check esr offset
    int branchcount;   // counter to check what branch voorstuk should be used in labels
    int exprscount;
    FILE *filepointer;
};

/* INFO structure macros */
#define INFO_FP(n) ((n)->filepointer)
#define INFO_FI(n) ((n)->firstinstruction)
#define INFO_LI(n) ((n)->lastinstruction)
#define INFO_CONSTANTS(n) ((n)->constants)
#define INFO_EXPORTFUN(n) ((n)->exportfun)
#define INFO_EXPORTVAR(n) ((n)->exportvar)
#define INFO_IMPORTVAR(n) ((n)->importvar)
#define INFO_GLOBAL(n) ((n)->global)

#define INFO_CC(n) ((n)->constantcount)
#define INFO_EFC(n) ((n)->exportfuncount)
#define INFO_EVC(n) ((n)->exportvarcount)
#define INFO_IVC(n) ((n)->importvarcount)
#define INFO_GC(n) ((n)->globalcount)
#define INFO_LC(n) ((n)->localvarcount) // for esr count
#define INFO_BC(n) ((n)->branchcount)
#define INFO_EC(n) ((n)->exprscount)

/* INFO functions */
static info *MakeInfo(void)
{
    DBUG_ENTER("MakeInfo");

    info *result;
    result = (info *)MEMmalloc(sizeof(info));

    INFO_FI(result) = NULL;
    INFO_LI(result) = NULL;
    INFO_EVC(result) = 0;
    INFO_CC(result) = 0;
    INFO_EFC(result) = 0;
    INFO_IVC(result) = 0;
    INFO_GC(result) = 0;
    INFO_LC(result) = 0;
    INFO_BC(result) = 1;
    INFO_EC(result) = 0;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *GBCfunctioncallstmt(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCfunctioncallstmt");
    node *n;
    char *s;

    /* ISRG Label. */
    n = TBmakeInstructions(I_isrg, NULL);
    addNode(n, arg_info);

    /* Load instr: for all parameters needed*/
    INFO_EC(arg_info) = 0;
    // CREATE LOAD INSTRUCTIONS
    FUNCTIONCALLSTMT_EXPRESSIONS(arg_node) = TRAVopt(FUNCTIONCALLSTMT_EXPRESSIONS(arg_node), arg_info);

    /* JSR instr: + # params + fun name     */
    if (SYMBOLTABLEENTRY_SCOPE(FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node)) != 0)
    {
        n = TBmakeInstructions(I_jsr, NULL);
        s = STRitoa(INFO_EC(arg_info));
        s = STRcat(s, FUNCTIONCALLSTMT_NAME(arg_node));
    }
    else
    {
        n = TBmakeInstructions(I_jsre, NULL);
        s = STRitoa(SYMBOLTABLEENTRY_OFFSET(FUNCTIONCALLSTMT_SYMBOLTABLEENTRY(arg_node)));
    }

    /* Add params, add to linked list. */
    INSTRUCTIONS_ARGS(n) = s;
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCglobaldec(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCglobaldec");

    /* Traverse into array subtree - not implemented. */
    GLOBALDEC_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEC_DIMENSIONS(arg_node), arg_info);

    /* Add global variable to import variable array. */
    INFO_IMPORTVAR(arg_info)
    [INFO_IVC(arg_info)] = arg_node;
    INFO_IVC(arg_info) += 1;

    DBUG_RETURN(arg_node);
}

node *GBCglobaldef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCglobaldef");

    /* Traverse into array subtree - not implemented. */
    GLOBALDEF_DIMENSIONS(arg_node) = TRAVopt(GLOBALDEF_DIMENSIONS(arg_node), arg_info);

    /* Check if variable should be exported, add it to that array too.*/
    if (GLOBALDEF_ISEXPORT(arg_node) == TRUE)
    {
        INFO_EXPORTVAR(arg_info)
        [INFO_EVC(arg_info)] = arg_node;
        INFO_EVC(arg_info) += 1;
    }

    /* Add it to globals array. */
    INFO_GLOBAL(arg_info)
    [INFO_GC(arg_info)] = arg_node;
    INFO_GC(arg_info) += 1;

    /* Traverse into assigning subtree. */
    GLOBALDEF_ASSIGN(arg_node) = TRAVopt(GLOBALDEF_ASSIGN(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

// VOID FUNCTIONS MOETEN OOK RETURNEN!
node *GBCfunction(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCfunction");
    node *n;

    if (FUNCTION_ISEXTERN(arg_node) == TRUE)
    {
        // DO NOT MAKE INSTRUCTIONS BUT PRINT IMPORT FUN AT BOTTOM
        CTInote("FOUND GLOBAL FUN");
    } 

    /* Add the labelname to the linked list.*/
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARGS(n) = FUNCTION_NAME(arg_node);
    addNode(n, arg_info);

    /* Create esr instruction. */
    if (STReq(FUNCTION_NAME(arg_node), "__init") == FALSE)
    {
        n = TBmakeInstructions(I_esr, NULL);
        addNode(n, arg_info);
    }

    /* Traverse into child nodes. */
    FUNCTION_PARAMETERS(arg_node) = TRAVopt(FUNCTION_PARAMETERS(arg_node), arg_info);
    FUNCTION_FUNCTIONBODY(arg_node) = TRAVopt(FUNCTION_FUNCTIONBODY(arg_node), arg_info);

    /* Add offset to esr, reset variablecount for next function. */
    if (INFO_LC(arg_info) != 0)
    {
        char *s = STRitoa(INFO_LC(arg_info));
        INSTRUCTIONS_ARGS(n) = s;
        INFO_LC(arg_info) = 0;
    }

    /* Check if function needs to be exported. */
    if (FUNCTION_ISEXPORT(arg_node) == TRUE)
    {
        INFO_EXPORTFUN(arg_info)[INFO_EFC(arg_info)] = arg_node;
        INFO_EFC(arg_info) += 1;
    }

    /* Create return instruction for init and void functions that did not have a return. */
    if (INSTRUCTIONS_INSTR(INFO_LI(arg_info)) != I_return)
    {
        n = TBmakeInstructions(I_return, NULL);
        addNode(n, arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *GBCvardeclaration(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCvardeclaration");

    INFO_LC(arg_info) += 1;
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
    char *str;

    /* Traverse into if condition. */
    IFELSE_CONDITION(arg_node) = TRAVdo(IFELSE_CONDITION(arg_node), arg_info);
    INFO_BC(arg_info) += 1;

    /* Check if there is an else block of statements. */
    if (IFELSE_ELSE(arg_node) != NULL)
    {
        str = STRitoa(INFO_BC(arg_info));
        otherwise = STRcat(str, "_else");

        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_branch_f, NULL);
        INSTRUCTIONS_ARGS(n) = otherwise;
        addNode(n, arg_info);

        /* Traverse into if block. */
        IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);

        /* Create jump instruction. */
        INFO_BC(arg_info) += 1;
        str = STRitoa(INFO_BC(arg_info));
        end = STRcat(str, "_end");

        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_jump, NULL);
        INSTRUCTIONS_ARGS(n) = end;
        addNode(n, arg_info);

        /* Create the else label as instruction. */
        n = TBmakeInstructions(I_ownbranch, NULL);
        INSTRUCTIONS_ARGS(n) = otherwise;
        addNode(n, arg_info);

        /* Traverse into else statements. */
        IFELSE_ELSE(arg_node) = TRAVdo(IFELSE_ELSE(arg_node), arg_info);
    }
    else
    {
        str = STRitoa(INFO_BC(arg_info));
        end = STRcat(str, "_end");

        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_branch_f, NULL);
        INSTRUCTIONS_ARGS(n) = end;
        addNode(n, arg_info);

        /* Traverse into block of statements. */
        IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);
    }

    /* Create the end label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARGS(n) = end;
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
    // char *str;
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
node *GBCwhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCwhile");
    node *n;
    char *start;
    char *end;
    char *str;

    /* Create the starting label for a branch (1_while, 2_end etc. ) */
    INFO_BC(arg_info) += 1;
    str = STRitoa(INFO_BC(arg_info));
    start = STRcat(str, "_while");

    /* Add the label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARGS(n) = start;
    addNode(n, arg_info);

    /* Traverse into condition. */
    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);

    /* Create the ending label for the branch. */
    INFO_BC(arg_info) += 1;
    str = STRitoa(INFO_BC(arg_info));
    end = STRcat(str, "_end");

    /* Add the branch_f labelname to the linked list.*/
    n = TBmakeInstructions(I_branch_f, NULL);
    INSTRUCTIONS_ARGS(n) = end;
    addNode(n, arg_info);

    /* Traverse into block of statements. */
    WHILE_BLOCK(arg_node) = TRAVdo(WHILE_BLOCK(arg_node), arg_info);

    /* Create the jump label. */
    n = TBmakeInstructions(I_jump, NULL);
    INSTRUCTIONS_ARGS(n) = start;
    addNode(n, arg_info);

    /* Add the ending label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARGS(n) = end;
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

/* Dowhile node. */
node *GBCdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCdowhile");
    char *label;
    char *str;
    node *n;

    /* Create the starting label for a branch (1_while, 2_end etc. ) */
    INFO_BC(arg_info) += 1;
    str = STRitoa(INFO_BC(arg_info));
    label = STRcat(str, "_dowhile");

    /* Add the label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARGS(n) = label;
    addNode(n, arg_info);

    /* Traverse into statements. */
    DOWHILE_BLOCK(arg_node) = TRAVopt(DOWHILE_BLOCK(arg_node), arg_info);

    /* Traverse into condition. */
    DOWHILE_CONDITION(arg_node) = TRAVdo(DOWHILE_CONDITION(arg_node), arg_info);

    /* Add the label as instruction. */
    n = TBmakeInstructions(I_branch_t, NULL);
    INSTRUCTIONS_ARGS(n) = label;
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
node *GBCexpressions(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCexpressions");
    INFO_EC(arg_info) += 1;

    EXPRESSIONS_EXPR(arg_node) = TRAVopt(EXPRESSIONS_EXPR(arg_node), arg_info);
    EXPRESSIONS_NEXT(arg_node) = TRAVopt(EXPRESSIONS_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCternop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCternop");

    node *n;
    char *str;
    char *label;
    char *end;

    /* Traverse expression */
    TERNOP_CONDITION(arg_node) = TRAVdo(TERNOP_CONDITION(arg_node), arg_info);

    /* Create branch + label. */
    n = TBmakeInstructions(I_branch_f, NULL);
    str = STRitoa(INFO_BC(arg_info));
    label = STRcat(str, "_false_ternop");
    INSTRUCTIONS_ARGS(n) = label;
    INFO_BC(arg_info) += 1;
    addNode(n, arg_info);

    TERNOP_THEN(arg_node) = TRAVdo(TERNOP_THEN(arg_node), arg_info);

    /* Create branch + label. */
    n = TBmakeInstructions(I_jump, NULL);
    str = STRitoa(INFO_BC(arg_info));
    end = STRcat(str, "_end");
    INSTRUCTIONS_ARGS(n) = end;
    INFO_BC(arg_info) += 1;
    addNode(n, arg_info);

    /* Label! */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARGS(n) = label;
    addNode(n, arg_info);

    TERNOP_ELSE(arg_node) = TRAVdo(TERNOP_ELSE(arg_node), arg_info);

    /* End label! */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARGS(n) = end;
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
    if (left != N_num && left != N_bool && left != N_float)
    {
        switch (SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(BINOP_LEFT(arg_node))))
        {
        case T_int:
            left = N_num;
            break;
        case T_float:
            left = N_float;
            break;
        case T_bool:
            left = N_bool;
            break;
        default:
            typeError(arg_info, arg_node, "Unknown types in binop.");
        }
    }

    switch (BINOP_OP(arg_node))
    {
    case BO_add:
        if (left == N_num)
            // TO DO > optimize : met iinc??
            n = TBmakeInstructions(I_iadd, NULL);
        else if (left == N_bool)
            n = TBmakeInstructions(I_badd, NULL);
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
        else if (left == N_bool)
            n = TBmakeInstructions(I_bmul, NULL);
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
    char *str;
    nodetype nt = NODE_TYPE(SYMBOLTABLEENTRY_ORIGINAL(VAR_SYMBOLTABLEENTRY(arg_node)));
    int offset = SYMBOLTABLEENTRY_OFFSET(VAR_SYMBOLTABLEENTRY(arg_node));

    /* Load var from array. */
    type t = SYMBOLTABLEENTRY_TYPE(VAR_SYMBOLTABLEENTRY(arg_node));

    switch (nt)
    {
    case N_vardeclaration:
    case N_parameters:
        if (t == T_int)
        {
            // if (offset == 1) OPTIMALISATION???
            //     n = TBmakeInstructions(I_iload_1, NULL);
            // else if (offset == 2)
            //     n = TBmakeInstructions(I_iload_2, NULL);
            // else if (offset == 3)
            //     n = TBmakeInstructions(I_iload_3, NULL);
            // else
            n = TBmakeInstructions(I_iload, NULL);
        }
        else if (t == T_float)
        {
            n = TBmakeInstructions(I_fload, NULL);
        }
        else
        {
            n = TBmakeInstructions(I_bload, NULL);
        }
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

    str = STRitoa(offset);
    INSTRUCTIONS_ARGS(n) = str;

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);
    DBUG_RETURN(arg_node);
}

/* WERKT */
node *GBCvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCvarlet");

    node *n;
    nodetype nt = NODE_TYPE(SYMBOLTABLEENTRY_ORIGINAL(VARLET_SYMBOLTABLEENTRY(arg_node)));
    int offset = SYMBOLTABLEENTRY_OFFSET(VARLET_SYMBOLTABLEENTRY(arg_node));
    char *str;

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

        str = STRitoa(SYMBOLTABLEENTRY_OFFSET(VARLET_SYMBOLTABLEENTRY(arg_node)));
        INSTRUCTIONS_ARGS(n) = str;
        break;

    case N_globaldef:
        if (t == T_int)
            n = TBmakeInstructions(I_istoreg, NULL);
        else if (t == T_float)
            n = TBmakeInstructions(I_fstoreg, NULL);
        else
            n = TBmakeInstructions(I_bstoreg, NULL);
        str = STRitoa(SYMBOLTABLEENTRY_OFFSET(VARLET_SYMBOLTABLEENTRY(arg_node)));
        INSTRUCTIONS_ARGS(n) = str;
        break;

    default:
        n = NULL;
    }

    str = STRitoa(offset);
    INSTRUCTIONS_ARGS(n) = str;

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
    char *str;
    int i;

    /* If the value of the integer is either 0 or 1, create a basic instruction, otherwise a custom instruction. */
    if (NUM_VALUE(arg_node) == 0)
        n = TBmakeInstructions(I_iloadc_0, NULL);
    else if (NUM_VALUE(arg_node) == 1)
        n = TBmakeInstructions(I_iloadc_1, NULL);
    else
    {
        /* Find original constant in array.*/
        for (i = 0; i < INFO_CC(arg_info); i++)
        {
            if (NODE_TYPE(INFO_CONSTANTS(arg_info)[i]) == N_num && NUM_VALUE(arg_node) == NUM_VALUE(INFO_CONSTANTS(arg_info)[i]))
            {
                foundDouble = TRUE;
                break;
            }
        }
        n = TBmakeInstructions(I_iloadc, NULL);

        /* Add the indices to the right place in the array to the instruction. */
        if (foundDouble == FALSE)
        {
            INFO_CONSTANTS(arg_info)
            [INFO_CC(arg_info)] = arg_node;
            str = STRitoa(INFO_BC(arg_info));
            INSTRUCTIONS_ARGS(n) = str;
            INFO_CC(arg_info) += 1;
        }
        else
        {
            str = STRitoa(INFO_BC(arg_info));
            INSTRUCTIONS_ARGS(n) = str;
        }
        // INSTRUCTIONS_OFFSET(n) = i;
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
    char *str;
    bool foundDouble = FALSE;

    /* If the value of the integer is either 0 or 1, create a basic instruction, otherwise a custom instruction. */
    if (FLOAT_VALUE(arg_node) == 0.0)
        n = TBmakeInstructions(I_fload_0, NULL);
    else if (FLOAT_VALUE(arg_node) == 1.0)
        n = TBmakeInstructions(I_fload_1, NULL);
    else
    {
        /* Find original float declaration in array. */
        for (i = 0; i < INFO_CC(arg_info); i++)
        {
            if (NODE_TYPE(INFO_CONSTANTS(arg_info)[i]) == N_float && FLOAT_VALUE(arg_node) == FLOAT_VALUE(INFO_CONSTANTS(arg_info)[i]))
            {
                foundDouble = TRUE;
                break;
            }
        }

        n = TBmakeInstructions(I_floadc, NULL);

        /* Add the indices to the right place in the array to the instruction. */
        if (foundDouble == FALSE)
        {
            INFO_CONSTANTS(arg_info)
            [INFO_CC(arg_info)] = arg_node;
            str = STRitoa(INFO_CC(arg_info));
            INSTRUCTIONS_ARGS(n) = str;
            INFO_CC(arg_info) += 1;
        }
        else
        {
            str = STRitoa(i);
            INSTRUCTIONS_ARGS(n) = str;
        }
        // INSTRUCTIONS_OFFSET(n) = i;
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

    INFO_FP(arg_info) = fopen(global.outfile, "w");

    if (INFO_FP(arg_info) == NULL)
        INFO_FP(arg_info) = stdout;

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
    if (INFO_FI(arg_info) != NULL)
    {
        /* Print all instructions */
        while (INSTRUCTIONS_NEXT(n) != NULL)
        {
            if (INSTRUCTIONS_INSTR(n) != I_ownbranch)
                printf("    ");

            printf("%s", instrToString(INSTRUCTIONS_INSTR(n)));

            if (INSTRUCTIONS_ARGS(n) != NULL)
                printf(" %s", INSTRUCTIONS_ARGS(n));
            // WHAT IF OFFSET = 0? WARNING!

            // if (INSTRUCTIONS_ARG(n) != NULL)
            //     printf(" %s", INSTRUCTIONS_ARG(n)); // spatie te veel?

            if (INSTRUCTIONS_INSTR(n) == I_ownbranch)
                printf(":");

            printf("\n");

            n = INSTRUCTIONS_NEXT(n);
        }

        /* Print the last instruction. */
        if (INSTRUCTIONS_INSTR(n) != I_ownbranch)
            printf("    ");

        printf("%s", instrToString(INSTRUCTIONS_INSTR(n)));
        if (INSTRUCTIONS_ARGS(n) != NULL)
            printf(" %s", INSTRUCTIONS_ARGS(n));

        // if (INSTRUCTIONS_ARG(n) != NULL)
        // printf(" %s", INSTRUCTIONS_ARG(n));

        printf("\n");
    }

    /* Add constants */
    char *consttype;
    int numval;
    float floatval;

    /* Add all used constants. */
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

    /* Add exported functions. */
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

    /* Add exported vars (global definitions). */
    char *globaltype;

    for (int i = 0; i < INFO_EVC(arg_info); i++)
    {
        globaltype = TypetoString(GLOBALDEF_TYPE(INFO_GLOBAL(arg_info)[i]));
        printf(".global %s ", globaltype);
    }

    /* Add imported vars (global declarations). */
    for (int i = 0; i < INFO_IVC(arg_info); i++)
    {
        globaltype = TypetoString(GLOBALDEC_TYPE(INFO_IMPORTVAR(arg_info)[i]));
        printf(".importvar \"%s\" %s\n", GLOBALDEC_NAME(INFO_IMPORTVAR(arg_info)[i]), globaltype);
    }
}
