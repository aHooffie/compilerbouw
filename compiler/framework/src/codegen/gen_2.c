/* Statements */
/* Ifelse - not sure if done */
node *GBCifelse(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCifelse");
    char *end;
    char str[12];

    /* Traverse into if condition. */
    IFELSE_CONDITION(arg_node) = TRAVdo(IFELSE_CONDITION(arg_node), arg_info);
    INFO_BC(arg_info) += 1;

    /* Check if there is an else block of statements. */
    if (IFELSE_ELSE(arg_node) != NULL) {
        sprintf(str, "%d", INFO_BC(arg_info));
        end = strcat(str, "_else");

        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_branch_f, NULL);
        INSTRUCTIONS_ARG(n) = end;
        addNode(n, arg_info);

        /* Traverse into if block. */
 		IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);
        INFO_BC(arg_info) += 1;

        /* Create jump instruction. */
        /* Add the branch_f labelname to the linked list.*/
        // n = TBmakeInstructions(I_branch_f, NULL);
        // INSTRUCTIONS_ARG(n) = end;
        // addNode(n, arg_info);
        
        // sprintf(buffer, "%d", INFO_BRANCHCOUNT(arg_info));
 		// command = STRcatn(3,"   jump ", buffer, "_end\n");
 		// fputs(command, INFO_CODE(arg_info));

 		// sprintf(buffer, "%d", INFO_BRANCHCOUNT(arg_info)-1);
 		// command = STRcat(buffer, "_else:\n");
 		// fputs(command, INFO_CODE(arg_info));

        /* Traverse into else statements. */
        IFELSE_ELSE(arg_node) = TRAVdo(IFELSE_ELSE(arg_node), arg_info);
 	}
 	else
    {
        sprintf(str, "%d", INFO_BC(arg_info));
        end = strcat(str, "_end");
        
        /* Add the branch_f labelname to the linked list.*/
        n = TBmakeInstructions(I_branch_f, NULL);
        INSTRUCTIONS_ARG(n) = end;
        addNode(n, arg_info);
         
        /* Traverse into block of statements. */
        IFELSE_BLOCK(arg_node) = TRAVdo(IFELSE_BLOCK(arg_node), arg_info);
 	}

    /* Add the branch_f labelname to the linked list.*/
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARG(n) = end;
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCfor");

    // for = rewrite to while loop

    FOR_START(arg_node) = TRAVdo(FOR_START(arg_node), arg_info);
    FOR_STOP(arg_node) = TRAVdo(FOR_STOP(arg_node), arg_info);
    FOR_BLOCK(arg_node) = TRAVdo(FOR_BLOCK(arg_node), arg_info);

    if (FOR_STEP(arg_node) != NULL)
        FOR_STEP(arg_node) = TRAVdo(FOR_STEP(arg_node), arg_info);
    // use iinc_1 if no step, else load ^ in iinc

    // Write as while loop
    DBUG_RETURN(arg_node);
}

/* While - done? */
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
    start = strcat(str, "_while");

    /* Add the label as instruction. */
    n = TBmakeInstructions(I_ownbranch, NULL);
    INSTRUCTIONS_ARG(n) = start;
    addNode(n, arg_info);

    /* Traverse into condition. */
    WHILE_CONDITION(arg_node) = TRAVdo(WHILE_CONDITION(arg_node), arg_info);

    /* Create the ending label for the branch. */
    INFO_BC(arg_info) += 1;
    sprintf(str, "%d", INFO_BC(arg_info));
    end = strcat(str, "_end");

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

/* Dowhile - done? */
node *GBCdowhile(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCdowhile");
    char *label;

    /* Create the starting label for a branch (1_while, 2_end etc. ) */
    INFO_BC(arg_info) += 1;
    sprintf(str, "%d", INFO_BC(arg_info));
    label = strcat(str, "_dowhile");

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

/* Return node - done. */
node *GBCreturn(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCreturn");
    node *n;

    /* Traverse into optional expression to return. */
    if (RETURN_EXPRESSION(arg_node) != NULL)
        RETURN_EXPRESSION(arg_node) = TRAVdo(RETURN_EXPRESSION(arg_node), arg_info);

    /* Create return instruction, according to expression type. */
    if (RETURN_TYPE(arg_node) == T_int)
        n = TBmakeInstructions(I_ireturn, NULL);
    else if (RETURN_TYPE(arg_node) == T_float)
        n = TBmakeInstructions(I_freturn, NULL);
    else if (RETURN_TYPE(arg_node) == T_bool)
        n = TBmakeInstructions(I_breturn, NULL);
    else
        n = TBmakeInstructions(I_return, NULL);

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

/* Assign node - done. */
node *GBCassign(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCassign");

    /* Load the right side. */
    ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

    /* Store result into left side. */
    if (ASSIGN_LET(arg_node) != NULL)
        ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

char *instrToString(instr type)
{
    char *s;
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

    DBUG_RETURN(arg_node);
}

void printInstructions(info *arg_info)
{
    node *n = INFO_FI(arg_info);

    // TO DO: INDENTATIE + LABELS WITH :

    /* Print all instructions to stdout (?) */
    while (INSTRUCTIONS_NEXT(n) != NULL)
    {
        sprintf(stdout, "%s", instrToString(INSTRUCTIONS_INSTR(n)));

        if (INSTRUCTIONS_OFFSET(n) != NULL)
            sprintf(stdout, " %i", INSTRUCTIONS_OFFSET(n));

        if (INSTRUCTIONS_ARG(n) != NULL)
            sprintf(stdout, " %s", INSTRUCTIONS_ARG(n));

        sprintf(stdout, "\n");
    }

    /* Print the last instruction. */
    printf("%s", INSTRUCTIONS_INSTR(n));
    if (INSTRUCTIONS_OFFSET(n) != NULL)
        sprintf(stdout, " %i", INSTRUCTIONS_OFFSET(n));

    if (INSTRUCTIONS_ARG(n) != NULL)
        sprintf(stdout, " %s", INSTRUCTIONS_ARG(n));

    sprintf(stdout, "\n");

    /* Print mogelijk nog andere instructies met een . (voor later) */
}