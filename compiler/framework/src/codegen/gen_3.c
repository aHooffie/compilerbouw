/* Expressions */
node *GBCternop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCternop");
    DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("GBCbinop");

    // find out what binop
    switch(BINOP_OP(arg_node))
    {
        case BO_add:
            // code

        case BO_sub:

        case BO_mul:

        case BO_div:

        case BO_mod:

        case BO_lt:

        case BO_le:

        case BO_gt:

        case BO_ge:

        case BO_eq:

        case BO_ne:

        case BO_and:

        case BO_or:

        case BO_unknown:

        default:
            // code
    }

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