#ifndef _CIVCC_MYTYPES_H_
#define _CIVCC_MYTYPES_H_

/* This enumeration defines all monops */
typedef enum {
    MO_not,
    MO_neg,
    MO_unknown
} monop;

/* This enumeration defines all binops */
typedef enum {
    BO_add,
    BO_sub,
    BO_mul,
    BO_div,
    BO_mod,
    BO_lt,
    BO_le,
    BO_gt,
    BO_ge,
    BO_eq,
    BO_ne,
    BO_and,
    BO_or,
    BO_unknown
} binop;

/* These enums define types. */
typedef enum { T_void,
               T_int,
               T_float,
               T_bool,
               T_unknown
} type;

/* This enumeration defines all instructions in assembly */
typedef enum {
    /* Arithmetic operations */
    I_iadd,
    I_fadd,
    I_isub,
    I_fsub,
    I_imul,
    I_fmul,
    I_idiv,
    I_fdiv,
    I_irem,
    I_ineg,
    I_fneg,
    I_bnot,
    I_iinc,
    I_iinc_1,
    I_idec,
    I_idec_1,
    I_badd,
    I_bmul,
    /* Comparison operations */
    I_ine,
    I_ieq,
    I_ilt,
    I_ile,
    I_igt,
    I_ige,
    I_fne,
    I_feq,
    I_flt,
    I_fle,
    I_fgt,
    I_fge,
    I_bne,
    I_beq,
    /* Control flow instructions */
    I_isr,
    I_isrn,
    I_isrl,
    I_isrg,
    I_jsr,
    I_jsre,
    I_esr,
    I_ireturn,
    I_freturn,
    I_breturn,
    I_return,
    I_jump,
    I_branch_t,
    I_branch_f,
    /* Load variables and constants */
    I_iload,
    I_iload_0,
    I_iload_1,
    I_iload_2,
    I_iload_3,
    I_fload,
    I_fload_0,
    I_fload_1,
    I_fload_2,
    I_fload_3,
    I_bload,
    I_bload_0,
    I_bload_1,
    I_bload_2,
    I_bload_3,
    I_iloadn,
    I_floadn,
    I_bloadn,
    I_iloadg,
    I_floadg,
    I_bloadg,
    I_iloade,
    I_floade,
    I_bloade,
    I_iloadc,
    I_floadc,
    I_bloadc,
    I_iloadc_0,
    I_floadc_0,
    I_bloadc_t,
    I_iloadc_1,
    I_floadc_1,
    I_bloadc_f,
    I_iloadc_m1,
    /* Store variables and constants */
    I_istore,
    I_fstore,
    I_bstore,
    I_istoren,
    I_fstoren,
    I_bstoren,
    I_istoreg,
    I_fstoreg,
    I_bstoreg,
    I_istoree,
    I_fstoree,
    I_bstoree,
    /* Type conversion */
    I_i2f,
    I_f2i,
    I_ipop,
    I_fpop,
    I_bpop,
    I_unknown,
    I_ownbranch
} instr;

#endif /* _CIVCC_MYTYPES_H_ */
