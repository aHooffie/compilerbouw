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
            // (only for integers)
            n = TBmakeInstructions(I_irem, NULL);

        case BO_lt:

        case BO_le:

        case BO_gt:

        case BO_ge:

        case BO_eq:
        // equal
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
        // not equal
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

        // case BO_unknown:

    }

    /* Add the node to the list of instructions. */
    addNode(n, arg_info);

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