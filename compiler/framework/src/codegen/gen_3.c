/* Expressions */
node *GBCternop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCternop");
    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info)
{
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
            {
                n = TBmakeInstructions(I_iadd, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_fadd, NULL);
            }

        case BO_sub:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_isub, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_fsub, NULL);
            }

        case BO_mul:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_imul, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_fmul, NULL);
            }

        case BO_div:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_idiv, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_fdiv, NULL);
            }

        case BO_mod:
            n = TBmakeInstructions(I_irem, NULL);

        case BO_lt:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_ilt, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_flt, NULL);
            }

        case BO_le:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_ile, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_fle, NULL);
            }

        case BO_gt:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_igt, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_fgt, NULL);
            }

        case BO_ge:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_ige, NULL);
            }
            else
            {
                n = TBmakeInstructions(I_fge, NULL);
            }

        case BO_eq:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_ieq, NULL);
            }
            else if (left == N_FLOAT)
            {
                n = TBmakeInstructions(I_feq, NULL);
            }
            else // bool
            {
                n = TBmakeInstructions(I_beq, NULL);
            }

        case BO_ne:
            if (left == N_num)
            {
                n = TBmakeInstructions(I_ine, NULL);
            }
            else if (left == N_FLOAT)
            {
                n = TBmakeInstructions(I_fne, NULL);
            }
            else // bool
            {
                n = TBmakeInstructions(I_bne, NULL);
            }

        case BO_and:

        case BO_or:

    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

    DBUG_RETURN(arg_node);
}

node *GBCmonop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCmonop");

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