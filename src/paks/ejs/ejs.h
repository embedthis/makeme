/*
    ejs.h -- Embedthis Ejscript Library Source

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: orion
 */

#include "me.h"
#if ME_COM_EJS

#include "osdep.h"
#include "mpr.h"
#include "http.h"
#include "ejs.slots.h"
#include "pcre.h"
#include "zlib.h"


/************************************************************************/
/*
    Start of file "src/ejsByteCode.h"
 */
/************************************************************************/

/*
    ejsByteCode.h - Ejscript VM Byte Code
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_EJS_ejsByteCode
#define _h_EJS_ejsByteCode 1


typedef enum EjsOpCode {
    EJS_OP_ADD,
    EJS_OP_ADD_NAMESPACE,
    EJS_OP_ADD_NAMESPACE_REF,
    EJS_OP_AND,
    EJS_OP_ATTENTION,
    EJS_OP_BRANCH_EQ,
    EJS_OP_BRANCH_STRICTLY_EQ,
    EJS_OP_BRANCH_FALSE,
    EJS_OP_BRANCH_GE,
    EJS_OP_BRANCH_GT,
    EJS_OP_BRANCH_LE,
    EJS_OP_BRANCH_LT,
    EJS_OP_BRANCH_NE,
    EJS_OP_BRANCH_STRICTLY_NE,
    EJS_OP_BRANCH_NULL,
    EJS_OP_BRANCH_NOT_ZERO,
    EJS_OP_BRANCH_TRUE,
    EJS_OP_BRANCH_UNDEFINED,
    EJS_OP_BRANCH_ZERO,
    EJS_OP_BRANCH_FALSE_8,
    EJS_OP_BRANCH_TRUE_8,
    EJS_OP_BREAKPOINT,
    EJS_OP_CALL,
    EJS_OP_CALL_GLOBAL_SLOT,
    EJS_OP_CALL_OBJ_SLOT,
    EJS_OP_CALL_THIS_SLOT,
    EJS_OP_CALL_BLOCK_SLOT,
    EJS_OP_CALL_OBJ_INSTANCE_SLOT,
    EJS_OP_CALL_OBJ_STATIC_SLOT,
    EJS_OP_CALL_THIS_STATIC_SLOT,
    EJS_OP_CALL_OBJ_NAME,
    EJS_OP_CALL_SCOPED_NAME,
    EJS_OP_CALL_CONSTRUCTOR,
    EJS_OP_CALL_NEXT_CONSTRUCTOR,
    EJS_OP_CAST,
    EJS_OP_CAST_BOOLEAN,
    EJS_OP_CLOSE_BLOCK,
    EJS_OP_COMPARE_EQ,
    EJS_OP_COMPARE_STRICTLY_EQ,
    EJS_OP_COMPARE_FALSE,
    EJS_OP_COMPARE_GE,
    EJS_OP_COMPARE_GT,
    EJS_OP_COMPARE_LE,
    EJS_OP_COMPARE_LT,
    EJS_OP_COMPARE_NE,
    EJS_OP_COMPARE_STRICTLY_NE,
    EJS_OP_COMPARE_NULL,
    EJS_OP_COMPARE_NOT_ZERO,
    EJS_OP_COMPARE_TRUE,
    EJS_OP_COMPARE_UNDEFINED,
    EJS_OP_COMPARE_ZERO,
    EJS_OP_DEFINE_CLASS,
    EJS_OP_DEFINE_FUNCTION,
    EJS_OP_DELETE_NAME_EXPR,
    EJS_OP_DELETE_SCOPED_NAME_EXPR,
    EJS_OP_DIV,
    EJS_OP_DUP,
    EJS_OP_DUP2,
    EJS_OP_DUP_STACK,
    EJS_OP_END_CODE,
    EJS_OP_END_EXCEPTION,
    EJS_OP_GOTO,
    EJS_OP_GOTO_8,
    EJS_OP_INC,
    EJS_OP_INIT_DEFAULT_ARGS,
    EJS_OP_INIT_DEFAULT_ARGS_8,
    EJS_OP_INST_OF,
    EJS_OP_IS_A,
    EJS_OP_LOAD_0,
    EJS_OP_LOAD_1,
    EJS_OP_LOAD_2,
    EJS_OP_LOAD_3,
    EJS_OP_LOAD_4,
    EJS_OP_LOAD_5,
    EJS_OP_LOAD_6,
    EJS_OP_LOAD_7,
    EJS_OP_LOAD_8,
    EJS_OP_LOAD_9,
    EJS_OP_LOAD_DOUBLE,
    EJS_OP_LOAD_FALSE,
    EJS_OP_LOAD_GLOBAL,
    EJS_OP_LOAD_INT,
    EJS_OP_LOAD_M1,
    EJS_OP_LOAD_NAMESPACE,
    EJS_OP_LOAD_NULL,
    EJS_OP_LOAD_REGEXP,
    EJS_OP_LOAD_STRING,
    EJS_OP_LOAD_THIS,
    EJS_OP_LOAD_THIS_LOOKUP,
    EJS_OP_LOAD_THIS_BASE,
    EJS_OP_LOAD_TRUE,
    EJS_OP_LOAD_UNDEFINED,
    EJS_OP_LOAD_XML,
    EJS_OP_GET_LOCAL_SLOT_0,
    EJS_OP_GET_LOCAL_SLOT_1,
    EJS_OP_GET_LOCAL_SLOT_2,
    EJS_OP_GET_LOCAL_SLOT_3,
    EJS_OP_GET_LOCAL_SLOT_4,
    EJS_OP_GET_LOCAL_SLOT_5,
    EJS_OP_GET_LOCAL_SLOT_6,
    EJS_OP_GET_LOCAL_SLOT_7,
    EJS_OP_GET_LOCAL_SLOT_8,
    EJS_OP_GET_LOCAL_SLOT_9,
    EJS_OP_GET_OBJ_SLOT_0,
    EJS_OP_GET_OBJ_SLOT_1,
    EJS_OP_GET_OBJ_SLOT_2,
    EJS_OP_GET_OBJ_SLOT_3,
    EJS_OP_GET_OBJ_SLOT_4,
    EJS_OP_GET_OBJ_SLOT_5,
    EJS_OP_GET_OBJ_SLOT_6,
    EJS_OP_GET_OBJ_SLOT_7,
    EJS_OP_GET_OBJ_SLOT_8,
    EJS_OP_GET_OBJ_SLOT_9,
    EJS_OP_GET_THIS_SLOT_0,
    EJS_OP_GET_THIS_SLOT_1,
    EJS_OP_GET_THIS_SLOT_2,
    EJS_OP_GET_THIS_SLOT_3,
    EJS_OP_GET_THIS_SLOT_4,
    EJS_OP_GET_THIS_SLOT_5,
    EJS_OP_GET_THIS_SLOT_6,
    EJS_OP_GET_THIS_SLOT_7,
    EJS_OP_GET_THIS_SLOT_8,
    EJS_OP_GET_THIS_SLOT_9,
    EJS_OP_GET_SCOPED_NAME,
    EJS_OP_GET_SCOPED_NAME_EXPR,
    EJS_OP_GET_OBJ_NAME,
    EJS_OP_GET_OBJ_NAME_EXPR,
    EJS_OP_GET_BLOCK_SLOT,
    EJS_OP_GET_GLOBAL_SLOT,
    EJS_OP_GET_LOCAL_SLOT,
    EJS_OP_GET_OBJ_SLOT,
    EJS_OP_GET_THIS_SLOT,
    EJS_OP_GET_TYPE_SLOT,
    EJS_OP_GET_THIS_TYPE_SLOT,
    EJS_OP_IN,
    EJS_OP_LIKE,
    EJS_OP_LOGICAL_NOT,
    EJS_OP_MUL,
    EJS_OP_NEG,
    EJS_OP_NEW,
    EJS_OP_NEW_ARRAY,
    EJS_OP_NEW_OBJECT,
    EJS_OP_NOP,
    EJS_OP_NOT,
    EJS_OP_OPEN_BLOCK,
    EJS_OP_OPEN_WITH,
    EJS_OP_OR,
    EJS_OP_POP,
    EJS_OP_POP_ITEMS,
    EJS_OP_PUSH_CATCH_ARG,
    EJS_OP_PUSH_RESULT,
    EJS_OP_PUT_LOCAL_SLOT_0,
    EJS_OP_PUT_LOCAL_SLOT_1,
    EJS_OP_PUT_LOCAL_SLOT_2,
    EJS_OP_PUT_LOCAL_SLOT_3,
    EJS_OP_PUT_LOCAL_SLOT_4,
    EJS_OP_PUT_LOCAL_SLOT_5,
    EJS_OP_PUT_LOCAL_SLOT_6,
    EJS_OP_PUT_LOCAL_SLOT_7,
    EJS_OP_PUT_LOCAL_SLOT_8,
    EJS_OP_PUT_LOCAL_SLOT_9,
    EJS_OP_PUT_OBJ_SLOT_0,
    EJS_OP_PUT_OBJ_SLOT_1,
    EJS_OP_PUT_OBJ_SLOT_2,
    EJS_OP_PUT_OBJ_SLOT_3,
    EJS_OP_PUT_OBJ_SLOT_4,
    EJS_OP_PUT_OBJ_SLOT_5,
    EJS_OP_PUT_OBJ_SLOT_6,
    EJS_OP_PUT_OBJ_SLOT_7,
    EJS_OP_PUT_OBJ_SLOT_8,
    EJS_OP_PUT_OBJ_SLOT_9,
    EJS_OP_PUT_THIS_SLOT_0,
    EJS_OP_PUT_THIS_SLOT_1,
    EJS_OP_PUT_THIS_SLOT_2,
    EJS_OP_PUT_THIS_SLOT_3,
    EJS_OP_PUT_THIS_SLOT_4,
    EJS_OP_PUT_THIS_SLOT_5,
    EJS_OP_PUT_THIS_SLOT_6,
    EJS_OP_PUT_THIS_SLOT_7,
    EJS_OP_PUT_THIS_SLOT_8,
    EJS_OP_PUT_THIS_SLOT_9,
    EJS_OP_PUT_OBJ_NAME_EXPR,
    EJS_OP_PUT_OBJ_NAME,
    EJS_OP_PUT_SCOPED_NAME,
    EJS_OP_PUT_SCOPED_NAME_EXPR,
    EJS_OP_PUT_BLOCK_SLOT,
    EJS_OP_PUT_GLOBAL_SLOT,
    EJS_OP_PUT_LOCAL_SLOT,
    EJS_OP_PUT_OBJ_SLOT,
    EJS_OP_PUT_THIS_SLOT,
    EJS_OP_PUT_TYPE_SLOT,
    EJS_OP_PUT_THIS_TYPE_SLOT,
    EJS_OP_REM,
    EJS_OP_RETURN,
    EJS_OP_RETURN_VALUE,
    EJS_OP_SAVE_RESULT,
    EJS_OP_SHL,
    EJS_OP_SHR,
    EJS_OP_SPREAD,
    EJS_OP_SUB,
    EJS_OP_SUPER,
    EJS_OP_SWAP,
    EJS_OP_THROW,
    EJS_OP_TYPE_OF,
    EJS_OP_USHR,
    EJS_OP_XOR,
    EJS_OP_CALL_FINALLY,
    EJS_OP_GOTO_FINALLY,
} EjsOpCode;

#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ejsByteCodeTable.h"
 */
/************************************************************************/

/**
    ejsByteCodeTable.h - Master Byte Code Table
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_EJS_BYTECODETABLE_H
#define _h_EJS_BYTECODETABLE_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
    Stack effect special values
 */
#define EBC_POPN            101             /* Operand 1 specifies the stack change (pop) */

/*
    Operands
 */
#define EBC_NONE            0x0             /* No operands */
#define EBC_BYTE            0x1             /* 8 bit integer */
#define EBC_DOUBLE          0x10            /* 64 bit floating */
#define EBC_NUM             0x40            /* Encoded integer */
#define EBC_STRING          0x80            /* Interned string as an encoded integer*/
#define EBC_GLOBAL          0x100           /* Encode global */
#define EBC_SLOT            0x200           /* Slot number as an encoded integer */
#define EBC_JMP             0x1000          /* 32 bit jump offset */
#define EBC_JMP8            0x2000          /* 8 bit jump offset */
#define EBC_INIT_DEFAULT    0x8000          /* Computed goto table, 32 bit jumps  */
#define EBC_INIT_DEFAULT8   0x10000         /* Computed goto table, 8 bit jumps */
#define EBC_ARGC            0x20000         /* Argument count */
#define EBC_ARGC2           0x40000         /* Argument count * 2 */
#define EBC_ARGC3           0x80000         /* Argument count * 3 */
#define EBC_NEW_ARRAY       0x100000        /* New Array: Argument count * 2, byte code */
#define EBC_NEW_OBJECT      0x200000        /* New Object: Argument count * 3, byte code: attributes * 3 */

typedef struct EjsOptable {
    char    *name;
    int     stackEffect;
    int     args[8];
} EjsOptable;

#if EJS_DEFINE_OPTABLE
/*  
        Opcode string         Stack Effect      Operands, ...                                   
 */      
EjsOptable ejsOptable[] = {
    {   "ADD",                      -1,         { EBC_NONE,                               },},
    {   "ADD_NAMESPACE",             0,         { EBC_STRING,                             },},
    {   "ADD_NAMESPACE_REF",        -1,         { EBC_NONE,                               },},
    {   "AND",                      -1,         { EBC_NONE,                               },},
    {   "ATTENTION",                -1,         { EBC_NONE,                               },},
    {   "BRANCH_EQ",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_STRICTLY_EQ",       -1,         { EBC_JMP,                                },},
    {   "BRANCH_FALSE",             -1,         { EBC_JMP,                                },},
    {   "BRANCH_GE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_GT",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_LE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_LT",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_NE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_STRICTLY_NE",       -1,         { EBC_JMP,                                },},
    {   "BRANCH_NULL",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_NOT_ZERO",          -1,         { EBC_JMP,                                },},
    {   "BRANCH_TRUE",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_UNDEFINED",         -1,         { EBC_JMP,                                },},
    {   "BRANCH_ZERO",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_FALSE_8",           -1,         { EBC_JMP8,                               },},
    {   "BRANCH_TRUE_8",            -1,         { EBC_JMP8,                               },},
    {   "BREAKPOINT",                0,         { EBC_NUM, EBC_STRING,                    },},
    {   "CALL",                     -2,         { EBC_ARGC,                               },},
    {   "CALL_GLOBAL_SLOT",          0,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_OBJ_SLOT",            -1,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_THIS_SLOT",            0,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_BLOCK_SLOT",           0,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_OBJ_INSTANCE_SLOT",   -1,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_OBJ_STATIC_SLOT",     -1,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_THIS_STATIC_SLOT",     0,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_OBJ_NAME",            -1,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CALL_SCOPED_NAME",          0,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CALL_CONSTRUCTOR",          0,         { EBC_ARGC,                               },},
    {   "CALL_NEXT_CONSTRUCTOR",     0,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CAST",                     -1,         { EBC_NONE,                               },},
    {   "CAST_BOOLEAN",              0,         { EBC_NONE,                               },},
    {   "CLOSE_BLOCK",               0,         { EBC_NONE,                               },},
    {   "COMPARE_EQ",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_STRICTLY_EQ",      -1,         { EBC_NONE,                               },},
    {   "COMPARE_FALSE",            -1,         { EBC_NONE,                               },},
    {   "COMPARE_GE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_GT",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_LE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_LT",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_NE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_STRICTLY_NE",      -1,         { EBC_NONE,                               },},
    {   "COMPARE_NULL",             -1,         { EBC_NONE,                               },},
    {   "COMPARE_NOT_ZERO",         -1,         { EBC_NONE,                               },},
    {   "COMPARE_TRUE",             -1,         { EBC_NONE,                               },},
    {   "COMPARE_UNDEFINED",        -1,         { EBC_NONE,                               },},
    {   "COMPARE_ZERO",             -1,         { EBC_NONE,                               },},
    {   "DEFINE_CLASS",              0,         { EBC_GLOBAL,                             },},
    {   "DEFINE_FUNCTION",           0,         { EBC_STRING, EBC_STRING,                 },},
    {   "DELETE_NAME_EXPR",         -2,         { EBC_NONE,                               },},
    {   "DELETE_SCOPED_NAME_EXPR",  -1,         { EBC_NONE,                               },},
    {   "DIV",                      -1,         { EBC_NONE,                               },},
    {   "DUP",                       1,         { EBC_NONE,                               },},
    {   "DUP2",                      2,         { EBC_NONE,                               },},
    {   "DUP_STACK",                 1,         { EBC_BYTE,                               },},
    {   "END_CODE",                  0,         { EBC_NONE,                               },},
    {   "END_EXCEPTION",             0,         { EBC_NONE,                               },},
    {   "GOTO",                      0,         { EBC_JMP,                                },},
    {   "GOTO_8",                    0,         { EBC_JMP8,                               },},
    {   "INC",                       0,         { EBC_BYTE,                               },},
    {   "INIT_DEFAULT_ARGS",         0,         { EBC_INIT_DEFAULT,                       },},
    {   "INIT_DEFAULT_ARGS_8",       0,         { EBC_INIT_DEFAULT8,                      },},
    {   "INST_OF",                  -1,         { EBC_NONE,                               },},
    {   "IS_A",                     -1,         { EBC_NONE,                               },},
    {   "LOAD_0",                    1,         { EBC_NONE,                               },},
    {   "LOAD_1",                    1,         { EBC_NONE,                               },},
    {   "LOAD_2",                    1,         { EBC_NONE,                               },},
    {   "LOAD_3",                    1,         { EBC_NONE,                               },},
    {   "LOAD_4",                    1,         { EBC_NONE,                               },},
    {   "LOAD_5",                    1,         { EBC_NONE,                               },},
    {   "LOAD_6",                    1,         { EBC_NONE,                               },},
    {   "LOAD_7",                    1,         { EBC_NONE,                               },},
    {   "LOAD_8",                    1,         { EBC_NONE,                               },},
    {   "LOAD_9",                    1,         { EBC_NONE,                               },},
    {   "LOAD_DOUBLE",               1,         { EBC_DOUBLE,                             },},
    {   "LOAD_FALSE",                1,         { EBC_NONE,                               },},
    {   "LOAD_GLOBAL",               1,         { EBC_NONE,                               },},
    {   "LOAD_INT",                  1,         { EBC_NUM,                                },},
    {   "LOAD_M1",                   1,         { EBC_NONE,                               },},
    {   "LOAD_NAMESPACE",            1,         { EBC_STRING,                             },},
    {   "LOAD_NULL",                 1,         { EBC_NONE,                               },},
    {   "LOAD_REGEXP",               1,         { EBC_STRING,                             },},
    {   "LOAD_STRING",               1,         { EBC_STRING,                             },},
    {   "LOAD_THIS",                 1,         { EBC_NONE,                               },},
    {   "LOAD_THIS_LOOKUP",          1,         { EBC_NONE,                               },},
    {   "LOAD_THIS_BASE",            1,         { EBC_NUM,                                },},
    {   "LOAD_TRUE",                 1,         { EBC_NONE,                               },},
    {   "LOAD_UNDEFINED",            1,         { EBC_NONE,                               },},
    {   "LOAD_XML",                  1,         { EBC_STRING,                             },},
    {   "GET_LOCAL_SLOT_0",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_1",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_2",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_3",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_4",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_5",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_6",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_7",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_8",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_9",          1,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_0",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_1",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_2",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_3",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_4",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_5",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_6",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_7",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_8",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_9",            0,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_0",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_1",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_2",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_3",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_4",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_5",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_6",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_7",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_8",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_9",           1,         { EBC_NONE,                               },},
    {   "GET_SCOPED_NAME",           1,         { EBC_STRING, EBC_STRING,                 },},
    {   "GET_SCOPED_NAME_EXPR",      -1,        { EBC_NONE,                               },},
    {   "GET_OBJ_NAME",              0,         { EBC_STRING, EBC_STRING,                 },},
    {   "GET_OBJ_NAME_EXPR",        -2,         { EBC_NONE,                               },},
    {   "GET_BLOCK_SLOT",            1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "GET_GLOBAL_SLOT",           1,         { EBC_SLOT,                               },},
    {   "GET_LOCAL_SLOT",            1,         { EBC_SLOT,                               },},
    {   "GET_OBJ_SLOT",              0,         { EBC_SLOT,                               },},
    {   "GET_THIS_SLOT",             1,         { EBC_SLOT,                               },},
    {   "GET_TYPE_SLOT",             0,         { EBC_SLOT, EBC_NUM,                      },},
    {   "GET_THIS_TYPE_SLOT",        1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "IN",                       -1,         { EBC_NONE,                               },},
    {   "LIKE",                     -1,         { EBC_NONE,                               },},
    {   "LOGICAL_NOT",               0,         { EBC_NONE,                               },},
    {   "MUL",                      -1,         { EBC_NONE,                               },},
    {   "NEG",                       0,         { EBC_NONE,                               },},
    {   "NEW",                       0,         { EBC_NONE,                               },},
    {   "NEW_ARRAY",                 1,         { EBC_GLOBAL, EBC_NEW_ARRAY,              },},
    {   "NEW_OBJECT",                1,         { EBC_GLOBAL, EBC_NEW_OBJECT,             },},
    {   "NOP",                       0,         { EBC_NONE,                               },},
    {   "NOT",                       0,         { EBC_NONE,                               },},
    {   "OPEN_BLOCK",                0,         { EBC_SLOT, EBC_NUM,                      },},
    {   "OPEN_WITH",                 1,         { EBC_NONE,                               },},
    {   "OR",                       -1,         { EBC_NONE,                               },},
    {   "POP",                      -1,         { EBC_NONE,                               },},
    {   "POP_ITEMS",          EBC_POPN,         { EBC_BYTE,                               },},
    {   "PUSH_CATCH_ARG",            1,         { EBC_NONE,                               },},
    {   "PUSH_RESULT",               1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_0",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_1",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_2",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_3",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_4",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_5",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_6",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_7",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_8",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_9",         -1,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_0",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_1",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_2",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_3",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_4",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_5",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_6",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_7",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_8",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_9",           -2,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_0",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_1",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_2",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_3",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_4",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_5",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_6",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_7",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_8",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_9",          -1,         { EBC_NONE,                               },},
    {   "PUT_OBJ_NAME_EXPR",        -4,         { EBC_NONE,                               },},
    {   "PUT_OBJ_NAME",             -2,         { EBC_STRING, EBC_STRING,                 },},
    {   "PUT_SCOPED_NAME",          -1,         { EBC_STRING, EBC_STRING,                 },},
    {   "PUT_SCOPED_NAME_EXPR",     -3,         { EBC_NONE,                               },},
    {   "PUT_BLOCK_SLOT",           -1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "PUT_GLOBAL_SLOT",          -1,         { EBC_SLOT,                               },},
    {   "PUT_LOCAL_SLOT",           -1,         { EBC_SLOT,                               },},
    {   "PUT_OBJ_SLOT",             -2,         { EBC_SLOT,                               },},
    {   "PUT_THIS_SLOT",            -1,         { EBC_SLOT,                               },},
    {   "PUT_TYPE_SLOT",            -2,         { EBC_SLOT, EBC_NUM,                      },},
    {   "PUT_THIS_TYPE_SLOT",       -1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "REM",                      -1,         { EBC_NONE,                               },},
    {   "RETURN",                    0,         { EBC_NONE,                               },},
    {   "RETURN_VALUE",             -1,         { EBC_NONE,                               },},
    {   "SAVE_RESULT",              -1,         { EBC_NONE,                               },},
    {   "SHL",                      -1,         { EBC_NONE,                               },},
    {   "SHR",                      -1,         { EBC_NONE,                               },},
    {   "SPREAD",                    0,         { EBC_NONE,                               },},
    {   "SUB",                      -1,         { EBC_NONE,                               },},
    {   "SUPER",                     0,         { EBC_NONE,                               },},
    {   "SWAP",                      0,         { EBC_NONE,                               },},
    {   "THROW",                     0,         { EBC_NONE,                               },},
    {   "TYPE_OF",                  -1,         { EBC_NONE,                               },},
    {   "USHR",                     -1,         { EBC_NONE,                               },},
    {   "XOR",                      -1,         { EBC_NONE,                               },},
    {   "CALL_FINALLY",              0,         { EBC_NONE,                               },},
    {   "GOTO_FINALLY",              0,         { EBC_NONE,                               },},
    {   0,                           0,         { EBC_NONE,                               },},
};
#endif /* EJS_DEFINE_OPTABLE */

PUBLIC EjsOptable *ejsGetOptable();

#ifdef __cplusplus
}
#endif

#endif /* _h_EJS_BYTECODETABLE_H */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ejs.h"
 */
/************************************************************************/

/*
    ejs.h - Ejscript header

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_CORE
#define _h_EJS_CORE 1

#include    "mpr.h"
#include    "http.h"




#ifdef __cplusplus
extern "C" {
#endif

/******************************* Tunable Constants ****************************/

#ifndef ME_XML_MAX_NODE_DEPTH
    #define ME_XML_MAX_NODE_DEPTH  64
#endif
#ifndef ME_MAX_EJS_STACK
#if ME_COMPILER_HAS_MMU
    #define ME_MAX_EJS_STACK       (1024 * 1024)   /**< Stack size on virtual memory systems */
#else
    #define ME_MAX_EJS_STACK       (1024 * 32)     /**< Stack size without MMU */
#endif
#endif

/*
    Internal constants
 */
#define EJS_LOTSA_PROP              256             /**< Object with lots of properties. Grow by bigger chunks */
#define EJS_MIN_FRAME_SLOTS         16              /**< Miniumum number of slots for function frames */
#define EJS_NUM_GLOBAL              256             /**< Number of globals slots to pre-create */
#define EJS_ROUND_PROP              16              /**< Rounding for growing properties */

#define EJS_HASH_MIN_PROP           8               /**< Min props to hash */
#define EJS_MAX_COLLISIONS          4               /**< Max intern string collion chain before rehash */
#define EJS_POOL_INACTIVITY_TIMEOUT (60  * 1000)    /**< Prune inactive pooled VMs older than this */
#define EJS_SESSION_TIMER_PERIOD    (60 * 1000)     /**< Timer checks ever minute */
#define EJS_FILE_PERMS              0664            /**< Default file perms */
#define EJS_DIR_PERMS               0775            /**< Default dir perms */

/*
    Sanity constants. Only for sanity checking. Set large enough to never be a
    real limit but low enough to catch some errors in development.
 */
#define EJS_MAX_POOL                (4*1024*1024)   /**< Size of constant pool */
#define EJS_MAX_ARGS                8192            /**< Max number of args */
#define EJS_MAX_LOCALS              (10*1024)       /**< Max number of locals */
#define EJS_MAX_EXCEPTIONS          8192            /**< Max number of exceptions */
#define EJS_MAX_TRAITS              (0x7fff)        /**< Max number of declared properties per block */

/*
    Should not need to change these
 */
#define EJS_INC_ARGS                8               /**< Frame stack increment */
#define EJS_MAX_BASE_CLASSES        256             /**< Max inheritance chain */
#define EJS_DOC_HASH_SIZE           1007            /**< Hash for doc descriptions */

/*
    Compiler constants
 */
#define EC_MAX_INCLUDE              32              /**< Max number of nested includes */
#define EC_LINE_INCR                1024            /**< Growth increment for input lines */
#define EC_TOKEN_INCR               64              /**< Growth increment for tokens */
#define EC_MAX_LOOK_AHEAD           12
#define EC_BUFSIZE                  4096            /**< General buffer size */
#define EC_MAX_ERRORS               25              /**< Max compilation errors before giving up */
#define EC_CODE_BUFSIZE             4096            /**< Initial size of code gen buffer */
#define EC_NUM_PAK_PROP             32              /**< Initial number of properties */

/********************************* Defines ************************************/

/*
    Pack defaults
 */
#ifndef ME_COM_SQLITE
    #define ME_COM_SQLITE 0
#endif

#if !DOXYGEN
/*
    Forward declare types
 */
struct Ejs;
struct EjsBlock;
struct EjsDate;
struct EjsFrame;
struct EjsFunction;
struct EjsGC;
struct EjsHelpers;
struct EjsIntern;
struct EjsMem;
struct EjsNames;
struct EjsModule;
struct EjsNamespace;
struct EjsObj;
struct EjsPot;
struct EjsService;
struct EjsString;
struct EjsState;
struct EjsTrait;
struct EjsTraits;
struct EjsType;
struct EjsUri;
struct EjsWorker;
struct EjsXML;
struct EjsVoid;
#endif

/*
    Trait, type, function and property attributes. These are sometimes combined into a single attributes word.
 */
#define EJS_TRAIT_CAST_NULLS            0x1         /**< Property casts nulls */
#define EJS_TRAIT_DELETED               0x2         /**< Property has been deleted */
#define EJS_TRAIT_GETTER                0x4         /**< Property is a getter */
#define EJS_TRAIT_FIXED                 0x8         /**< Property is not configurable */
#define EJS_TRAIT_HIDDEN                0x10        /**< !Enumerable */
#define EJS_TRAIT_INITIALIZED           0x20        /**< Readonly property has been initialized */
#define EJS_TRAIT_READONLY              0x40        /**< !Writable (used for const) */
#define EJS_TRAIT_SETTER                0x80        /**< Property is a settter */
#define EJS_TRAIT_THROW_NULLS           0x100       /**< Property rejects null */
#define EJS_PROP_HAS_VALUE              0x200       /**< Property has a value record */
#define EJS_PROP_NATIVE                 0x400       /**< Property is backed by native code */
#define EJS_PROP_STATIC                 0x800       /**< Class static property */
#define EJS_PROP_ENUMERABLE             0x1000      /**< Property will be enumerable (compiler use only) */
#define EJS_FUN_CONSTRUCTOR             0x2000      /**< Method is a constructor */
#define EJS_FUN_FULL_SCOPE              0x4000      /**< Function needs closure when defined */
#define EJS_FUN_HAS_RETURN              0x8000      /**< Function has a return statement */
#define EJS_FUN_INITIALIZER             0x10000     /**< Type initializer code */
#define EJS_FUN_OVERRIDE                0x20000     /**< Override base type */
#define EJS_FUN_MODULE_INITIALIZER      0x40000     /**< Module initializer */
#define EJS_FUN_REST_ARGS               0x80000     /**< Parameter is a "..." rest */
#define EJS_TRAIT_MASK                  0xFFFFF     /**< Mask of trait attributes */

/*
    These attributes are never stored in EjsTrait but are often passed in "attributes" which is int64
 */
#define EJS_TYPE_CALLS_SUPER            0x100000    /**< Constructor calls super() */
#define EJS_TYPE_HAS_INSTANCE_VARS      0x200000    /**< Type has non-method instance vars (state) */
#define EJS_TYPE_DYNAMIC_INSTANCES      0x400000    /**< Instances are not sealed and can add properties */
#define EJS_TYPE_FINAL                  0x800000    /**< Type can't be subclassed */
#define EJS_TYPE_FIXUP                  0x1000000   /**< Type needs to inherit base types properties */
#define EJS_TYPE_HAS_CONSTRUCTOR        0x2000000   /**< Type has a constructor */
#define EJS_TYPE_HAS_TYPE_INITIALIZER   0x4000000   /**< Type has an initializer */
#define EJS_TYPE_INTERFACE              0x8000000   /**< Type is an interface */

#define EJS_TYPE_OBJ                    0x10000000  /**< Type is using object helpers */
#define EJS_TYPE_POT                    0x20000000  /**< Type is using pot helpers */
#define EJS_TYPE_BLOCK                  0x40000000  /**< Type is using block helpers */
#define EJS_TYPE_MUTABLE                0x80000000  /**< Type is mutable */

#define EJS_TYPE_MUTABLE_INSTANCES      UINT64(0x100000000) /**< Type has mutable instances */
#define EJS_TYPE_IMMUTABLE_INSTANCES    UINT64(0x200000000) /**< Type has immutable instances */
#define EJS_TYPE_VIRTUAL_SLOTS          UINT64(0x400000000) /**< Type is unsing virtual slots */
#define EJS_TYPE_NUMERIC_INDICIES       UINT64(0x800000000) /**< Type is using numeric indicies for properties */

/*
    Interpreter flags
 */
#define EJS_FLAG_EVENT          0x1         /**< Event pending */
#define EJS_FLAG_NO_INIT        0x8         /**< Don't initialize any modules*/
#define EJS_FLAG_DOC            0x40        /**< Load documentation from modules */
#define EJS_FLAG_NOEXIT         0x200       /**< App should service events and not exit */
#define EJS_FLAG_HOSTED         0x400       /**< Interp is hosted in a web server */

#define EJS_STACK_ARG           -1          /* Offset to locate first arg */

/** 
    Configured numeric type
 */
#define ME_NUM_TYPE double
typedef ME_NUM_TYPE MprNumber;

/*  
    Sizes (in bytes) of encoded types in a ByteArray
 */
#define EJS_SIZE_BOOLEAN        1
#define EJS_SIZE_SHORT          2
#define EJS_SIZE_INT            4
#define EJS_SIZE_LONG           8
#define EJS_SIZE_DOUBLE         8
#define EJS_SIZE_DATE           8

/*  
    Reserved and system Namespaces
    The empty namespace is special. When seaching for properties, the empty namespace implies to search all namespaces.
    When properties are defined without a namespace, they are defined in the the empty namespace.
 */
#define EJS_EMPTY_NAMESPACE         ""
#define EJS_BLOCK_NAMESPACE         "-block-"
#define EJS_CONSTRUCTOR_NAMESPACE   "-constructor-"
#define EJS_EJS_NAMESPACE           "ejs"
#define EJS_ITERATOR_NAMESPACE      "iterator"
#define EJS_INIT_NAMESPACE          "-initializer-"
#define EJS_INTERNAL_NAMESPACE      "internal"
#define EJS_META_NAMESPACE          "meta"
#define EJS_PRIVATE_NAMESPACE       "private"
#define EJS_PROTECTED_NAMESPACE     "protected"
#define EJS_PROTOTYPE_NAMESPACE     "-prototype-"
#define EJS_PUBLIC_NAMESPACE        "public"
#define EJS_WORKER_NAMESPACE        "ejs.worker"

/*  
    Flags for fast comparison of namespaces
 */
#define EJS_NSP_PRIVATE         0x1
#define EJS_NSP_PROTECTED       0x2

/*  
    When allocating slots, name hashes and traits, we optimize by rounding up allocations
 */
#define EJS_PROP_ROUNDUP(x) (((x) + EJS_ROUND_PROP - 1) / EJS_ROUND_PROP * EJS_ROUND_PROP)

/*  Property enumeration flags
 */
#define EJS_FLAGS_ENUM_INHERITED 0x1            /**< Enumerate inherited base classes */
#define EJS_FLAGS_ENUM_ALL      0x2             /**< Enumerate non-enumerable and fixture properties */

/*  
    Ejscript return codes.
 */
#define EJS_SUCCESS             MPR_ERR_OK
#define EJS_ERR                 MPR_ERR
#define EJS_EXCEPTION           (MPR_ERR_MAX - 1)

/*  
    Convenient slot aliases
 */
#define EJSLOT_CONSTRUCTOR          EJSLOT_Object___constructor__

/*  
    Default names and extensions
 */
#define EJS_GLOBAL                  "global"
#define EJS_DEFAULT_MODULE          "default"
#define EJS_DEFAULT_MODULE_NAME     EJS_DEFAULT_MODULE EJS_MODULE_EXT
#define EJS_DEFAULT_CLASS_NAME      "__defaultClass__"
#define EJS_INITIALIZER_NAME        "__initializer__"

#define EJS_NAME                    "ejs"
#define EJS_MOD                     "ejs.mod"
#define EJS_MODULE_EXT              ".mod"
#define EJS_SOURCE_EXT              ".es"
#define EJS_LISTING_EXT             ".lst"

/************************************************ EjsObj ************************************************/

#define EJS_SHIFT_VISITED       0
#define EJS_SHIFT_DYNAMIC       1
#define EJS_SHIFT_TYPE          0

#define EJS_MASK_VISITED        0x1
#define EJS_MASK_DYNAMIC        0x2
#define EJS_MASK_TYPE           ~(EJS_MASK_VISITED | EJS_MASK_DYNAMIC)

#define DYNAMIC(obj)            ((int) ((((EjsObj*) obj)->xtype) & EJS_MASK_DYNAMIC))
#define VISITED(obj)            ((int) ((((EjsObj*) obj)->xtype) & EJS_MASK_VISITED))
#define TYPE(obj)               ((EjsType*) ((((EjsObj*) obj)->xtype) & EJS_MASK_TYPE))
#define SET_VISITED(obj, value) ((EjsObj*) obj)->xtype = \
                                    ((value) << EJS_SHIFT_VISITED) | (((EjsObj*) obj)->xtype & ~EJS_MASK_VISITED)
#define SET_DYNAMIC(obj, value) ((EjsObj*) obj)->xtype = \
                                    (((size_t) value) << EJS_SHIFT_DYNAMIC) | (((EjsObj*) obj)->xtype & ~EJS_MASK_DYNAMIC)
#if ME_DEBUG
    #define SET_TYPE_NAME(obj, t) \
    if (1) { \
        if (t && ((EjsType*) t)->qname.name) { \
            ((EjsObj*) obj)->kind = ((EjsType*) t)->qname.name->value; \
        } \
        ((EjsObj*) obj)->type = ((EjsType*) t); \
    } else
#else
    #define SET_TYPE_NAME(obj, type)
#endif

#define SET_TYPE(obj, value) \
    if (1) { \
        ((EjsObj*) obj)->xtype = \
            (((size_t) value) << EJS_SHIFT_TYPE) | (((EjsObj*) obj)->xtype & ~EJS_MASK_TYPE); \
        SET_TYPE_NAME(obj, value); \
    } else

typedef void EjsAny;

#if ME_DEBUG
    #define ejsSetMemRef(obj) if (1) { ((EjsObj*) obj)->mem = MPR_GET_MEM(obj); } else 
#else
    #define ejsSetMemRef(obj) 
#endif

/************************************************* Names ************************************************/
/**
    Qualified name structure
    @description All names in Ejscript consist of a property name and a name space. Namespaces provide discrete
        spaces to manage and minimize name conflicts. These names will soon be converted to unicode.
    @defgroup EjsName EjsName
    @see EjsName ejsMarkName
    @stability Internal
 */       
typedef struct EjsName {
    struct EjsString *name;                          /**< Property name */
    struct EjsString *space;                         /**< Property namespace */
} EjsName;

/**
    Mark a name for GC
    @param qname Qualified name reference
    @ingroup EjsName
 */
PUBLIC void ejsMarkName(EjsName *qname);

/**
    Initialize a Qualified Name structure
    @description Initialize the statically allocated qualified name structure using a name and namespace.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */
PUBLIC EjsName ejsName(struct Ejs *ejs, cchar *space, cchar *name);

/**
    Initialize a Qualified Name structure using a wide namespace and name
    @description Initialize the statically allocated qualified name structure using a name and namespace.
        @param ejs Interpreter instance returned from #ejsCreateVM
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */
PUBLIC EjsName ejsWideName(struct Ejs *ejs, wchar *space, wchar *name);

/*
    Internal
 */
PUBLIC EjsName ejsEmptyWideName(struct Ejs *ejs, wchar *name);
PUBLIC EjsName ejsEmptyName(struct Ejs *ejs, cchar *name);

#define WEN(name) ejsEmptyWideName(ejs, name)
#define EN(name) ejsEmptyName(ejs, name)
#define N(space, name) ejsName(ejs, space, name)
#define WN(space, name) ejsWideName(ejs, space, name)

/********************************************** Special Values ******************************************/
/*
    Immutable object slot definitions
 */
#define S_Array ES_Array
#define S_Block ES_Block
#define S_Boolean ES_Boolean
#define S_ByteArray ES_ByteArray
#define S_Config ES_Config
#define S_Date ES_Date
#define S_Error ES_Error
#define S_ErrorEvent ES_ErrorEvent
#define S_Event ES_Event
#define S_File ES_File
#define S_FileSystem ES_FileSystem
#define S_Frame ES_Frame
#define S_Function ES_Function
#define S_Http ES_Http
#define S_Namespace ES_Namespace
#define S_Null ES_Null
#define S_Number ES_Number
#define S_Object ES_Object
#define S_Path ES_Path
#define S_RegExp ES_RegExp
#define S_String ES_String
#define S_Type ES_Type
#define S_Uri ES_Uri
#define S_Void ES_Void
#define S_Worker ES_Worker
#define S_XML ES_XML
#define S_XMLList ES_XMLList
#define S_commaProt ES_commaProt
#define S_empty ES_empty
#define S_false ES_false
#define S_global ES_global
#define S_iterator ES_iterator
#define S_length ES_length
#define S_max ES_max
#define S_min ES_min
#define S_minusOne ES_minusOne
#define S_nop ES_nop
#define S_null ES_null
#define S_one ES_one
#define S_public ES_public
#define S_true ES_true
#define S_undefined ES_undefined
#define S_zero ES_zero

#define S_StopIteration ES_iterator_StopIteration
#define S_nan ES_NaN
#define S_Iterator ES_iterator_Iterator
#define S_infinity ES_Infinity
#define S_negativeInfinity ES_NegativeInfinity
#define S_ejsSpace ES_ejs
#define S_iteratorSpace ES_iterator
#define S_internalSpace ES_internal
#define S_publicSpace ES_public
#define S_emptySpace ES_emptySpace

/*
    Special values outside ejs.mod

*/
#define S_LocalCache ES_global_NUM_CLASS_PROP + 1
#define EJS_MAX_SPECIAL ES_global_NUM_CLASS_PROP + 10

#if DOXYGEN
    /**
        Get immutable special value
        @param name Literal name
        @return special value
      */
    extern EjsAny *ESV(void *name);

    /**
        Special type
     */
    extern EjsType *EST(void *name);
#else
    #define ESV(name) ejs->service->immutable->properties->slots[S_ ## name].value.ref
    #define EST(name) ((EjsType*) ESV(name))
#endif

/************************************** Ejs ***********************************/
/**
    Ejsript VM Structure
    @description The Ejs structure contains the state for a single interpreter. The #ejsCreateVM routine may be used
        to create multiple interpreters and returns a reference to be used in subsequent Ejscript API calls.
    @defgroup Ejs Ejs
    @see ejsAddImmutable ejsAppendSearchPath ejsClearException ejsCloneVM ejsCreateService ejsCreateVM 
        ejsDestroyVM ejsEvalFile ejsEvalModule ejsEvalScript ejsExit ejsGetImmutable ejsGetImmutableByName 
        ejsGetVarByName ejsGethandle ejsLog ejsLookupScope ejsLookupVar ejsLookupVarWithNamespaces ejsReportError 
        ejsRun ejsRunProgram ejsSetDispatcher ejsSetSearchPath ejsThrowException 
    @stability Internal.
 */
typedef struct Ejs {
    char                *name;              /**< Unique interpreter name */
    EjsAny              *exception;         /**< Pointer to exception object */
    EjsAny              *result;            /**< Last expression result */
    struct EjsState     *state;             /**< Current evaluation state and stack */
    struct EjsService   *service;           /**< Back pointer to the service */
    EjsAny              *global;            /**< The "global" object */
    cchar               *bootSearch;        /**< Module search when bootstrapping the VM (not alloced) */
    struct EjsArray     *search;            /**< Module load search path */
    cchar               *className;         /**< Name of a specific class to run for a program */
    cchar               *methodName;        /**< Name of a specific method to run for a program */
    char                *errorMsg;          /**< Error message */
    cchar               **argv;             /**< Command line args (not alloced) */
    char                *hostedDocuments;   /**< Documents directory for hosted HttpServer */
    char                *hostedHome;        /**< Home directory for hosted HttpServer */
    int                 argc;               /**< Count of command line args */
    int                 flags;              /**< Execution flags */
    int                 exitStatus;         /**< Status to exit() */
    int                 firstGlobal;        /**< First global to examine for GC */
    int                 joining;            /**< In Worker.join */
    int                 serializeDepth;     /**< Serialization depth */
    int                 spreadArgs;         /**< Count of spread args */
    int                 gc;                 /**< GC required (don't make bit field) */
    uint                abandoned: 1;       /**< Pooled VM is released awaiting GC  */
    uint                hosted: 1;          /**< Interp is hosted (webserver) */
    uint                configSet: 1;       /**< Config properties defined */
    uint                compiling: 1;       /**< Currently executing the compiler */
    uint                destroying: 1;      /**< Interpreter is being destroyed */
    uint                dontExit: 1;        /**< Prevent App.exit() from exiting */
    uint                empty: 1;           /**< Interpreter will be created empty */
    uint                exiting: 1;         /**< VM should exit */
    uint                hasError: 1;        /**< Interpreter has an initialization error */
    uint                initialized: 1;     /**< Interpreter fully initialized and not empty */

    EjsAny              *exceptionArg;      /**< Exception object for catch block */
    MprDispatcher       *dispatcher;        /**< Event dispatcher */
    MprList             *workers;           /**< Worker interpreters */
    MprList             *modules;           /**< Loaded modules */
    MprList             *httpServers;       /**< Configured HttpServers */

    void                (*loaderCallback)(struct Ejs *ejs, int kind, ...);

    void                *loadData;          /**< Arg to load callbacks */
    void                *httpServer;        /**< HttpServer instance when VM is embedded */

    MprHash             *doc;               /**< Documentation */
    void                *sqlite;            /**< Sqlite context information */

    Http                *http;              /**< Http service object (copy of EjsService.http) */
    MprMutex            *mutex;             /**< Multithread locking */
} Ejs;


/**
    Add an immutable reference. 
    @description Ejscript keeps a set of immutable objects that are shared across virtual machines. This call adds an
        object to that set. If the object already exists in the immutable set, its slot number if returned.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param slotNum Unique slot number for the object
    @param qname Qualified name for the object
    @param obj Object to store
    @returns Returns the actual slot number allocated for the object
    @ingroup Ejs
 */
PUBLIC int ejsAddImmutable(struct Ejs *ejs, int slotNum, EjsName qname, EjsAny *obj);

/**
    Get an immutable object. 
    @description Ejscript keeps a set of immutable objects that are shared across virtual machines. This call retrieves an
        immutable object from that set.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param slotNum Unique slot number for the object
    @return obj Immutable object found at the given slotNum.
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsGetImmutable(struct Ejs *ejs, int slotNum);

/**
    Get an immutable object by name 
    @description Ejscript keeps a set of immutable objects that are shared across virtual machines. This call retrieves an
        immutable object from that set.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name for the object
    @return obj Immutable object found at the given slotNum.
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsGetImmutableByName(struct Ejs *ejs, EjsName qname);

/**
    Block garbage collection
    @description Garbage collection requires cooperation from threads. However, the VM will normally permit garbage
    collection at various points in the execution of byte code. This call block garbage collection while allowing
    program execution to continue. This is useful for short periods when transient objects are not reachable by the 
    GC marker.
    @param ejs Interpeter object returned from #ejsCreateVM
    @return The previous GC blocked state
    @ingroup Ejs
 */
PUBLIC int ejsBlockGC(Ejs *ejs);

/**
    Unblock garbage collection
    @description Unblock garbage collection that was blocked via #ejsBlockGC
    @param ejs Interpeter object returned from #ejsCreateVM
    @param blocked Blocked state to re-establish
    @ingroup Ejs
 */
PUBLIC void ejsUnblockGC(Ejs *ejs, int blocked);

/************************************ EjsPool *********************************/
/**
    Cached pooled of virtual machines.
    @defgroup EjsPool EjsPool
    @see ejsCreatePool ejsAllocPoolVM ejsFreePoolVM 
    @stability Internal
  */
typedef struct EjsPool {
    MprList     *list;                      /**< Free list */
    MprTicks    lastActivity;               /**< When a VM was last used */
    MprEvent    *timer;                     /**< VM prune timer */
    MprMutex    *mutex;                     /**< Multithread lock */
    int         count;                      /**< Count of allocated VMs */
    int         max;                        /**< Maximum number of VMs */
    Ejs         *template;                  /**< VM template to clone */
    char        *templateScript;            /**< Template initialization script filename */
    char        *startScript;               /**< Template initialization literal script */
    char        *startScriptPath;           /**< Template initialization script filename */
    char        *hostedDocuments;           /**< Documents directory for hosted HttpServer */
    char        *hostedHome;                /**< Home directory for hosted HttpServer */
} EjsPool;


/**
    Create a pool for virutal machines
    @description 
    @param poolMax Maximum number of VMs in the pool
    @param templateScript Script to execute to initialize a template VM from which all VMs in the pool will be cloned.
        This is executed only once when the pool is created. This is typically used to pre-load modules.
    @param startScript Startup script literal. This script is executed each time the VM is allocated from the pool
    @param startScriptPath As an alternative to startScript, a path to a script may be provided in startScriptPath.
        If startScriptPath is specified, startScript is ignored.
    @param home Default home directory for virtual machines
    @param documents Default documents directory for virtual machines
    @returns Allocated pool object
    @ingroup EjsPool
 */
PUBLIC EjsPool *ejsCreatePool(int poolMax, cchar *templateScript, cchar *startScript, cchar *startScriptPath, cchar *home,
        cchar *documents);

/**
    Allocate a VM from the pool
    @param pool EjsPool reference
    @param flags Reserved
    @returns Returns an Ejs VM instance
    @ingroup EjsPool
 */
PUBLIC Ejs *ejsAllocPoolVM(EjsPool *pool, int flags);

/**
    Free a VM back to the pool
    @param pool EjsPool reference
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsPool
 */
PUBLIC void ejsFreePoolVM(EjsPool *pool, Ejs *ejs);

/************************************ EjsObj **********************************/
/**
    Base object from which all objects inherit.
    @defgroup EjsObj EjsObj
    @see ejsAlloc ejsCastType ejsClone ejsCreateInstance ejsCreateObj ejsDefineProperty ejsDeleteProperty 
        ejsDeletePropertyByName ejsDeserialize ejsGetLength ejsGetProperty ejsGetPropertyByName ejsGetPropertyName 
        ejsGetPropertyTraits ejsInvokeOperator ejsInvokeOperatorDefault ejsLookupProperty ejsParse ejsSetProperty 
        ejsSetPropertyByName ejsSetPropertyName ejsSetPropertyTraits 
    @stability Internal
 */
typedef struct EjsObj {
    //  WARNING: changes to this structure require changes to mpr/src/mprPrintf.c
    ssize           xtype;              /**< xtype: typeBits | dynamic << 1 | visited */
#if ME_DEBUG
    char            *kind;              /**< If DEBUG, Type name of object (Type->qname.name) */
    struct EjsType  *type;              /**< If DEBUG, Pointer to object type */
    MprMem          *mem;               /**< If DEBUG, Pointer to underlying memory block */
#endif
} EjsObj;

/** 
    Allocate a new variable
    @description This will allocate space for a bare variable. This routine should only be called by type factories
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type object from which to create an object instance
    @param extra Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsAlloc(Ejs *ejs, struct EjsType *type, ssize extra);

/** 
    Cast a variable to a new type
    @description Cast a variable and return a new variable of the required type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to cast
    @param type Type to cast to
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCastType(Ejs *ejs, EjsAny *obj, struct EjsType *type);

/** 
    Clone a variable
    @description Copy a variable and create a new copy. This may do a shallow or deep copy. A shallow copy
        will not copy the property instances, rather it will only duplicate the property reference. A deep copy
        will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to clone
    @param deep Set to true to do a deep copy.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsClone(Ejs *ejs, EjsAny *obj, bool deep);

/** 
    Create a new variable instance 
    @description Create a new variable instance and invoke any required constructors with the given arguments.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type from which to create a new instance
    @param argc Count of args in argv
    @param argv Vector of arguments. Each arg is an EjsAny.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreateInstance(Ejs *ejs, struct EjsType *type, int argc, void *argv);

/** 
    Create a variable
    @description Create a variable of the required type. This invokes the createVar helper method for the specified type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type to cast to
    @param numSlots Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreateObj(Ejs *ejs, struct EjsType *type, int numSlots);

/** 
    Define a property
    @description Define a property in a variable and give it a name, base type, attributes and default value.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object in which to define a property
    @param slotNum Slot number in the variable for the property. Slots are numbered sequentially from zero. Set to
        -1 to request the next available slot number.
    @param qname Qualified name containing a name and a namespace.
    @param type Base type of the property. Set to ejs->voidType to leave as untyped.
    @param attributes Attribute traits. 
    @param value Initial value of the property
    @return A postitive slot number or a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsDefineProperty(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *type, int64 attributes, 
    EjsAny *value);

/** 
    Delete a property
    @description Delete a variable's property and set its slot to null. The slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable in which to delete the property
    @param slotNum Slot number in the variable for the property to delete.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsDeleteProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Delete a property by name
    @description Delete a variable's property by name and set its slot to null. The property is resolved by using 
        ejsLookupProperty with the specified name. Once deleted, the slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable in which to delete the property
    @param qname Qualified name for the property including name and namespace.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsDeletePropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The variable property stored at the nominated slot.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsGetProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a count of properties in a variable
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @return A positive integer count of the properties stored by the variable. 
    @ingroup EjsObj
 */
PUBLIC int ejsGetLength(Ejs *ejs, EjsAny *obj);

/** 
    Get a variable property's name
    @description Get a property name for the property at a given slot in the  variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The qualified property name including namespace and name. Caller must not free.
    @ingroup EjsObj
 */
PUBLIC EjsName ejsGetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a property by name
    @description Get a property from a variable by name.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param qname Qualified name specifying both a namespace and name.
    @return The variable property stored at the nominated slot.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsGetPropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property's traits
    @description Get a property's trait description. The property traits define the properties base type,
        and access attributes.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param slotNum Slot number for the requested property.
    @return A trait structure reference for the property.
    @ingroup EjsObj
 */
PUBLIC struct EjsTrait *ejsGetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Invoke an opcode on a native type.
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
        Native types would normally implement the invokeOperator helper function to respond to this function call.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsInvokeOperator(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);

//  TODO rename
/** 
    Default implementation for operator invoke
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsInvokeOperatorDefault(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);

/** 
    Lookup a property by name
    @description Search for a property by name in the given variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param qname Qualified name of the property to search for.
    @return The slot number containing the property. Then use $ejsGetProperty to retrieve the property or alternatively
        use ejsGetPropertyByName to lookup and retrieve in one step.
    @ingroup EjsObj
 */
PUBLIC int ejsLookupProperty(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Set a property's value
    @description Set a value for a property at a given slot in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetProperty(Ejs *ejs, void *obj, int slotNum, void *value);

/** 
    Set a property's value 
    @description Set a value for a property. The property is located by name in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param qname Qualified property name.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetPropertyByName(Ejs *ejs, void *obj, EjsName qname, void *value);

/** 
    Set a property's name 
    @description Set a qualified name for a property at the specified slot in the variable. The qualified name
        consists of a namespace and name - both of which must be persistent. A typical paradigm is for these name
        strings to be owned by the memory context of the variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param qname Qualified property name.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);

/** 
    Set a property's traits
    @description Set the traits describing a property. These include the property's base type and access attributes.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param type Base type for the property. Set to NULL for an untyped property.
    @param attributes Integer mask of access attributes.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

/**
    Deserialize a JSON encoded string
    @description This is the calling signature for C Functions.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param str JSON string to deserialize
    @returns Returns an allocated object equivalent to the supplied JSON encoding
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsDeserialize(Ejs *ejs, struct EjsString *str);

/**
    Parse a string 
    @description This parses a string and intelligently interprets the data type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param str String to parse
    @param prefType Preferred type. Set to the reserved type slot number. E.g. S_Number, S_String etc.
    @returns Returns an allocated object. Returns undefined if the input cannot be parsed.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsParse(Ejs *ejs, wchar *str,  int prefType);

/************************************ EjsPot **********************************/
/** 
    Property traits. 
    @description Property traits describe the type and access attributes of a property. The Trait structure
        is used by EjsBlock to describe the attributes of properties defined within a block.
        Note: These traits apply to a property definition and not to the referenced object. ie. two property 
        definitions may have different traits but will refer to the same object.
    @stability Evolving
    @ingroup EjsPot
 */
typedef struct EjsTrait {
    struct EjsType  *type;                  /**< Property type (prototype) */
    int             attributes;             /**< Modifier attributes */
} EjsTrait;


/*
    Ejs Value storage

    Use a nun-boxing scheme where pointers and core types are stored in the unclaimed NaN space. IEEE doubles have a large space of 
    unclaimed NaN values that can be used to store pointers and other values. IEEE defines:

        Sign(1)  Exponent(11) Mantissa(52)
        
        +inf            0x7ff0 0000 0000 0000
        -inf            0xfff0 0000 0000 0000
        NaN             0xfff8 0000 0000 0000
        Nan Space       0xfff1 type 0000 0000 (we use this one)
        Other NaN Space 0x7ff0 0000 0000 0000

    Doubles are encoded by adding a 2^48 bias so that all doubles have a MSB 16 bits in the range 0x0001 or 0xfffe. This means that the 
    the ranges 0x0000 is available for pointers and 0xffff is available for integers. Further, since all pointers are aligned on 8 byte
    boundaries, the bottom 3 bits are available for use as a tag to encode core types.

    References:
        http://wingolog.org/archives/2011/05/18/value-representation-in-javascript-implementations
        http://evilpie.github.com/sayrer-fatval-backup/cache.aspx.htm
        http://www.slideshare.net/greenwop/high-performance-javascript
        webkit/Source/JavaScriptCore/runtime 
 */

#define NUMBER_BIAS     0x1000000000000L            /* Add 2^48 to all double values. Subtract to use. */

#define GROUP_MASK      0xffff000000000000L         /* Mask for MSB 16 bits */
#define GROUP_DOUBLE    0x0001 // to 0xfffe
#define GROUP_CORE      0x0000000000000000L
#define GROUP_INT       0xffff000000000000L

/*
    Core group tags
    Null/Undefined are paried to have VALID_MASK set.
    True/False are paired to have BOOL_MASK set.
 */
#define TAG_MASK        0x7
#define TAG_POINTER     0x0
#define VALID_MASK      0x1
#define TAG_NULL        0x1
#define TAG_UNDEFINED   0x5
#define BOOL_MASK       0x2
#define TAG_FALSE       0x2
#define TAG_TRUE        0x6
#define TAG_STRING      0x4

#define POINTER_MASK    0xffff000000000007L         /* Single mask to test for pointers */
#define POINTER_VALUE   0x0000fffffffffff8L
#define INTEGER_VALUE   0x00000000ffffffffL

#define viscore(v)      (((v)->word & GROUP_MASK) == GROUP_CORE)
#define vispointer(v)   (((v)->word & POINTER_MASK) == 0)
#define viint(v)        ((v)->word == GROUP_INT)
#define visdouble(v)    (!(vispointer(v) || visint(v)))

#define visbool(v)      (viscore(v) && (v)->word & VALID_MASK)
#define visvalid(v)     (!(viscore(v) && (v)->word & VALID_MASK))
#define vistrue(v)      (viscore(v) && ((v)->word & TAG_MASK) == TAG_TRUE))
#define visfalse(v)     (vicore(v) && ((v)->word & TAG_MASK) == TAG_FALSE))
#define visnull(v)      (viscore(v) && (v)->word & TAG_MASK) == TAG_NULL)
#define visundefined(v) (viscore(v) && (v)->word & TAG_MASK) == TAG_UNDEFINED)
#define visstring(v)    (viscore(v) && (v)->word & TAG_MASK) == TAG_STRING)

#if ME_64
    #define vpointer(v) ((v)->pointer & POINTER_VALUE)
    #define vinteger(v) ((v)->integer & INTEGER_VALUE)
#else
    #define vpointer(v) ((v)->pointer)
    #define vinteger(v) ((v)->integer)
#endif
#define vdouble(v)      ((v)->number - NUMBER_BIAS)
#define vbool(v)        (v->word & BOOL_MASK) >> 2)
#define vstring(v)      (v->pointer & POINTER_VALUE)

typedef union EjsValue {
    uint64  word;
    double  number;
#if ME_64
    void    *pointer;
    int     integer;
#else
    #if CPU_ENDIAN == ME_LITTLE_ENDIAN
        struct {
            void    *pointer;
            int32   filler1;
        };
        struct {
            int32   integer;
            int32   filler2;
        };
    #else
        struct {
            int32   filler1;
            void    *pointer;
        };
        struct {
            int32   filler2;
            int32   integer;
        };
    #endif
#endif
} EjsValue;


/**
    Property slot structure
    @ingroup EjsPot
    @stability Internal
 */
typedef struct EjsSlot {
    EjsName         qname;                  /**< Property name */
    EjsTrait        trait;                  /**< Property descriptor traits */
    int             hashChain;              /**< Next property in hash chain */
    union {
        EjsAny      *ref;                   /**< Property reference */
#if UNUSED
        EjsValue    value;
#endif
    } value;
} EjsSlot;

/**
    Property hash linkage
    @ingroup EjsPot
    @stability Internal
 */
typedef struct EjsHash {
    int             size;                   /**< Size of hash */
    int             *buckets;               /**< Hash buckets and head of link chains */
} EjsHash;


/**
    Object properties
    @ingroup EjsPot
    @stability Internal
 */
typedef struct EjsProperties {
    EjsHash         *hash;                  /**< Hash buckets and head of link chains */
    int             size;                   /**< Current size of slots[] in elements */
    struct EjsSlot  slots[ARRAY_FLEX];      /**< Vector of slots containing property references */
} EjsProperties;


/** 
    Object with properties Type. Base object for generic objects with properties.
    @description The EjsPot type is the foundation for types, blocks, functions and scripted classes. 
        It provides storage and hashed lookup for properties.
        \n\n
        EjsPots may be either dynamic or sealed. Dynamic objects can grow the number of properties. Sealed 
        objects cannot. Sealed objects will store the properties as part of the EjsPot memory chunk. Dynamic 
        objects will perform a separate allocation for the properties that it can grow.
        \n\n
        EjsPot stores properties in an array of slots. These slots store the property name and a reference to the 
        property value.  Dynamic objects own their own name hash. Sealed object instances of a type, will refer to the 
        hash of names owned by the type.
    @defgroup EjsPot EjsPot
    @see EjsPot ejsAlloc ejsBlendObject ejsCast ejsCheckSlot ejsClone ejsCloneObject ejsClonePot ejsCoerceOperands 
        ejsCompactPot ejsCopySlots ejsCreateEmptyPot ejsCreateInstance ejsCreateObject ejsCreatePot 
        ejsCreatePotHelpers ejsDefineProperty ejsDeleteProperty ejsDeletePropertyByName 
        ejsDeserialize ejsFixTraits ejsGetHashSize ejsGetPotPropertyName ejsGetProperty ejsGrowObject ejsGrowPot 
        ejsIndexProperties ejsInsertPotProperties ejsIsPot ejsLookupPotProperty ejsLookupProperty ejsManageObject 
        ejsManagePot ejsMatchName ejsObjToJSON ejsObjToString ejsParse ejsPropertyHasTrait ejsRemovePotProperty 
        ejsSetProperty ejsSetPropertyByName ejsSetPropertyName ejsSetPropertyTraits ejsZeroSlots 
    @stability Internal.
 */
typedef struct EjsPot {
    EjsObj  obj;                                /**< Base object */
    uint    isBlock         : 1;                /**< Instance is a block */
    uint    isFrame         : 1;                /**< Instance is a frame */
    uint    isFunction      : 1;                /**< Instance is a function */
    uint    isPrototype     : 1;                /**< Object is a type prototype object */
    uint    isType          : 1;                /**< Instance is a type object */
    uint    separateHash    : 1;                /**< Object has separate hash memory */
    uint    separateSlots   : 1;                /**< Object has separate slots[] memory */
    uint    shortScope      : 1;                /**< Don't follow type or base classes */

    EjsProperties   *properties;                /** Object properties */
    //  TODO - OPT - merge numProp with bits above (24 bits)
    int             numProp;                    /** Number of properties */
} EjsPot;

#define POT(ptr)  (TYPE(ptr)->isPot)
#if DOXYGEN
    /** 
        Determine if a variable is a Pot.
        @description This call tests if the variable is a Pot.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @returns True if the variable is based on EjsPot
        @ingroup EjsPot
     */
    extern bool ejsIsPot(Ejs *ejs, EjsAny *obj);
#else
#define ejsIsPot(ejs, obj) (obj && POT(obj))
#endif

PUBLIC void ejsZeroSlots(Ejs *ejs, EjsSlot *slots, int count);
PUBLIC void ejsCopySlots(Ejs *ejs, EjsPot *dest, int destOff, EjsPot *src, int srcOff, int count);

/** 
    Create an empty property object
    @description Create a simple object using Object as its base type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @return A new object instance
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreateEmptyPot(Ejs *ejs);

/** 
    Create an object instance of the specified type
    @description Create a new object using the specified type as a base class. 
        Note: the constructor is not called. If you require the constructor to be invoked, use ejsCreateInstance
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Base type to use when creating the object instance
    @param size Number of extra slots to allocate when creating the object
    @return A new object instance
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreatePot(Ejs *ejs, struct EjsType *type, int size);

/**
    Compact an object
    @description This removes deleted properties and compacts property slot references
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to compact
    @returns The number of properties in the object.
    @ingroup EjsObj
 */
PUBLIC int ejsCompactPot(Ejs *ejs, EjsPot *obj);

/**
    Insert properties
    @description Insert properties at the given offset
    @param ejs Ejs reference returned from #ejsCreateVM
    @param pot Object to modify
    @param numSlots Number of slots to insert at offset
    @param offset Slot offset in pot
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
    @internal
 */
PUBLIC int ejsInsertPotProperties(Ejs *ejs, EjsPot *pot, int numSlots, int offset);

/**
    Make or remake a property index
    @description Make a hash lookup of properties. This will be skipped if there are insufficient properties to make the
        index worthwhile.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC int ejsIndexProperties(Ejs *ejs, EjsPot *obj);

/**
    Test a property's traits
    @description Make a hash lookup of properties. This will be skipped if there are insufficient properties to make the
        index worthwhile.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine.
    @param slotNum Property slot number in obj to examine.
    @param attributes Attribute mask to test with the selected property's traits.
    @returns A mask of the selected attributes. Returns zero if none match.
    @ingroup EjsPot
 */
PUBLIC int ejsPropertyHasTrait(Ejs *ejs, EjsAny *obj, int slotNum, int attributes);

/**
    Remove a property
    @description Remove a property and compact previous properties. WARNING: this should only be used by the compiler.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param slotNum Property slot number to remove.
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
    @internal
 */
PUBLIC int ejsRemovePotProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/**
    Lookup a property in a Pot
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param qname Property name to look for
    @returns If successful, return the slot number of the propert in obj. Otherwise return -1.
    @ingroup EjsPot
    @internal
 */
PUBLIC int ejsLookupPotProperty(Ejs *ejs, EjsPot *obj, EjsName qname);

/**
    Get a property name
    @description Get the name of the property at the given slot.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param slotNum Slot number of property to examine
    @returns EjsName for the property
    @ingroup EjsPot
    @internal
 */
PUBLIC EjsName ejsGetPotPropertyName(Ejs *ejs, EjsPot *obj, int slotNum);

/** 
    Copy an object
    @description Copy an object create a new instance. This may do a shallow or deep copy depending on the value of 
        deep. A shallow copy will not copy the property instances, rather it will only duplicate the property 
        reference. A deep copy will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param src Source object to copy
    @param deep Set to true to do a deep copy.
    @return A newly allocated object. Caller must not free as the GC will manage the lifecycle of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsClonePot(Ejs *ejs, EjsAny *src, bool deep);


/**
    Fix traits
    @description Fix the trait type references to point to mutable types in the current interpreter. This is needed
    after cloning the global object.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to fixup.
    @ingroup EjsPot
    @internal
 */
PUBLIC void ejsFixTraits(Ejs *ejs, EjsPot *obj);

/** 
    Grow a pot object
    @description Grow the property storage for an object. Object properties are stored in slots. To store more 
        properties, you need to grow the slots.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object reference to grow
    @param numSlots New minimum count of properties. If size is less than the current number of properties, the call
        will be ignored, i.e. it will not shrink objects.
    @return Zero if successful
    @ingroup EjsObj
 */
PUBLIC int ejsGrowPot(Ejs *ejs, EjsPot *obj, int numSlots);

/** 
    Mark an object as currently in use.
    @description Mark an object as currently active so the garbage collector will preserve it. This routine should
        be called by native types that extend EjsObj in their markVar helper.
    @param obj Object to mark as currently being used.
    @param flags manager flags
    @ingroup EjsObj
 */
PUBLIC void ejsManagePot(void *obj, int flags);

/**
    Check the slot
    @description Check the slot refers to a valid property
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param slotNum Slot number to check
    @returns The slotNum if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC int ejsCheckSlot(Ejs *ejs, EjsPot *obj, int slotNum);

/**
    Cast the operands as required by the operation code.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param lhs Left-hand-side of operation
    @param opcode Operation byte code
    @param rhs Right-hand-side of operation
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC EjsAny *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);

/**
    Get the preferred hash size
    @param numProp Number of properties to hash
    @returns A positive hash size integer
    @ingroup EjsPot
 */
PUBLIC int ejsGetHashSize(int numProp);

/**
    Create the Pot helpers 
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsPot
    @internal
 */
PUBLIC void ejsCreatePotHelpers(Ejs *ejs);

/**
    Method proc for conversion to a string.
    @description This method provides the default conversion to a string implementation.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert to a string
    @param argc Ignored
    @param argv Ignored
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC struct EjsString *ejsObjToString(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv);

/**
    Method proc for conversion to a JSON string.
    @description This method provides the default conversion to a JSON string implementation.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert to a JSON string
    @param argc Ignored
    @param argv Ignored
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC struct EjsString *ejsObjToJSON(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv);

/*
    ejsBlendObject flags
 */
#define EJS_BLEND_COMBINE       0x1         /**< Flag for ejsBlendObject to combine key values */
#define EJS_BLEND_DEEP          0x2         /**< Flag for ejsBlendObject to copy nested object recursively */
#define EJS_BLEND_FUNCTIONS     0x4         /**< Flag for ejsBlendObject to copy function properties */
#define EJS_BLEND_OVERWRITE     0x8         /**< Flag for ejsBlendObject to overwrite existing properties */
#define EJS_BLEND_SUBCLASSES    0x10        /**< Flag for ejsBlendObject to copy subclassed properties */
#define EJS_BLEND_PRIVATE       0x20        /**< Flag for ejsBlendObject to copy private properties */
#define EJS_BLEND_TRACE         0x40        /**< Flag for ejsBlendObject to trace blend operations to the log */
#define EJS_BLEND_ADD           0x80        /**< Flag for ejsBlendObject for "+" property blend */
#define EJS_BLEND_SUB           0x100       /**< Flag for ejsBlendObject for "-" property blend */
#define EJS_BLEND_ASSIGN        0x200       /**< Flag for ejsBlendObject for "=" property blend */
#define EJS_BLEND_COND_ASSIGN   0x400       /**< Flag for ejsBlendObject for "?" property blend */

//  TODO - rename ejsBlend
/**
    Blend objects
    @description Merge one object into another. This is useful for inheriting and optionally overwriting option 
        hashes (among other things). The blending is done at the primitive property level. If overwrite is true, 
        the property is replaced. If overwrite is false, the property will be added if it does not already exist
        index worthwhile.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param dest Destination object.
    @param src Source object.
    @param flags Select from:
        <ul>
            <li>EJS_BLEND_DEEP - to copy nested objects recursively</li>
            <li>EJS_BLEND_FUNCTIONS - to copy function properties</li>
            <li>EJS_BLEND_OVERWRITE - to overwrite existing properties in the destination when copying from source</li>
            <li>EJS_BLEND_SUBCLASSES - to copy subclasses in src</li>
            <li>EJS_BLEND_PRIVATE - to copy private properties</li>
        </ul>
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int flags);

/**
    Test if two names match
    @description This tests if two names are equivalent.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param a First name to test
    @param b Second name to test
    @returns True if the names are equivalent
    @ingroup EjsPot
 */
PUBLIC bool ejsMatchName(Ejs *ejs, EjsName *a, EjsName *b);

/********************************************** String ********************************************/
/** 
    String Class
    @description The String class provides the base class for all strings. Each String object represents a single 
    immutable linear sequence of characters. Strings have operators for: comparison, concatenation, copying, 
    searching, conversion, matching, replacement, and, subsetting.
    \n\n
    Strings are currently sequences of Unicode characters. Depending on the configuration, they may be 8, 16 or 32 bit
    code point values.
    @defgroup EjsString EjsString
    @see EjsString ejsAtoi ejsCompareAsc ejsCompareString ejsCompareSubstring ejsCompareWide ejsContainsAsc 
        ejsContainsChar ejsContainsString ejsCreateBareString ejsCreateString ejsCreateStringFromAsc 
        ejsCreateStringFromBytes ejsCreateStringFromConst ejsCreateStringFromMulti ejsCreateStringWithLength 
        ejsDestroyIntern ejsInternAsc ejsInternMulti ejsInternString ejsInternWide ejsJoinString 
        ejsJoinStrings ejsSerialize ejsSerializeWithOptions ejsSprintf ejsStartsWithAsc ejsStrcat ejsStrdup 
        ejsSubstring ejsToJSON ejsToLiteralString ejsToMulti ejsToString ejsToUpper ejsTruncateString ejsVarToString 
        ejsToLower 
    @stability Internal.
 */
typedef struct EjsString {
    //  WARNING: changes to EjsString require changes to mpr/src/mprPrintf.c
    struct EjsObj    obj;               /**< Base object */
    struct EjsString *next;             /**< Next string in hash chain link when interning */
    struct EjsString *prev;             /**< Prev string in hash chain */
    ssize            length;            /**< Length of string */
    wchar            value[ARRAY_FLEX]; /**< String value */
} EjsString;

/** 
    Create a string object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @param len Length of string to examine in value
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateString(Ejs *ejs, wchar *value, ssize len);

/**
    Create a string from a module string constant
    @param ejs Ejs reference returned from #ejsCreateVM
    @param mp Module object
    @param index String constant index
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
    @internal
 */
PUBLIC EjsString *ejsCreateStringFromConst(Ejs *ejs, struct EjsModule *mp, int index);

/**
    Create a string from ascii
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Null terminated ascii string value to intern
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateStringFromAsc(Ejs *ejs, cchar *value);

/**
    Create a string from an ascii block
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value UTF-8 multibyte string value to intern
    @param len Length of the string in bytes
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateStringFromBytes(Ejs *ejs, cchar *value, ssize len);

/**
    Create a string from UTF-8 multibyte string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Ascii string value to intern
    @param len Length of value in bytes
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateStringFromMulti(Ejs *ejs, cchar *value, ssize len);

/** 
    Create an empty string object. This creates an uninitialized string object of the requrired size. Once initialized,
        the string must be "interned" via $ejsInternString.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param len Length of space to reserve for future string data
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateBareString(Ejs *ejs, ssize len);

/** 
    Intern a string object. 
    @description This stores the string in the internal string pool. This is required if the string was
        created via ejsCreateBareString. The ejsCreateString routine will intern the string automatcially.
    @param sp String object to intern
    @return The internalized string object. NOTE: this may be different to the object passed in, if the string value
        was already present in the intern pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternString(EjsString *sp);

/** 
    Intern a string object from a UTF-8 string. 
    @description A string is created using the UTF-8 string as input. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value UTF-8 string buffer
    @param len Size of the input value string
    @return The internalized string object.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternMulti(struct Ejs *ejs, cchar *value, ssize len);

/** 
    Intern a string object from an Ascii string.
    @description A string is created using the ascii string as input. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Ascii string buffer
    @param len Size of the input value string
    @return The internalized string object.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternAsc(struct Ejs *ejs, cchar *value, ssize len);

/** 
    Intern a string object from a UTF-16 string. 
    @description A string is created using the UTF-16 string as input. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value UTF-16 string buffer
    @param len Size of the input value string
    @return The internalized string object.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternWide(struct Ejs *ejs, wchar *value, ssize len);

/** 
    Destroy the intern string cache
    @param intern Reference to the intern object
    @ingroup EjsString
 */
PUBLIC void ejsDestroyIntern(struct EjsIntern *intern);

/** 
    Parse a string and convert to an integer
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp String to parse
    @param radix Radix for parsing the string
    @return Integer representation of the string 
    @ingroup EjsString
 */
PUBLIC int ejsAtoi(Ejs *ejs, EjsString *sp, int radix);

/** 
    Join two strings
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string to join
    @param s2 Second string to join
    @return A new string representing the joined strings
    @ingroup EjsString
 */
PUBLIC EjsString *ejsJoinString(Ejs *ejs, EjsString *s1, EjsString *s2);

/** 
    Join strings
    @param ejs Ejs reference returned from #ejsCreateVM
    @param src First string to join
    @param ... Other strings to join
    @return A new string representing the joined strings
    @ingroup EjsString
 */
PUBLIC EjsString *ejsJoinStrings(Ejs *ejs, EjsString *src, ...);

/** 
    Get a substring 
    @description Get a substring at a given offset
    @param ejs Ejs reference returned from #ejsCreateVM
    @param src Source string
    @param offset Offset in string to take the substring
    @param len Length of the substring
    @return The substring
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSubstring(Ejs *ejs, EjsString *src, ssize offset, ssize len);

/** 
    Compare two strings
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string
    @param s2 Second string
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareString(Ejs *ejs, EjsString *s1, EjsString *s2);

/** 
    Compare a substring
    @description This call compares the first string with a substring in the second.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string to compare
    @param s2 Second string
    @param offset Offset in string to take the substring
    @param len Length of the substring
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareSubstring(Ejs *ejs, EjsString *s1, EjsString *s2, ssize offset, ssize len);

/** 
    Convert a string to lower case
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @return A lower case version of the input string
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToLower(Ejs *ejs, EjsString *sp);

/** 
    Convert a string to upper case
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @return A upper case version of the input string
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToUpper(Ejs *ejs, EjsString *sp);

/** 
    Truncate a string
    @description Truncate the string and return a new string. Note: the original is not modified.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param src Source string
    @param len Length of the result string
    @return The substring
    @ingroup EjsString
 */
PUBLIC EjsString *ejsTruncateString(Ejs *ejs, EjsString *src, ssize len);

/** 
    Compare a string with a multibyte string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string
    @param s2 Null terminated Ascii string
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareAsc(Ejs *ejs, EjsString *s1, cchar *s2);

/** 
    Compare a string with a wide string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string
    @param s2 Wide string
    @param len Maximum length in characters to compare
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareWide(Ejs *ejs, EjsString *s1, wchar *s2, ssize len);

/** 
    Test if a string contains a character
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param charPat Character to search for
    @return The index in the string where the character was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsContainsChar(Ejs *ejs, EjsString *sp, int charPat);

/** 
    Test if a string contains an ascii substring
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param pat Ascii string pattern to search for
    @return The index in the string where the pattern was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsContainsAsc(Ejs *ejs, EjsString *sp, cchar *pat);

/** 
    Test if a string contains another string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param pat String pattern to search for
    @return The index in the string where the pattern was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsContainsString(Ejs *ejs, EjsString *sp, EjsString *pat);

/** 
    Test if a string starts with an ascii pattern
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param pat Pattern to search for
    @return The index in the string where the pattern was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsStartsWithAsc(Ejs *ejs, EjsString *sp, cchar *pat);

/** 
    Convert an object to a UTF-8 string representation
    @description The object is converted to a string and then serialized into UTF-8.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A multibyte UTF-8 representation.
    @ingroup EjsString
 */
PUBLIC char *ejsToMulti(Ejs *ejs, void *obj);

//  TODO - rename ejsFormat
/** 
    Format arguments
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Format specifier.
    @param ... Arguments for the format specifiers
    @return A formatted string 
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSprintf(Ejs *ejs, cchar *fmt, ...);

/**
    Convert a variable to a string in JSON format
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Value to cast
    @param options Encoding options. See serialize for details.
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToJSON(Ejs *ejs, EjsAny *obj, EjsObj *options);

/*
    Serialization flags
 */
#define EJS_JSON_SHOW_COMMAS        0x1     /**< ejsSerialize flag to always put commas after properties*/
#define EJS_JSON_SHOW_HIDDEN        0x2     /**< ejsSerialize flag to include hidden properties */
#define EJS_JSON_SHOW_NAMESPACES    0x4     /**< ejsSerialize flag to include namespaces in names */
#define EJS_JSON_SHOW_PRETTY        0x8     /**< ejsSerialize flag to render in human-readible multiline format */
#define EJS_JSON_SHOW_SUBCLASSES    0x10    /**< ejsSerialize flag to include subclass properties */
#define EJS_JSON_SHOW_NOQUOTES      0x20    /**< ejsSerialize flag to omit quotes if property has no spaces */
#define EJS_JSON_SHOW_REGEXP        0x40    /**< ejsSerialize flag to emit native RegExp literals */

/**
    Serialize a variable into JSON format
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Value to cast
    @param flags Serialization options. The supported options are:
        <ul>
        <li> EJS_JSON_SHOW_COMMAS - Always use commas after properties</li> 
        <li> EJS_JSON_SHOW_HIDDEN - Include hidden properties </li>
        <li> EJS_JSON_SHOW_NOQUOTES - Omit quotes on properties if possible</li> 
        <li> EJS_JSON_SHOW_NAMESPACES - Include namespaces in property names </li>
        <li> EJS_JSON_SHOW_REGEXP - Emit native regular expression literals</li>
        <li> EJS_JSON_SHOW_PRETTY - Use human-readable multiline presentation </li> 
        <li> EJS_JSON_SHOW_SUBCLASSES - Include subclass properties </li>
        </ul>
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSerialize(Ejs *ejs, EjsAny *obj, int flags);

/**
    Serialize a variable into JSON format
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Value to cast
    @param options Serialization options. The supported options are:
        <ul>
        <li> baseClasses - Include subclass properties </li>
        <li> hidden - Include hidden properties </li>
        <li> namespaces - Include namespaces in property names </li>
        <li> pretty - Use human-readable multiline presentation </li> 
        <li> depth - Set a maximum depth to recurse in the object</li> 
        <li> replacer - Function that determines how object values are stringified for objects without a toJSON method. 
                The replace has the following signature: function replacer(key: String, value: String): String</li>
        </ul>
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSerializeWithOptions(Ejs *ejs, EjsAny *obj, EjsObj *options);

/** 
    Cast a variable to a string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToString(Ejs *ejs, EjsAny *obj);

/**
    Convert a string to a literal string style representation
    @description The object is converted to a string and then is wrapped with quotes. Embedded quotes and backquotes
        are backquoted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A string representation of the object
 */
PUBLIC EjsString *ejsToLiteralString(Ejs *ejs, EjsObj *obj);

/*
    Internal
 */
PUBLIC void ejsManageString(struct EjsString *sp, int flags);

/******************************************** EjsArray ********************************************/
/** 
    Array class
    @description Arrays provide a resizable, integer indexed, in-memory store for objects. An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered). Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list. The Array class can store objects with numerical indicies and can also store 
        any named properties. The named properties are stored in the obj field, whereas the numeric indexed values are
        stored in the data field. Array extends EjsObj and has all the capabilities of EjsObj.
    @defgroup EjsArray EjsArray
    @see EjsArray ejsAddItem ejsClearArray ejsCloneArray ejsCreateArray ejsGetFirstItem ejsGetItem ejsGetLastItem 
        ejsGetNextItem ejsGetPrevItem ejsInsertItem ejsAppendArray ejsLookupItem ejsRemoveItem ejsRemoveItemAtPos 
        ejsRemoveLastItem 
    @stability Internal
 */
typedef struct EjsArray {
    EjsPot          pot;                /**< Property storage */
    EjsObj          **data;             /**< Array elements */
    int             length;             /**< Array length property */
} EjsArray;


/** 
    Append an array
    @description This will append the contents of the source array to the destination array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param dest Destination array to modify
    @param src Source array from which to copy elements
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsArray
 */
PUBLIC int ejsAppendArray(Ejs *ejs, EjsArray *dest, EjsArray *src);

/** 
    Create an array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param size Initial size of the array
    @return A new array object
    @ingroup EjsArray
 */
PUBLIC EjsArray *ejsCreateArray(Ejs *ejs, int size);

/** 
    Clone an array
    @description This will create a new array and copy the contents from the source array. Both array elements and
        object properties are copied. If deep is true, the call creates a distinct clone with no shared elements. If
        deep is false, object references will be copied and shared between the source and cloned array.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Array source
    @param deep Set to true to clone each element of the array. Otherwise object references will have their references
        copied and not the reference targets.
    @return A new array.
    @ingroup EjsArray
 */
PUBLIC EjsArray *ejsCloneArray(Ejs *ejs, EjsArray *ap, bool deep);

/** 
    Add an item to the array
    @description This will add a new item to the end of the array and grow the array if required.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Array to modify
    @param item Object item to add.
    @return The item index in the array.
    @ingroup EjsArray
 */
PUBLIC int ejsAddItem(Ejs *ejs, EjsArray *ap, EjsAny *item);

/** 
    Clear an array and remove all items
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @ingroup EjsArray
 */
PUBLIC void ejsClearArray(Ejs *ejs, EjsArray *ap);

/** 
    Get an item from an array
    @description This will retrieve the item at the index location
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param index Location to retrieve
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetItem(Ejs *ejs, EjsArray *ap, int index);

/** 
    Get the first item from an array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetFirstItem(Ejs *ejs, EjsArray *ap);

/** 
    Get the last item from an array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetLastItem(Ejs *ejs, EjsArray *ap);

/** 
    Get the next item from an array
    @description This will retrieve the item at *next and increment *next
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param next Pointer to an integer index. The *next location is updated to prepare to advance to the next element.
        The *next location should be initialized to zero for the first call to an ejsGetNextItem sequence.
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetNextItem(Ejs *ejs, EjsArray *ap, int *next);

/** 
    Get the previous item from an array
    @description This will retrieve the item at *prev and increment *prev
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param prev Pointer to an integer index. The *prev location is updated to prepare to advance to the previous element.
        The *prev location should be initialized to zero for the first call to an ejsGetPrevItem sequence.
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetPrevItem(Ejs *ejs, EjsArray *ap, int *prev);

/** 
    Insert an item
    @description This will insert an item at the given index. Items at the index and above will be moved upward to 
        make room for the inserted item.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param index Index at which to insert the item. The item will be inserted at the "index" position.
    @param item Item to insert
    @return The index.
    @ingroup EjsArray
 */
PUBLIC int ejsInsertItem(Ejs *ejs, EjsArray *ap, int index, EjsAny *item);

/** 
    Join an array
    @description This will join the array elements using the given join string separator.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param join String to use as a delimiter between elements.
    @return The result string
    @ingroup EjsArray
 */
PUBLIC EjsString *ejsJoinArray(Ejs *ejs, EjsArray *ap, EjsString *join);

/** 
    Lookup an item in the array
    @description This search for the given item (reference) in the array. NOTE: currently numbers are implemented as
        object references and so using this routine to search for a number reference will not work.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param item Item to search for
    @return A positive array element index. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsLookupItem(Ejs *ejs, EjsArray *ap, EjsAny *item);

/** 
    Remove an item from the array
    @description This will remove an item from the array. The array will not be compacted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param item Item to remove
    @param compact Set to true to compact following properties 
    @return The index where the item was found. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsRemoveItem(Ejs *ejs, EjsArray *ap, EjsAny *item, int compact);

/** 
    Remove the last item from the array
    @description This will remove the last item from the array. The array will not be compacted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @return The index where the item was found. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsRemoveLastItem(Ejs *ejs, EjsArray *ap);

/** 
    Remove an item at a given index from the array
    @description This will remove an item from the array. The array will not be compacted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param index Array index from which to remove the item
    @param compact Set to true to compact following properties 
    @return The index where the item was found. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsRemoveItemAtPos(Ejs *ejs, EjsArray *ap, int index, int compact);

/*
    Internal
 */
PUBLIC EjsArray *ejsSortArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv);

/************************************************ Block ********************************************************/
/** 
    Block class
    @description The block class is the base class for all program code block scope objects. This is an internal class
        and not exposed to the script programmer.
    Blocks (including types) may describe their properties via traits. The traits store the property 
    type and access attributes and are stored in EjsBlock which is a sub class of EjsObj. See ejsBlock.c for details.
    @defgroup EjsBlock EjsBlock
    @see EjsBlock ejsIsBlock ejsBindFunction
    @stability Internal
 */
typedef struct EjsBlock {
    EjsPot          pot;                            /**< Property storage */
    MprList         namespaces;                     /**< Current list of namespaces open in this block of properties */
    struct EjsBlock *scope;                         /**< Lexical scope chain for this block */
    struct EjsBlock *prev;                          /**< Previous block in activation chain */

    //  TODO -- OPT and compress / eliminate some of these fields. Every function has these.
    EjsObj          *prevException;                 /**< Previous exception if nested exceptions */
    EjsObj          **stackBase;                    /**< Start of stack in this block */
    uchar           *restartAddress;                /**< Restart instruction address */
    uint            nobind: 1;                      /**< Don't bind to properties in this block */
#if ME_DEBUG
    struct EjsLine  *line;
#endif
} EjsBlock;

#if DOXYGEN
    /** 
        Determine if a variable is a block.
        @description This call tests if the variable is a block.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @returns True if the variable is based on EjsBlock
        @ingroup EjsBlock
     */
    extern bool ejsIsBlock(Ejs *ejs, EjsObj *obj);
#else
    #define ejsIsBlock(ejs, obj) (ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isBlock)
#endif

/*  
    These are an internal APIs. Native types should probably not be using these routines. Speak up if you find
    you need these routines in your code.
 */
PUBLIC int ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *blockRef, struct EjsNamespace *nsp);
PUBLIC EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep);
PUBLIC EjsBlock *ejsCreateBlock(Ejs *ejs, int numSlots);
PUBLIC void ejsCreateBlockHelpers(Ejs *ejs);
PUBLIC int ejsGetNamespaceCount(EjsBlock *block);
PUBLIC void ejsManageBlock(EjsBlock *block, int flags);
PUBLIC void ejsPopBlockNamespaces(EjsBlock *block, int count);
PUBLIC void ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block);

#if ME_DEBUG
    #define ejsSetBlockLocation(block, loc) block->line = loc
#else
    #define ejsSetBlockLocation(block, loc)
#endif

/******************************************** Emitter *********************************************/
/** 
    Add an observer 
    @description Add an observer for events 
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitterPtr Reference to an emitter. If the reference location is NULL, a new emitter will be created and 
        passed back via *emitterPtr.
    @param name Name of events to observe. Can be an array of events.
    @param observer Function to run when selected events are triggered
    @return Zero if successful.
    @ingroup EjsObj
 */
PUBLIC int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, struct EjsFunction *observer);

/** 
    Remove an observer 
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitter Emitter created via #ejsAddObserver.
    @param name Name of observed events. Can be an array of events.
    @param observer Observer function provided to #ejsAddObserver
    @return Zero if successful.
    @ingroup EjsObj
 */
PUBLIC int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, struct EjsFunction *observer);

/** 
    Send an event to observers
    @description This call allows multiple arguments to be passed to the observer. If you only need to pass one, 
        use #ejsSendEvent.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitter Emitter object
    @param name Name of event to fire
    @param thisObj Object to use for "this" when invoking the observer
    @param argc Argument count of argv
    @param argv Arguments to pass to the observer
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, int argc, void *argv);

/** 
    Send an event to observers
    @description This call allows one argument to pass to the observer. If you need to pass more, use #ejsSendEventv.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitter Emitter object
    @param name Name of event to fire
    @param thisObj Object to use for "this" when invoking the observer
    @param arg Argument to pass to the observer
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, EjsAny *arg);

/******************************************** Function ********************************************/
/*
    Exception flags and structure
 */
#define EJS_EX_CATCH            0x1             /**< EjsEx flag for a catch block */
#define EJS_EX_FINALLY          0x2             /**< EjsEx flag for a finally block */
#define EJS_EX_ITERATION        0x4             /**< EjsEx flag for an iteration catch block */
#define EJS_EX_INC              4               /**< Growth increment for exception handlers */

/** 
    Exception Handler Record
    @description Each exception handler has an exception handler record.
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsEx {
    // TODO - OPT. Should this be compressed via bit fields for flags Could use short for these offsets.
    struct EjsType  *catchType;             /**< Type of error to catch */
    uint            flags;                  /**< Exception flags */
    uint            tryStart;               /**< Ptr to start of try block */
    uint            tryEnd;                 /**< Ptr to one past the end */
    uint            handlerStart;           /**< Ptr to start of catch/finally block */
    uint            handlerEnd;             /**< Ptr to one past the end */
    uint            numBlocks;              /**< Count of blocks opened before the try block */
    uint            numStack;               /**< Count of stack slots pushed before the try block */
} EjsEx;

#define EJS_INDEX_INCR  256                 /**< Constant pool growth increment */

/**
    Constant pool for module files
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsConstants {
    char          *pool;                    /**< Constant pool string data */
    ssize         poolSize;                 /**< Size of constant pool storage in bytes */
    ssize         poolLength;               /**< Length of used bytes in constant pool */
    int           indexSize;                /**< Size of index in elements */
    int           indexCount;               /**< Number of constants used in index */
    int           locked;                   /**< No more additions allowed */
    MprHash       *table;                   /**< Hash table for fast lookup when compiling */
    EjsString     **index;                  /**< Interned string index */
} EjsConstants;

/**
    Symbolic debugging storage for source code in module files
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsLine {
    int         offset;                     /**< Optional PC offsets of each line in function */
    wchar       *source;                    /**< Program source code. Format: path line: code */         
} EjsLine;

#define EJS_DEBUG_INCR      16              /**< Growth increment for EjsDebug */
#define EJS_DEBUG_MAGIC     0x78654423      /**< Debug record integrity check */
#define EJS_CODE_MAGIC      0x91917128      /**< Code record integrity check */

/**
    Debug record for module files
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsDebug {
    int         magic;
    ssize      size;                        /**< Size of lines[] in elements */
    int        numLines;                    /**< Number of entries in lines[] */
    EjsLine    lines[ARRAY_FLEX];           /**< Debug lines */
} EjsDebug;

/*
    Internal
 */
PUBLIC EjsDebug *ejsCreateDebug(Ejs *ejs, int length);
PUBLIC int ejsAddDebugLine(Ejs *ejs, EjsDebug **debug, int offset, wchar *source);
PUBLIC EjsLine *ejsGetDebugLine(Ejs *ejs, struct EjsFunction *fun, uchar *pc);
PUBLIC int ejsGetDebugInfo(Ejs *ejs, struct EjsFunction *fun, uchar *pc, char **path, int *lineNumber, wchar **source);

/** 
    Byte code
    @description This structure describes a sequence of byte code for a function. It also defines a set of
        execption handlers pertaining to this byte code.
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsCode {
    // TODO OPT. Could compress this.
    int              magic;                  /**< Debug magic id */
    struct EjsModule *module;                /**< Module owning this function */
    EjsDebug         *debug;                 /**< Source code debug information */
    EjsEx            **handlers;             /**< Exception handlers */
    int              codeLen;                /**< Byte code length */
    int              debugOffset;            /**< Offset in mod file for debug info */
    int              numHandlers;            /**< Number of exception handlers */
    int              sizeHandlers;           /**< Size of handlers array */
    uchar            byteCode[ARRAY_FLEX];   /**< Byte code */
} EjsCode;

/**
    Native Function signature
    @description This is the calling signature for C Functions.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param thisObj Reference to the "this" object. (The object containing the method).
    @param argc Number of arguments.
    @param argv Array of arguments.
    @returns Returns a result variable or NULL on errors and exceptions.
    @ingroup EjsFunction
    @stability Evolving.
 */
#if DOXYGEN
typedef EjsObj* (*EjsProc)(Ejs *ejs, EjsAny *thisObj, int argc, struct EjsObj **argv);
#else
typedef struct EjsObj *(*EjsProc)(Ejs *ejs, EjsAny *thisObj, int argc, struct EjsObj **argv);
#endif

/** 
    Function class
    @description The Function type is used to represent closures, function expressions and class methods. 
        It contains a reference to the code to execute, the execution scope and possibly a bound "this" reference.
    @defgroup EjsFunction EjsFunction
    @see EjsFunction ejsIsFunction ejsIsNativeFunction ejsIsInitializer ejsCreateFunction ejsCloneFunction
        ejsRunFunctionBySlot ejsRunFunction ejsRunInitializer
        ejsIsFunction ejsBindFunction ejsCloneFunction ejsCreateFunction ejsInitFunction
        ejsCreateBareFunction ejsCreateActivation ejsRemoveConstructor ejsRunInitializer ejsRunFunction
        ejsRunFunctionBySlot ejsrunFunctionByName 
    @stability Internal
 */
typedef struct EjsFunction {
    /*
        A function can store properties like any other object. If it has parameters, it must also must maintain an
        activation object. When compiling, the compiler stores parameters in the normal property "block", it then
        transfers them into the activation block when complete.
     */
    EjsBlock        block;                  /** Function properties */
    EjsPot          *activation;            /** Parameter and local properties */
    EjsString       *name;                  /** Function name */
#if FUTURE
    union {
#endif
        struct EjsFunction *setter;         /**< Setter function for this property */
        struct EjsType  *archetype;         /**< Type to use to create instances */
#if FUTURE
    } extra;
#endif
    union {
        EjsCode     *code;                  /**< Byte code */
        EjsProc     proc;                   /**< Native function pointer */
    } body;

    struct EjsArray *boundArgs;             /**< Bound "args" */
    EjsAny          *boundThis;             /**< Bound "this" object value */
    struct EjsType  *resultType;            /**< Return type of method */
    int             endFunction;            /**< Offset in mod file for end of function */

    uint    numArgs: 8;                     /**< Count of formal parameters */
    uint    numDefault: 8;                  /**< Count of formal parameters with default initializers */
    uint    allowMissingArgs: 1;            /**< Allow unsufficient args for native functions */
    uint    castNulls: 1;                   /**< Cast return values of null */
    uint    fullScope: 1;                   /**< Closures must capture full scope */
    uint    hasReturn: 1;                   /**< Has a return stmt */
    uint    inCatch: 1;                     /**< Executing catch block */
    uint    inException: 1;                 /**< Executing catch/finally exception processing */
    uint    isConstructor: 1;               /**< Is a constructor */
    uint    isInitializer: 1;               /**< Is a type initializer */
    uint    isNativeProc: 1;                /**< Is native procedure */
    uint    moduleInitializer: 1;           /**< Is a module initializer */
    uint    rest: 1;                        /**< Has a "..." rest of args parameter */
    uint    staticMethod: 1;                /**< Is a static method */
    uint    strict: 1;                      /**< Language strict mode (vs standard) */
    uint    throwNulls: 1;                  /**< Return type cannot be null */
} EjsFunction;

#if DOXYGEN
    /** 
        Determine if a variable is a function. This will return true if the variable is a function of any kind, including
            methods, native and script functions or initializers.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Variable to test
        @return True if the variable is a function
        @ingroup EjsFunction
     */
    extern bool ejsIsFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is a native function. Functions can be either native - meaning the implementation is
            via a C function, or can be scripted.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a native function.
        @ingroup EjsFunction
     */
    extern bool ejsIsNativeFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is an initializer. Initializers are special functions created by the compiler to do
            static and instance initialization of classes during construction.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is an initializer
        @ingroup EjsFunction
     */
    extern bool ejsIsInitializer(Ejs *ejs, EjsAny *obj);
#else
    //  OPT
    #define ejsIsFunction(ejs, obj)       (obj && POT(obj) && ((EjsPot*) obj)->isFunction)
    #define ejsIsNativeFunction(ejs, obj) (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isNativeProc))
    #define ejsIsInitializer(ejs, obj)    (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isInitializer)
#endif

/** 
    Bind a native C function to a function property
    @description Bind a native C function to an existing javascript function. Functions are typically created
        by compiling a script file of native function definitions into a mod file. When loaded, this mod file 
        will create the function properties. This routine will then bind the specified C function to the 
        function property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fun Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsBindFunction(Ejs *ejs, EjsAny *obj, int slotNum, void *fun);

/** 
    Clone a function
    @description Copy a function and create a new copy. This may do a shallow or deep copy. A shallow copy
        will not copy the property instances, rather it will only duplicate the property reference. A deep copy
        will recursively clone all the properties of the variable.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to clone
    @param deep Set to true to clone each property of the function. Otherwise object references will have their references
        copied and not the reference targets.
    @return The allocated activation object
    @ingroup EjsFunction
 */
PUBLIC EjsFunction *ejsCloneFunction(Ejs *ejs, EjsFunction *fun, int deep);

//  TODO - refactor into several functions
/** 
    Create a function object
    @description This creates a function object and optionally associates byte code with the function.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Function name used in stack backtraces.
    @param code Pointer to the byte code. The byte code is not copied so this must be a persistent pointer.
    @param codeLen Length of the code.
    @param numArgs Number of formal arguments to the function.
    @param numDefault Number of default args to the function.
    @param numExceptions Number of exception handlers
    @param returnType Return type of the function. Set to NULL for no defined type.
    @param attributes Integer mask of access attributes.
    @param module Reference to the module owning the function.
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
PUBLIC EjsFunction *ejsCreateFunction(Ejs *ejs, EjsString *name, cuchar *code, int codeLen, int numArgs, int numDefault,
    int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, EjsBlock *scope, 
    int strict);

/** 
    Initialize a function object
    @description This initializes a pre-existing function object and optionally associates byte code with the function.
        This is useful to create constructors which are stored inside type objects.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function object.
    @param name Function name used in stack backtraces.
    @param code Pointer to the byte code. The byte code is not copied so this must be a persistent pointer.
    @param codeLen Length of the code.
    @param numArgs Number of formal arguments to the function.
    @param numDefault Number of default args to the function.
    @param numExceptions Number of exception handlers
    @param returnType Return type of the function. Set to NULL for no defined type.
    @param attributes Integer mask of access attributes.
    @param module Reference to the module owning the function.
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
PUBLIC int ejsInitFunction(Ejs *ejs, EjsFunction *fun, EjsString *name, cuchar *code, int codeLen, int numArgs, 
    int numDefault, int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, 
    EjsBlock *scope, int strict);

/** 
    Create a bare function 
    @description This creates a function without code, exceptions or module linkage
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Function name
    @param attributes Function attributes
    @return The allocated function
    @ingroup EjsFunction
 */
PUBLIC EjsFunction *ejsCreateBareFunction(Ejs *ejs, EjsString *name, int attributes);

/** 
    Create an activation record for a function
    @description This creates an activation object that stores the local variables for a function
        This is a onetime operation and is not done for each function invocation.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to examine
    @param numSlots Number of local variables to reserve room for
    @return The allocated activation object
    @ingroup EjsFunction
 */
PUBLIC EjsPot *ejsCreateActivation(Ejs *ejs, EjsFunction *fun, int numSlots);

/** 
    Remove a constructor function from a type.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Type reference
    @ingroup EjsFunction
 */
PUBLIC void ejsRemoveConstructor(Ejs *ejs, struct EjsType *type);

/** 
    Run the initializer for a module
    @description A module's initializer runs global code defined in the module
    @param ejs Ejs reference returned from #ejsCreateVM
    @param module Module object reference
    @return The last expression result of global code executed
    @ingroup EjsFunction
 */
PUBLIC EjsObj *ejsRunInitializer(Ejs *ejs, struct EjsModule *module);

/** 
    Run a function
    @description Run a function with the given actual parameters
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fn Function object to run
    @param thisObj Object to use as the "this" object when running the function.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
PUBLIC EjsAny *ejsRunFunction(Ejs *ejs, EjsFunction *fn, EjsAny *thisObj, int argc, void *argv);

/** 
    Run a function by slot number
    @description Run a function identified by slot number with the given actual parameters. This will run the function
        stored at slotNum in the obj variable. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object that holds the function at its "slotNum" slot. Also use this object as the "this" object 
        when running the function.
    @param slotNum Slot number in obj that contains the function to run.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
PUBLIC EjsAny *ejsRunFunctionBySlot(Ejs *ejs, EjsAny *obj, int slotNum, int argc, void *argv);

/** 
    Run a function by name
    @description Run a function identified by name in the given container with the given actual parameters. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param container Object that holds the function at its "name". 
    @param qname Qualified name for the function in container.
    @param thisObj Object to use as "this" when invoking the function.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
PUBLIC EjsAny *ejsRunFunctionByName(Ejs *ejs, EjsAny *container, EjsName qname, EjsAny *thisObj, int argc, void *argv);

/** 
    Add an exception record
    @description This creates an exception record to define a catch or finally block.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to modify 
    @param tryStart Pointer to the bytecode start of try block
    @param tryEnd Pointer to one past the end of the try block
    @param catchType Type of error to catch. Set to null for all.
    @param handlerStart Pointer to the start of the catch / finally block 
    @param handlerEnd Pointer ot one past the end of the catch / finally block
    @param numBlocks Count of blocks opened before the try block
    @param numStack Count of stack slots pushed before the try block
    @param flags Reserved
    @param preferredIndex Preferred index in the function exceptions list. Set to -1 for the next available slot.
    @return The allocated exception object
    @ingroup EjsFunction
    @internal
 */
PUBLIC EjsEx *ejsAddException(Ejs *ejs, EjsFunction *fun, uint tryStart, uint tryEnd, struct EjsType *catchType,
    uint handlerStart, uint handlerEnd, int numBlocks, int numStack, int flags, int preferredIndex);

/** 
    Set the byte code for a function
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to examine
    @param module Module owning the function
    @param byteCode ByteCode buffer
    @param len Size of the byteCode buffer
    @param debug Debug record with symbolic debug information
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsFunction
    @internal
 */
PUBLIC int ejsSetFunctionCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, ssize len, 
    EjsDebug *debug);

/*
    Internal
 */

/** 
    Create a code block
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to examine
    @param module Module owning the function
    @param byteCode ByteCode buffer
    @param len Size of the byteCode buffer
    @param debug Debug record with symbolic debug information
    @return An allocated code block
    @ingroup EjsFunction
    @internal
 */
PUBLIC EjsCode *ejsCreateCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, ssize len, 
    EjsDebug *debug);
PUBLIC void ejsManageFunction(EjsFunction *fun, int flags);
PUBLIC void ejsShowOpFrequency(Ejs *ejs);

/******************************************** Frame ***********************************************/
/**
    Frame record 
    @defgroup EjsFrame EjsFrame
    @see ejsIsFrame
    @stability Internal
 */
typedef struct EjsFrame {
    EjsFunction     function;               /**< Activation frame for function calls. Stores local variables */
    EjsFunction     *orig;                  /**< Original function frame is based on */
    struct EjsFrame *caller;                /**< Previous invoking frame */
    EjsObj          **stackBase;            /**< Start of stack in this function */
    EjsObj          **stackReturn;          /**< Top of stack to return to */
    EjsLine         *line;                  /**< Debug source line */
    uchar           *pc;                    /**< Program counter */
    uchar           *attentionPc;           /**< Restoration PC value after attention */
    uint            argc;                   /**< Actual parameter count */
    int             slotNum;                /**< Slot in owner */
    uint            getter: 1;              /**< Frame is a getter */
} EjsFrame;

#if DOXYGEN
    /** 
        Determine if a variable is a frame. Only used internally in the VM.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a frame. 
        @ingroup EjsFrame
     */
    extern bool ejsIsFrame(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFrame(ejs, obj) (obj && ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isFrame)
#endif

/*
    Internal
 */
PUBLIC EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *src, EjsObj *thisObj, int argc, EjsObj **argv);
PUBLIC EjsFrame *ejsCreateCompilerFrame(Ejs *ejs, EjsFunction *src);
PUBLIC EjsBlock *ejsPopBlock(Ejs *ejs);
PUBLIC EjsBlock *ejsPushBlock(Ejs *ejs, EjsBlock *block);

/******************************************** Boolean *********************************************/
/** 
    Boolean class
    @description The Boolean class provides the base class for the boolean values "true" and "false".
        EjsBoolean is a primitive native type and extends EjsObj. It is still logically an Object, but implements
        Object properties and methods itself. Only two instances of the boolean class are ever created created
        these are referenced as ejs->trueValue and ejs->falseValue.
    @defgroup EjsBoolean EjsBoolean
    @see EjsBoolean ejsCreateBoolean ejsGetBoolean ejsToBoolean
    @stability Internal
 */
typedef struct EjsBoolean {
    EjsObj  obj;                /**< Base object */
    bool    value;              /**< Boolean value */
} EjsBoolean;


#if DOXYGEN
/** 
    Create a boolean
    @description Create a boolean value. This will not actually create a new boolean instance as there can only ever
        be two boolean instances (true and false). Boolean properties are immutable in Ejscript and so this routine
        will simply return the appropriate pre-created true or false boolean value.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Desired boolean value. Set to 1 for true and zero for false.
    @ingroup EjsBoolean
 */
PUBLIC EjsBoolean *ejsCreateBoolean(Ejs *ejs, int value);
#else
#define ejsCreateBoolean(ejs, v) ((v) ? ESV(true) : ESV(false))
#endif

/** 
    Get the C boolean value from a boolean object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Boolean variable to access
    @return True or false
    @ingroup EjsBoolean
 */
PUBLIC bool ejsGetBoolean(Ejs *ejs, EjsAny *obj);

/** 
    Cast a variable to a boolean 
    @description
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to cast
    @return A new boolean object
    @ingroup EjsBoolean
 */
PUBLIC EjsBoolean *ejsToBoolean(Ejs *ejs, EjsAny *obj);

/******************************************** ByteArray *******************************************/
/** 
    ByteArray class
    @description ByteArrays provide a resizable, integer indexed, in-memory store for bytes. ByteArrays can be used as a 
    simple array type to store and encode data as bytes or they can be used as buffered Streams implementing the Stream 
    interface.
    \n\n
    When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
    extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
    can be used to get and put blocks of data. In this mode, the read and write position properties are ignored. 
    Access to the byte array is from index zero up to the size defined by the length property. When constructed, 
    the ByteArray can be designated as resizable, in which case the initial size will grow as required to accomodate 
    data and the length property will be updated accordingly.
    \n\n
    When used as a Stream, the byte array additional write methods to store data at the location specified by the 
    $writePosition property and read methods to read from the $readPosition property. The $available method 
    indicates how much data is available between the read and write position pointers. The $reset method can 
    reset the pointers to the start of the array.  When used with for/in, ByteArrays will iterate or 
    enumerate over the available data between the read and write pointers.
    \n\n
    If numeric values are read or written, they will be encoded according to the value of the endian property 
    which can be set to either LittleEndian or BigEndian. 
    \n\n
    In Stream mode ByteArrays can be configured to run in sync or async mode. Adding observers via the $addObserver
    method will put a stream into async mode. Events will then be issued for close, EOF, read and write events.
    @defgroup EjsByteArray EjsByteArray
    @see EjsByteArray ejsCopyToByteArray ejsCreateByteArray ejsGetByteArrayAvailableData ejsGetByteArrayRoom 
        ejsGrowByteArray ejsMakeRoomInByteArray ejsResetByteArray ejsSetByteArrayPositions ejsWriteToByteArray 
    @stability Internal
 */
typedef struct EjsByteArray {
    EjsObj          obj;                /**< Base object */
    EjsObj          *emitter;           /**< Event emitter for listeners */
    uchar           *value;             /**< Data bytes in the array */
    int             async;              /**< Async mode */
    int             endian;             /**< Endian encoding */
    int             growInc;            /**< Current read position */
    ssize           readPosition;       /**< Current read position */
    ssize           writePosition;      /**< Current write position */
    ssize           size;               /**< Size property */
    int             swap;               /**< I/O must swap bytes due to endian byte ordering */
    bool            resizable;          /**< Aray is resizable */
} EjsByteArray;

/** 
    Create a byte array
    @description Create a new byte array instance.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param size Initial size of the byte array
    @return A new byte array instance
    @ingroup EjsByteArray
 */
PUBLIC EjsByteArray *ejsCreateByteArray(Ejs *ejs, ssize size);

/** 
    Set the I/O byte array positions
    @description Set the read and/or write positions into the byte array. ByteArrays implement the Stream interface
        and support sequential and random access reading and writing of data in the array. The byte array maintains
        read and write positions that are automatically updated as data is read or written from or to the array. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array object
    @param readPosition New read position to set
    @param writePosition New write position to set
    @ingroup EjsByteArray
 */
PUBLIC void ejsSetByteArrayPositions(Ejs *ejs, EjsByteArray *ba, ssize readPosition, ssize writePosition);

/** 
    Copy data into a byte array
    @description Copy data into a byte array at a specified offset. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array object
    @param offset Offset in the byte array to which to copy the data.
    @param data Pointer to the source data
    @param length Length of the data to copy
    @return Count of bytes written or negative MPR error code.
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, ssize offset, cchar *data, ssize length);

/** 
    Reset the byte
    @description This will reset the byte array read/write positions if the array is empty
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to modify
    @ingroup EjsByteArray
 */
PUBLIC void ejsResetByteArray(Ejs *ejs, EjsByteArray *ba);

/**
    Get the number of available bytes
    @param ba Byte array to examine
    @return The number of bytes of data available to read
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsGetByteArrayAvailableData(EjsByteArray *ba);

/**
    Determine the spare room in the byte array for more data
    @param ba Byte array to examine
    @return The number of bytes the byte array can fit without growing
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsGetByteArrayRoom(EjsByteArray *ba);

/**
    Grow the byte array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to grow
    @param size The requested new size of the byte array
    @return The new size of the byte array. Otherwise EJS_ERROR if the memory cannot be allocated.
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsGrowByteArray(Ejs *ejs, EjsByteArray *ba, ssize size);

/**
    Make room in the byte array for data
    @description This will ensure there is sufficient room in the byte array. If the required number of bytes of spare 
        room is not available, the byte array will grow.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to examine
    @param require Number of bytes needed.
    @return The number of bytes of data available to read
    @ingroup EjsByteArray
 */
PUBLIC bool ejsMakeRoomInByteArray(Ejs *ejs, EjsByteArray *ba, ssize require);

/**
    Write data to the byte array
    This implements the ByteArray.write function. It is most useful for other types to implement a write to byte 
        array capability.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to examine
    @param argc Count of args in argv
    @param argv Arguments to write
    @return The number of bytes of data written (EjsNumber)
    @ingroup EjsByteArray
 */
PUBLIC struct EjsNumber *ejsWriteToByteArray(Ejs *ejs, EjsByteArray *ba, int argc, EjsObj **argv);

/******************************************* Cache ************************************************/
/**
    EjsCache
    @defgroup EjsCache EjsCache
    @see ejsCacheExpire ejsCacheRead ejsCacheReadObj ejsCacheRemove ejsSetCacheLimits ejsCacheWrite ejsCacheWriteObj
 */

/** 
    Expire a cache item
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param when When to expire the cache item
    @return Returns Null
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheExpire(Ejs *ejs, EjsObj *cache, struct EjsString *key, struct EjsDate *when);

/** 
    Read an item from the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param options Cache read options
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheRead(Ejs *ejs, EjsObj *cache, struct EjsString *key, EjsObj *options);

/** 
    Read an object from the cache
    @description This call reads a cache item and then deserializes using JSON encoding into an object.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param options Cache read options
    @return Cache item object.
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheReadObj(Ejs *ejs, EjsObj *cache, struct EjsString *key, EjsObj *options);

/** 
    Read an item from the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC EjsBoolean *ejsCacheRemove(Ejs *ejs, EjsObj *cache, struct EjsString *key);

//  TODO - rename ejsSetCacheLimits
/** 
    Set the cache limits
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param limits Limits is an object hash. Depending on the cache backend in-use, the limits object may have
    some of the following properties. Consult the documentation for the actual cache backend for which properties
    are supported by the backend.
    <ul>
    <li> keys Maximum number of keys in the cache. Set to zero for no limit.</li>
    <li> lifespan Default time in seconds to preserve key data. Set to zero for no timeout.</li>
    <li> memory Total memory to allocate for cache keys and data. Set to zero for no limit.</li>
    <li> retries Maximum number of times to retry I/O operations with cache backends.</li>
    <li> timeout Maximum time to transact I/O operations with cache backends. Set to zero for no timeout.</li>
    </ul>
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheSetLimits(Ejs *ejs, EjsObj *cache, EjsObj *limits);

/** 
    Write an item to the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param value Value to write
    @param options Cache write options
    <ul>
    <li> lifespan Preservation time for the key in seconds.</li>
    <li> expire When to expire the key. Takes precedence over lifetime.</li>
    <li> mode Mode of writing: "set" is the default and means set a new value and create if required.
    "add" means set the value only if the key does not already exist. "append" means append to any existing
    value and create if required. "prepend" means prepend to any existing value and create if required.</li>
    <li> version Unique version identifier to be used for a conditional write. The write will only be 
    performed if the version id for the key has not changed. This implements an atomic compare and swap. </li>
    <li> throw Throw an exception rather than returning null if the version id has been updated for the key.</li>
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC struct EjsNumber *ejsCacheWrite(Ejs *ejs, EjsObj *cache, struct EjsString *key, struct EjsString *value, 
    EjsObj *options);

/** 
    Write an object to the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param value Value to write
    @param options Cache write options
    <ul>
    <li> lifespan Preservation time for the key in seconds.</li>
    <li> expire When to expire the key. Takes precedence over lifetime.</li>
    <li> mode Mode of writing: "set" is the default and means set a new value and create if required.
    "add" means set the value only if the key does not already exist. "append" means append to any existing
    value and create if required. "prepend" means prepend to any existing value and create if required.</li>
    <li> version Unique version identifier to be used for a conditional write. The write will only be 
    performed if the version id for the key has not changed. This implements an atomic compare and swap. </li>
    <li> throw Throw an exception rather than returning null if the version id has been updated for the key.</li>
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC struct EjsNumber *ejsCacheWriteObj(Ejs *ejs, EjsObj *cache, struct EjsString *key, EjsAny *value, EjsObj *options);

/******************************************** Cmd *************************************************/
/** 
    Cmd class
    @defgroup EjsCmd EjsCmd
    @stability Internal
 */
typedef struct EjsCmd {
    EjsPot          pot;                /**< Property storage */
    Ejs             *ejs;               /**< Interpreter back link */
    EjsObj          *emitter;           /**< Event emitter for listeners */
    MprCmd          *mc;                /**< MprCmd object */
    MprBuf          *stdoutBuf;         /**< Stdout from the command */
    MprBuf          *stderrBuf;         /**< Stderr from the command */
    EjsAny          *command;           /**< Command to run */
    EjsAny          *env;               /**< Optional environment */
    EjsAny          *options;           /**< Command options object */
    struct EjsByteArray *error;         /**< Error stream */
    cchar           **argv;             /**< Actual argv when invoking the command */
    int             argc;               /**< Length of argv */
    int             async;              /**< Async mode */
    int             throw;              /**< Set to true if the command should throw exceptions for failures */
    MprTicks        timeout;            /**< Command timeout in milliseconds */
} EjsCmd;

/******************************************** Date ************************************************/
/** 
    Date class
    @description The Date class is a general purpose class for working with dates and times. 
        is a a primitive native type and extends EjsObj. It is still logically an Object, but implements Object 
        properties and methods itself. 
    @defgroup EjsDate EjsDate
    @see EjsDate ejsCreateDate ejsGetDate ejsIsDate  
    @stability Internal
 */
typedef struct EjsDate {
    EjsObj          obj;                /**< Object base */
    MprTime         value;              /**< Time in milliseconds since "1970/01/01 GMT" */
} EjsDate;

/** 
    Create a new date instance
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Date/time value to set the new date instance to
    @return An initialized date instance
    @ingroup EjsDate
 */
PUBLIC EjsDate *ejsCreateDate(Ejs *ejs, MprTime value);

#if DOXYGEN
    /** 
        Get the numeric value stored in a EjsDate object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param date Date object to examine
        @return An MprTime value
        @ingroup EjsDate
     */
    extern MprTime ejsGetDate(Ejs *ejs, EjsDate *date);
#else
    #define ejsGetDate(ejs, obj) (ejsIs(ejs, obj, Date) ? ((EjsDate*) obj)->value : 0)
#endif

/******************************************** Error ***********************************************/
//  TODO - missing SecurityException PermissionsException
/** 
    Error classes
    @description Base class for error exception objects. Exception objects are created by programs and by the system 
    as part of changing the normal flow of execution when some error condition occurs. 
    When an exception is created and acted upon ("thrown"), the system transfers the flow of control to a 
    pre-defined instruction stream (the handler or "catch" code). The handler may return processing to the 
    point at which the exception was thrown or not. It may re-throw the exception or pass control up the call stack.
    @stability Evolving.
    @defgroup EjsError  EjsError
    @see ejeCreateError ejsCaptureStack ejsGetErrorMsg ejsGetException 
        ejsHasException ejsIsError ejsThrowArgError ejsThrowArithmeticError ejsThrowAssertError ejsThrowError 
        ejsThrowIOError ejsThrowInstructionError ejsThrowInternalError ejsThrowMemoryError ejsThrowOutOfBoundsError 
        ejsThrowReferenceError ejsThrowResourceError ejsThrowStateError ejsThrowStopIteration ejsThrowString 
        ejsThrowSyntaxError ejsThrowTypeError 
 */
typedef EjsPot EjsError;

#if DOXYGEN
    /**
        Test if the given object is an error instance
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @return True if the object is an error
        @ingroup EjsError
     */
    extern bool ejsIsError(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsError(ejs, obj) (obj && ejsIsA(ejs, obj, ESV(Error)))
#endif

/**
    Create an error object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Error base type
    @param message Error message to use when constructing the error object
    @return Error object
    @ingroup EjsError
 */
PUBLIC EjsError *ejsCreateError(Ejs *ejs, struct EjsType *type, EjsObj *message);

/**
    Capture the execution stack
    @param ejs Ejs reference returned from #ejsCreateVM
    @param skip How many levels of stack to skip before capturing (counts from the top down)
    @return Array of stack records.
    @ingroup EjsError
 */
PUBLIC EjsArray *ejsCaptureStack(Ejs *ejs, int skip);

/** 
    Get the interpreter error message
    @description Return a string containing the current interpreter error message
    @param ejs Ejs reference returned from #ejsCreateVM
    @param withStack Set to 1 to include a stack backtrace in the error message
    @return A string containing the error message. The caller must not free.
    @ingroup EjsError
 */
PUBLIC cchar *ejsGetErrorMsg(Ejs *ejs, int withStack);

/**
    Get the Ejs exception object for this interpreter
    @param ejs Ejs reference returned from #ejsCreateVM
    @return The exception object if one exists, otherwise NULL.
    @ingroup EjsError
 */
PUBLIC EjsObj *ejsGetException(Ejs *ejs);

/** 
    Determine if an exception has been thrown
    @param ejs Ejs reference returned from #ejsCreateVM
    @return True if an exception has been thrown
    @ingroup EjsError
 */
PUBLIC bool ejsHasException(Ejs *ejs);

/** 
    Throw an argument exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an assertion exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an math exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an instruction code exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an general error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an internal error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an IO exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an Memory depletion exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowMemoryError(Ejs *ejs);

/** 
    Throw an out of bounds exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an reference exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an resource exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an state exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an stop iteration exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsError
 */
PUBLIC EjsObj *ejsThrowStopIteration(Ejs *ejs);

/** 
    Throw a string message. This will not capture the stack as part of the exception message.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsString *ejsThrowString(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an syntax error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an type error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...);

/******************************************** File ************************************************/
/** 
    File class
    @description The File class provides a foundation of I/O services to interact with physical files and directories.
    Each File object represents a single file or directory and provides methods for creating, opening, reading, writing 
    and deleting files, and for accessing and modifying information about the file.
    @defgroup EjsFile EjsFile 
    @see EjsFile ejsCreateFile ejsCreateFileFromFd
    @stability Internal
 */
typedef struct EjsFile {
    EjsObj          obj;                /**< Base object */
    Ejs             *ejs;               /**< Interp reference */
    MprFile         *file;              /**< Open file handle */
    MprPath         info;               /**< Cached file info */
    char            *path;              /**< Filename path */
    char            *modeString;        /**< User supplied mode string */
    int             mode;               /**< Current open mode */
    int             perms;              /**< Posix permissions mask */
    int             attached;           /**< Attached to existing descriptor */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsFile;

/** 
    Create a File object
    @description Create a file object associated with the given filename. The filename is not opened, just stored.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param filename Filename to associate with the file object
    @return A new file object
    @ingroup EjsFile
 */
PUBLIC EjsFile *ejsCreateFile(Ejs *ejs, cchar *filename);

/**
    Create a file object from an O/S file descriptor
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fd O/S file descriptor handle
    @param name Filename to associate with the file object
    @param mode O/S file access mode (see man 2 open)
    @return A new file object
    @ingroup EjsFile
 */
PUBLIC EjsFile *ejsCreateFileFromFd(Ejs *ejs, int fd, cchar *name, int mode);

/******************************************** Path ************************************************/
/**
    Path class
    @description The Path class provides file path name services.
    @defgroup EjsPath EjsPath 
    @see EjsFile ejsCreatePath ejsCreatePathFromAsc ejsToPath
    @stability Internal
 */
typedef struct EjsPath {
    EjsObj          obj;                /**< Base object */
    cchar           *value;             /**< Filename path */
    MprPath         info;               /**< Cached file info */
    MprList         *files;             /**< File list for enumeration */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsPath;

/** 
    Create a Path object
    @description Create a path object associated with the given pathname.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path object
    @return A new Path object
    @ingroup EjsPath
 */
PUBLIC EjsPath *ejsCreatePath(Ejs *ejs, EjsString *path);

/** 
    Create a Path object
    @description Create a path object from the given ascii path string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Null terminated Ascii pathname.
    @return A new Path object
    @ingroup EjsPath
 */
PUBLIC EjsPath *ejsCreatePathFromAsc(Ejs *ejs, cchar *path);

/** 
    Convert the object to a Path
    @description Convert the object to a string and then to a Path.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A new Path object
    @ingroup EjsPath
 */
PUBLIC EjsPath *ejsToPath(Ejs *ejs, EjsAny *obj);

/** 
    Set the owner, group and permissions of a file.
    @description Convert the object to a string and then to a Path.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path name to modify
    @param options Owner, group and permissions options.
    @arg permissions optional Posix permissions number mask. Defaults to 0664.
    @arg owner String representing the file owner
    @arg group String representing the file group
    @return A new Path object
    @ingroup EjsPath
    @internal
 */
PUBLIC int ejsSetPathAttributes(Ejs *ejs, cchar *path, EjsObj *options);

/**
    Get files below a path. 
    @description Expand wild cards in a path. Function used to implement Path.files().
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path to use as the base 
    @param argc Count of args (set to 1)
    @param argv Args array. (Set to an array with a single element)
    @return Array of matching paths
  */
PUBLIC EjsArray *ejsGetPathFiles(Ejs *ejs, EjsPath *path, int argc, EjsObj **argv);

/******************************************** FileSystem*******************************************/
/** 
    FileSystem class
    @description The FileSystem class provides file system services.
    @defgroup EjsFileSystem EjsFileSystem 
    @see EjsFile ejsCreateFile 
    @stability Internal
 */
typedef struct EjsFileSystem {
    EjsObj          obj;                /**< Base object */
    char            *path;              /**< Filename path */
    MprFileSystem   *fs;                /**< MPR file system object */
} EjsFileSystem;


/** 
    Create a FileSystem object
    @description Create a file system object associated with the given pathname.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path to describe the file system. Can be any path in the file system.
    @return A new file system object
    @ingroup EjsPath
 */
PUBLIC EjsFileSystem *ejsCreateFileSystem(Ejs *ejs, cchar *path);

/*
    Internal
 */
PUBLIC void ejsFreezeGlobal(Ejs *ejs);
PUBLIC void ejsCreateGlobalNamespaces(Ejs *ejs);
PUBLIC void ejsDefineGlobalNamespaces(Ejs *ejs);

/******************************************** Http ************************************************/
/** 
    Http Class
    @description
        Http objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @defgroup EjsHttp EjsHttp
    @see EjsHttp ejsCreateHttp ejsGetHttpLimits ejsSetHttpLimits ejsSetupHttpTrace ejsLoadHttpService
    @stability Internal
 */
typedef struct EjsHttp {
    EjsObj          obj;                        /**< Base object */
    Ejs             *ejs;                       /**< Interp reference */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsByteArray    *data;                      /**< Buffered write data */
    EjsObj          *limits;                    /**< Limits object */
    EjsString       *responseCache;             /**< Cached response (only used if response() is used) */
    HttpConn        *conn;                      /**< Http connection object */
    MprSsl          *ssl;                       /**< SSL configuration */
    MprBuf          *requestContent;            /**< Request body data supplied */
    MprBuf          *responseContent;           /**< Response data */
    char            *uri;                       /**< Target uri */
    char            *method;                    /**< HTTP method */
    char            *keyFile;                   /**< SSL key file */
    char            *caFile;                    /**< SSL CA certificate file */
    char            *certFile;                  /**< SSL certificate file */
    int             closed;                     /**< Http is closed and "close" event has been issued */
    int             error;                      /**< Http errored and "error" event has been issued */
    ssize           readCount;                  /**< Count of body bytes read */
    ssize           requestContentCount;        /**< Count of bytes written from requestContent */
    ssize           writeCount;                 /**< Count of bytes written via write() */
} EjsHttp;

/*
    Thse constants match Stream.READ, Stream.WRITE, Stream.BOTH
 */
#define EJS_STREAM_READ     0x1         /**< Http constant Stream.Read */
#define EJS_STREAM_WRITE    0x2         /**< Http constant Stream.Write */
#define EJS_STREAM_BOTH     0x3         /**< Http constant Stream.Both */

/** 
    Create a new Http object
    @param ejs Ejs reference returned from #ejsCreateVM
    @return a new Http object
    @ingroup EjsHttp
 */
PUBLIC EjsHttp *ejsCreateHttp(Ejs *ejs);

/**
    Get a Http limits 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to contain the limits properties
    @param limits The HttpLimits object 
    @param server Set to true if defining server side limits
    @ingroup EjsHttp
    @internal
 */
PUBLIC void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, bool server);

/** 
    Load the Http service
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsPath
    @internal
 */
PUBLIC void ejsLoadHttpService(Ejs *ejs);

/**
    Set a Http limits 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param limits The HttpLimits object receiving the limit settings
    @param obj Object containing the limits values
    @param server Set to true if defining server side limits
    @ingroup EjsHttp
    @internal
 */
PUBLIC void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, bool server);

/** 
    Setup tracing for Http transactions
    @param ejs Ejs reference returned from #ejsCreateVM
    @param trace HttpTrace object
    @param options Trace options
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPath
    @internal
 */
PUBLIC int ejsSetupHttpTrace(Ejs *ejs, HttpTrace *trace, EjsObj *options);

/******************************************** WebSocket ************************************************/
/** 
    WebSocket Class
    @description Client side WebSocket support
    @defgroup EjsWebSocket EjsWebSocket
    @see EjsWebSocket ejsCreateWebSocket 
    @stability Internal
 */
typedef struct EjsWebSocket {
    EjsPot          pot;                        /**< Base pot */
    Ejs             *ejs;                       /**< Interp reference */
    EjsObj          *emitter;                   /**< Event emitter */
    HttpConn        *conn;                      /**< Underlying HttpConn object */
    MprSsl          *ssl;                       /**< SSL configuration */
    char            *certFile;                  /**< SSL certificate file */
    char            *uri;                       /**< Target URI */
    char            *protocols;                 /**< Set of supported protocols */
    char            *protocol;                  /**< Protocol selected by the server */
    int             closed;                     /**< Http is closed and "close" event has been issued */
    int             error;                      /**< Http errored and "error" event has been issued */
    int             frames;                     /**< Preserve frames */
} EjsWebSocket;

/** 
    Create a new WebSocket object
    @param ejs Ejs reference returned from #ejsCreateVM
    @return a new WebSocket object
    @ingroup EjsWebSocket
 */
PUBLIC EjsWebSocket *ejsCreateWebSocket(Ejs *ejs);


/******************************************** Iterator ********************************************/
/** 
    Iterator Class
    @description Iterator is a helper class to implement iterators in other native classes
    @defgroup EjsIterator EjsIterator
    @see EjsIterator ejsCreateIterator
    @stability Internal
 */
typedef struct EjsIterator {
    EjsObj          obj;                /**< Base object */
    EjsObj          *target;            /**< Object to be enumerated */
    EjsProc         nativeNext;         /**< Native next function */
    bool            deep;               /**< Iterator deep (recursively over all properties) */
    EjsArray        *namespaces;        /**< Namespaces to consider in iteration */
    int             index;              /**< Current index */
    int             length;             /**< Collection length prior to iteration */
    EjsObj          *indexVar;          /**< Reference to current item */
} EjsIterator;

/** 
    Create an iterator object
    @description The EjsIterator object is a helper class for native types to implement iteration and enumeration.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param target Target variable to iterate or enumerate 
    @param length Length of collection prior to iteration.
    @param next Function to invoke to step to the next element
    @param deep Set to true to do a deep iteration/enumeration
    @param namespaces Reserved and not used. Supply NULL.
    @return A new EjsIterator object
    @ingroup EjsIterator
 */
PUBLIC EjsIterator *ejsCreateIterator(Ejs *ejs, EjsAny *target, int length, void *next, bool deep, EjsArray *namespaces);

/******************************************** Namespace *******************************************/
/** 
    Namespace Class
    @description Namespaces are used to qualify names into discrete spaces.
    @defgroup EjsNamespace EjsNamespace
    @see EjsNamespace ejsCreateNamespace ejsCreateReservedNamespace ejsDefineReservedNamespace ejsFormatReservedNamespace 
    @stability Internal
 */
typedef struct EjsNamespace {
    EjsObj          obj;                /**< Base object */
    EjsString       *value;             /**< Textual name of the namespace */
} EjsNamespace;

/** 
    Create a namespace object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Space name to use for the namespace
    @return A new namespace object
    @ingroup EjsNamespace
 */
PUBLIC EjsNamespace *ejsCreateNamespace(Ejs *ejs, EjsString *name);

/** 
    Create a reserved namespace
    @param ejs Ejs reference returned from #ejsCreateVM
    @param typeName Type on which to base the formatted namespace name
    @param name Formatted base name for the namespace
    @return A new namespace object
    @ingroup EjsNamespace
 */
PUBLIC EjsNamespace *ejsCreateReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *name);

/** 
    Define a reserved namespace on a block
    @param ejs Ejs reference returned from #ejsCreateVM
    @param block Block to modify
    @param typeName Type on which to base the formatted namespace name
    @param name Formatted base name for the namespace
    @param block Block to modify
    @return A new namespace object
    @ingroup EjsNamespace
 */
PUBLIC EjsNamespace *ejsDefineReservedNamespace(Ejs *ejs, EjsBlock *block, EjsName *typeName, cchar *name);

/** 
    Format a reserved namespace name to create a unique namespace. 
    @description This is used to extend the "internal", "public", "private", and "protected" namespaces to be 
    unique for their owning class.
    \n\n
    Namespaces are formatted as strings using the following format, where type is optional. Types may be qualified.
        [type,space]
    \n\n
    Example: [debug::Shape,public] where Shape was declared as "debug class Shape"
    @param ejs Ejs reference returned from #ejsCreateVM
    @param typeName Type on which to base the formatted namespace name
    @param spaceName Namespace name
    @return A string containing the formatted name
    @ingroup EjsNamespace
 */
PUBLIC EjsString *ejsFormatReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *spaceName);

/******************************************** Null ************************************************/
/** 
    Null Class
    @description The Null class provides the base class for the singleton null instance. This instance is stored
        in ejs->nullValue.
    @stability Evolving
    @defgroup EjsNull EjsNull
    @see EjsNull ejsCreateNull
 */
typedef EjsObj EjsNull;

/** 
    Create the null object
    @description There is one null object in the system.
    @param ejs Ejs reference returned from #ejsCreateVM
    @return The null object
    @ingroup EjsNull
    @internal
 */
PUBLIC EjsNull *ejsCreateNull(Ejs *ejs);

/******************************************** Number **********************************************/
/** 
    Number class
    @description The Number class provide the base class for all numeric values. 
        The primitive number storage data type may be set via the configure program to be either double, float, int
        or int64. 
    @defgroup EjsNumber EjsNumber
    @see EjsNumber ejsCreateNumber ejsGetDouble ejsGetInt ejsGetInt64 ejsGetNumber ejsIsInfinite ejsIsNan ejsToNumber 
    @stability Internal
 */
typedef struct EjsNumber {
    EjsObj      obj;                /**< Base object */
    MprNumber   value;              /**< Numeric value */
} EjsNumber;

/** 
    Create a number object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Numeric value to initialize the number object
    @return A number object
    @ingroup EjsNumber
 */
PUBLIC EjsNumber *ejsCreateNumber(Ejs *ejs, MprNumber value);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return A double value
    @ingroup EjsNumber
 */
PUBLIC double ejsGetDouble(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return An integer value
    @ingroup EjsNumber
 */
PUBLIC int ejsGetInt(Ejs *ejs, EjsAny *obj);

/** 
    Get an 64 bit integer value equivalent to that stored in an EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return A 64 bit integer value
    @ingroup EjsNumber
 */
PUBLIC int64 ejsGetInt64(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return A numeric value
    @ingroup EjsNumber
 */
PUBLIC MprNumber ejsGetNumber(Ejs *ejs, EjsAny *obj);

/** 
    Test if a number is infinite
    @param n Number to test
    @return True if the number is infinite
    @ingroup EjsNumber
 */
PUBLIC bool ejsIsInfinite(MprNumber n);

#if DOXYGEN
    /** 
        Test if a value is not-a-number
        @param n Number to test
        @return True if the number is not-a-number
        @ingroup EjsNumber
     */
    extern bool ejsIsNan(MprNumber n);
#elif WINDOWS
    #define ejsIsNan(f) (_isnan(f))
#elif MACOSX || LINUX || VXWORKS
    #define ejsIsNan(f) isnan(f)
#else
    #define ejsIsNan(f) (fpclassify(f) == FP_NAN)
#endif

/** 
    Cast a variable to a number
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to cast
    @return A number object
    @ingroup EjsNumber
 */
PUBLIC struct EjsNumber *ejsToNumber(Ejs *ejs, EjsAny *obj);

/******************************************** RegExp **********************************************/
/** 
    RegExp Class
    @description The regular expression class provides string pattern matching and substitution.
    @defgroup EjsRegExp EjsRegExp
    @see EjsRegExp ejsCreateRegExp ejsRegExpToString
    @stability Internal
 */
typedef struct EjsRegExp {
    EjsObj          obj;                /**< Base object */
    wchar           *pattern;           /**< Pattern to match */
    void            *compiled;          /**< Compiled pattern (not alloced) */
    bool            global;             /**< Search for pattern globally (multiple times) */
    bool            ignoreCase;         /**< Do case insensitive matching */
    bool            multiline;          /**< Match patterns over multiple lines */
    bool            sticky;
    int             options;            /**< Pattern matching options */
    int             endLastMatch;       /**< End of the last match (one past end) */
    int             startLastMatch;     /**< Start of the last match */
    EjsString       *matched;           /**< Last matched component */
} EjsRegExp;

/** 
    Create a new regular expression object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param pattern Regular expression pattern string. The regular expression string should not contain the leading or
        trailing slash. Embedded slash characters should not be back-quoted.
    @param flags Regular expression flags. Support flags include "g" for global match, "i" to ignore case, "m" match over
        multiple lines, "y" for sticky match.
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
PUBLIC EjsRegExp *ejsCreateRegExp(Ejs *ejs, cchar *pattern, cchar *flags);

/** 
    Parse a string and create a regular expression object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param pattern Regular expression pattern string
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
PUBLIC EjsRegExp *ejsParseRegExp(Ejs *ejs, EjsString *pattern);

/** 
    Get a string representation of a regular expression
    @param ejs Ejs reference returned from #ejsCreateVM
    @param rp Regular expression 
    @return A string representation of a regular expression. The result will be of the format: "/PATTERN/suffixes"
    @ingroup EjsRegExp
 */
PUBLIC EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp);

/******************************************** Socket **********************************************/
/**
    Socket Class
    @description
    @defgroup EjsSocket EjsSocket
    @see EjsSocket ejsCreateSocket
    @stability Internal
 */
typedef struct EjsSocket {
    EjsObj          obj;                /**< Base object */
    Ejs             *ejs;               /**< Interp reference */
    EjsObj          *emitter;           /**< Event emitter */
    EjsByteArray    *data;              /**< Buffered write data */
    MprSocket       *sock;              /**< Underlying MPR socket object */
    char            *address;           /**< Remote address */
    int             port;               /**< Remote port */
    int             async;              /**< In async mode */
    int             mask;               /**< IO event mask */
    MprMutex        *mutex;             /**< Multithread sync */
} EjsSocket;

/** 
    Create a new Socket object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sock Socket object
    @param async True if running in async non-blocking mode
    @return a new Socket object
    @ingroup EjsSocket
 */
PUBLIC EjsSocket *ejsCreateSocket(Ejs *ejs, MprSocket *sock, bool async);

/******************************************** Timer ***********************************************/
/** 
    Timer Class
    @description Timers manage the scheduling and execution of Ejscript functions. Timers run repeatedly 
        until stopped by calling the stop method and are scheduled with a granularity of 1 millisecond. 
    @defgroup EjsTimer EjsTimer
    @see EjsTimer
    @stability Internal
 */
typedef struct EjsTimer {
    EjsObj          obj;                /**< Base object */
    Ejs             *ejs;               /**< Interp reference - needed for background timers */
    MprEvent        *event;             /**< MPR event for the timer */
    int             drift;              /**< Timer event is allowed to drift if system conditions requrie */
    int             repeat;             /**< Timer repeatedly fires */
    int             period;             /**< Time in msec between invocations */          
    EjsFunction     *callback;          /**< Callback function */
    EjsFunction     *onerror;           /**< onerror function */
    EjsArray        *args;              /**< Callback args */
} EjsTimer;

/******************************************** Uri *************************************************/
/** 
    Uri class
    @description The Uri class provides file path name services.
    @defgroup EjsUri EjsUri 
    @see EjsFile ejsCreateUri ejsCreateUriFromAsc ejsCreateUriFromParts ejsToUri
    @stability Internal
 */
typedef struct EjsUri {
    EjsObj      obj;            /**< Base object */
    HttpUri     *uri;           /**< Decoded URI */
} EjsUri;


/** 
    Create a Uri object
    @description Create a URI object associated with the given URI string.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param uri Uri string to parse
    @return A new Uri object
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsCreateUri(Ejs *ejs, EjsString *uri);

/** 
    Create a URI from an ascii path
    @param ejs Ejs reference returned from #ejsCreateVM
    @param uri URI Ascii string 
    @return A new URI object
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsCreateUriFromAsc(Ejs *ejs, cchar *uri);

/** 
    @description This call constructs a URI from the given parts. Various URI parts can be omitted by setting to null.
        The URI path is the only mandatory parameter.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param scheme The URI scheme. This is typically "http" or "https".
    @param host The URI host name portion. This can be a textual host and domain name or it can be an IP address.
    @param port The URI port number. Set to zero to accept the default value for the selected scheme.
    @param path The URI path to the requested document.
    @param reference URI reference with an HTML document. This is the URI component after the "#" in the URI path.
    @param query URI query component. This is the URI component after the "?" in the URI.
    @param flags Set to HTTP_COMPLETE_URI to complete the URI by supplying missing URI parts with default values.
    @return A new URI 
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsCreateUriFromParts(Ejs *ejs, cchar *scheme, cchar *host, int port, cchar *path, cchar *query, 
        cchar *reference, int flags);

/** 
    Convert an object to a URI
    @description the object is first converted to a String and then to a URI.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Any object
    @return A new URI object
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsToUri(Ejs *ejs, EjsAny *obj);

/******************************************** Worker **********************************************/
/** 
    Worker Class
    @description The Worker class provides the ability to create new interpreters in dedicated threads
    @defgroup EjsWorker EjsWorker
    @see EjsObj ejsCreateWorker ejsRemoveWorkers
    @stability Internal
 */
typedef struct EjsWorker {
    EjsPot          pot;                /**< Property storage */
    char            *name;              /**< Optional worker name */
    Ejs             *ejs;               /**< Interpreter */
    EjsAny          *event;             /**< Current event object */
    struct EjsWorker *pair;             /**< Corresponding worker object in other thread */
    char            *scriptFile;        /**< Script or module to run */
    EjsString       *scriptLiteral;     /**< Literal script string to run */
    int             state;              /**< Worker state */
    int             inside;             /**< Running inside the worker */
    int             complete;           /**< Worker has completed its work */
    int             gotMessage;         /**< Worker has received a message */
} EjsWorker;

#define EJS_WORKER_BEGIN        1                   /**< Worker state before starting */
#define EJS_WORKER_STARTED      2                   /**< Worker state once started a script */
#define EJS_WORKER_CLOSED       3                   /**< Worker state when finished */
#define EJS_WORKER_COMPLETE     4                   /**< Worker state when completed all messages */

/** 
    Create a worker
    @description This creates a bare worker object
    @param ejs Ejs reference returned from #ejsCreateVM
    @return A new worker object
    @ingroup EjsWorker
 */
PUBLIC EjsWorker *ejsCreateWorker(Ejs *ejs);

/** 
    Remove workers before exiting
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsWorker
    @internal
 */
PUBLIC void ejsRemoveWorkers(Ejs *ejs);

/******************************************** Void ************************************************/
/** 
    Void class
    @description The Void class provides the base class for the singleton "undefined" instance. This instance is stored
        in ejs->undefinedValue..
    @stability Evolving
    @defgroup EjsVoid EjsVoid
    @see EjsVoid ejsCreateUndefined
 */

typedef EjsObj EjsVoid;

/** 
    Create the undefined object
    @description There is one undefined object in the system.
    @param ejs Ejs reference returned from #ejsCreateVM
    @return The undefined object
    @ingroup EjsVoid
    @internal
 */
PUBLIC EjsVoid *ejsCreateUndefined(Ejs *ejs);

/******************************************** XML *************************************************/
/*  
    Xml defines
 */
#define E4X_TEXT_PROPERTY       "-txt"
#define E4X_TAG_NAME_PROPERTY   "-tag"
#define E4X_COMMENT_PROPERTY    "-com"
#define E4X_ATTRIBUTES_PROPERTY "-att"
#define E4X_PI_PROPERTY         "-pi"
#define E4X_PARENT_PROPERTY     "-parent"

#define EJS_XML_FLAGS_TEXT      0x1             /* Node is a text node */
#define EJS_XML_FLAGS_PI        0x2             /* Node is a processing instruction */
#define EJS_XML_FLAGS_COMMENT   0x4             /* Node is a comment */
#define EJS_XML_FLAGS_ATTRIBUTE 0x8             /* Node is an attribute */
#define EJS_XML_FLAGS_ELEMENT   0x10            /* Node is an element */

/*  
    XML node kinds
 */
#define EJS_XML_LIST        1                   /**< XML node is a list */
#define EJS_XML_ELEMENT     2                   /**< XML node is an element */
#define EJS_XML_ATTRIBUTE   3                   /**< XML node is an attribute */
#define EJS_XML_TEXT        4                   /**< XML node is text */
#define EJS_XML_COMMENT     5                   /**< XML node is a comment */
#define EJS_XML_PROCESSING  6                   /**< XML node is a processing instruction */

/** 
    Xml tag state
    @ingroup EjsXML
    @stability Internal
 */
typedef struct EjsXmlTagState {
    struct EjsXML   *obj;                       /**< Current object */
    EjsObj          *attributes;                /**< List of attributes */
    EjsObj          *comments;                  /**< List of comments */
} EjsXmlTagState;


/*  
    Xml Parser state
    @ingroup EjsXML
    @stability Internal
 */
typedef struct EjsXmlState {
    //  TODO -- should not be fixed but should be growable
    EjsXmlTagState  nodeStack[ME_XML_MAX_NODE_DEPTH];      /**< nodeStack */
    Ejs             *ejs;                                   /**< Convenient reference to ejs */
    struct EjsType  *xmlType;                               /**< Xml type reference */
    struct EjsType  *xmlListType;                           /**< Xml list type reference */
    int             topOfStack;                             /**< Pointer to the top of state stack */
    ssize           inputSize;                              /**< Size of input to parse */
    ssize           inputPos;                               /**< Current input position */
    cchar           *inputBuf;                              /**< Input buffer */
    cchar           *filename;                              /**< Input filename */
} EjsXmlState;


/** 
    XML and XMLList class
    @description The XML class and API is based on ECMA-357 -- ECMAScript for XML (E4X). The XML class is a 
    core class in the E4X specification; it provides the ability to load, parse and save XML documents.
    @defgroup EjsXML EjsXML
    @see EjsXML ejsAppendAttributeToXML ejsAppendToXML ejsConfigureXML ejsCreateXML ejsCreateXMLList ejsDeepCopyXML 
        ejsGetXMLDescendants ejsIsXML ejsLoadXMLAsc ejsLoadXMLString ejsSetXML ejsXMLToBuf 
    @stability Internal
 */
typedef struct EjsXML {
    EjsObj          obj;                /**< Base object */
    EjsName         qname;              /**< XML node name (e.g. tagName) */
    int             kind;               /**< Kind of XML node */
    MprList         *elements;          /**< List elements or child nodes */
    MprList         *attributes;        /**< Node attributes */
    MprList         *namespaces;        /**< List of namespaces as Namespace objects */
    struct EjsXML   *parent;            /**< Parent node reference (XML or XMLList) */
    struct EjsXML   *targetObject;      /**< XML/XMLList object modified when items inserted into an empty list */
    EjsName         targetProperty;     /**< XML property modified when items inserted into an empty list */
    EjsString       *value;             /**< Value of text|attribute|comment|pi */
    int             flags;
} EjsXML;

#if DOXYGEN
    /** 
        Determine if a variable is an XML object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to test
        @return true if the variable is an XML or XMLList object
        @ingroup EjsXML
     */
    extern bool ejsIsXML(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsXML(ejs, obj) (ejsIs(ejs, obj, XML) || ejsIs(ejs, obj, XMLList))
#endif

/** 
    Append an attribute
    @param ejs Ejs reference returned from #ejsCreateVM
    @param parent Xml node to receive the attribute
    @param attribute Attribute node to append.
    @return A new XML object
    @ingroup EjsXML
 */
PUBLIC int ejsAppendAttributeToXML(Ejs *ejs, EjsXML *parent, EjsXML *attribute);

/** 
    Append a node
    @param ejs Ejs reference returned from #ejsCreateVM
    @param dest Node to receive the appended node
    @param node Node to append to dest
    @return The dest node
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsAppendToXML(Ejs *ejs, EjsXML *dest, EjsXML *node);

/** 
    Create an XML node object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param kind XML node kind. Set to EJS_XML_LIST, EJS_XML_ELEMENT, EJS_XML_ATTRIBUTE, EJS_XML_TEXT, EJS_XML_COMMENT or
        EJS_XML_PROCESSING.
    @param name Node name. Only the EjsName.name field is used.
    @param parent Parent node
    @param value Node value
    @return A new XML object
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsCreateXML(Ejs *ejs, int kind, EjsName name, EjsXML *parent, EjsString *value);

/** 
    Create an XML list object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param targetObject Object to set as the target object for the list
    @param targetProperty Property to set as the target property for the list
    @return A new XML list object
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsCreateXMLList(Ejs *ejs, EjsXML *targetObject, EjsName targetProperty);

/** 
    Get the descendants of an XML node that match the given name
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml Node to examine
    @param qname Name to search for
    @return An XML node with elements for the descendants.
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsGetXMLDescendants(Ejs *ejs, EjsXML *xml, EjsName qname);

/** 
    Load an XML document
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml XML node to hold the parsed XML data.
    @param xmlString String containing XML data to parse
    @ingroup EjsXML
 */
PUBLIC void ejsLoadXMLString(Ejs *ejs, EjsXML *xml, EjsString *xmlString);

/** 
    Load an XML document from an Ascii string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml XML node to hold the parsed XML data.
    @param xmlString String containing XML data to parse
    @return A new XML object
    @ingroup EjsXML
 */
PUBLIC void ejsLoadXMLAsc(Ejs *ejs, EjsXML *xml, cchar *xmlString);

/** 
    Set an indexed element to a value
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml XML node to receive the appended node
    @param index Element index at which to set the node
    @param node Node to insert
    @return The xml node
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsSetXMLElement(Ejs *ejs, EjsXML *xml, int index, EjsXML *node);

/** 
    Convert an xml node to string representation in a buffer 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param buf Buffer to hold the output string
    @param xml Node to examine
    @param indentLevel Maximum indent level
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsXML
 */
PUBLIC int ejsXMLToBuf(Ejs *ejs, MprBuf *buf, EjsXML *xml, int indentLevel);

/*  
    Internal
 */
PUBLIC EjsXML *ejsConfigureXML(Ejs *ejs, EjsXML *xml, int kind, EjsString *name, EjsXML *parent, EjsString *value);
PUBLIC void ejsManageXML(EjsXML *xml, int flags);
PUBLIC MprXml *ejsCreateXmlParser(Ejs *ejs, EjsXML *xml, cchar *filename);

/******************************************** Type ************************************************/
/** 
    Allocation and Type Helpers
    @description The type helpers interface defines the set of primitive operations a type must support to
        interact with the virtual machine.
    @ingroup EjsType
    @stability Internal
 */
typedef struct EjsHelpers {
    /* Used by objects and values */
    EjsAny  *(*cast)(struct Ejs *ejs, EjsAny *obj, struct EjsType *type);
    EjsAny  *(*clone)(struct Ejs *ejs, EjsAny *obj, bool deep);
    EjsAny  *(*create)(struct Ejs *ejs, struct EjsType *type, int size);
    int     (*defineProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                int64 attributes, EjsAny *value);
    int     (*deleteProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    int     (*deletePropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    EjsAny  *(*getProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*getPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*getPropertyCount)(struct Ejs *ejs, EjsAny *obj);
    EjsName (*getPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    struct EjsTrait *(*getPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*invokeOperator)(struct Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
    int     (*lookupProperty)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*setProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
    int     (*setPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
    int     (*setPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
    int     (*setPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);
} EjsHelpers;

typedef EjsAny  *(*EjsCreateHelper)(Ejs *ejs, struct EjsType *type, int size);
typedef EjsAny  *(*EjsCastHelper)(Ejs *ejs, EjsAny *obj, struct EjsType *type);
typedef EjsAny  *(*EjsCloneHelper)(Ejs *ejs, EjsAny *obj, bool deep);
typedef int     (*EjsDefinePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                    int64 attributes, EjsAny *value);
typedef int     (*EjsDeletePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsDeletePropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef EjsAny  *(*EjsGetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsGetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef struct EjsTrait *(*EjsGetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsGetPropertyCountHelper)(Ejs *ejs, EjsAny *obj);
typedef EjsName (*EjsGetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsInvokeOperatorHelper)(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
typedef int     (*EjsLookupPropertyHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef int     (*EjsSetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
typedef int     (*EjsSetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
typedef int     (*EjsSetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
typedef int     (*EjsSetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

/** 
    Type class
    @description Classes in Ejscript are represented by instances of an EjsType. 
        Types are templates for creating instances of the given type, but they are also are runtime accessible objects.
        Types contain the static properties and methods for objects and store these in their object slots array. 
        They store the instance properties in the type->instance object. EjsType inherits from EjsBlock, EjsObj 
        and EjsObj. 
    @defgroup EjsType EjsType
    @see EjsType ejsBindAccess ejsBindConstructor ejsBindMethod ejsCast ejsConfigureType ejsCreateArchetype 
        ejsCreateCoreType ejsCreatePrototype ejsCreateType ejsDefineGlobalFunction ejsDefineInstanceProperty 
        ejsFinalizeCoreType ejsFinalizeScriptType ejsGetPrototype ejsGetType ejsGetTypeByName ejsGetTypeof ejsIs 
        ejsIsA ejsIsDefined ejsIsType ejsIsTypeSubType 
    @stability Internal
 */
typedef struct EjsType {
    EjsFunction     constructor;                    /**< Constructor function and type properties */
    EjsName         qname;                          /**< Qualified name of the type. Type name and namespace */
    EjsPot          *prototype;                     /**< Prototype for instances when using prototype inheritance (only) */
    EjsHelpers      helpers;                        /**< Type helper methods */
    struct EjsType  *baseType;                      /**< Base class */
    MprManager      manager;                        /**< Manager callback */
    MprMutex        *mutex;                         /**< Optional locking for types that require it */
    MprList         *implements;                    /**< List of implemented interfaces */
        
    uint            callsSuper           : 1;       /**< Constructor calls super() */
    uint            configured           : 1;       /**< Type has been configured with native methods */
    uint            dynamicInstances     : 1;       /**< Object instances may add properties */
    uint            final                : 1;       /**< Type is final */
    uint            hasBaseConstructors  : 1;       /**< Base types has constructors */
    uint            hasBaseInitializers  : 1;       /**< Base types have initializers */
    uint            hasConstructor       : 1;       /**< Type has a constructor */
    uint            hasInitializer       : 1;       /**< Type has static level initialization code */
    uint            hasInstanceVars      : 1;       /**< Type has non-function instance vars (state) */
    uint            hasMeta              : 1;       /**< Type has meta methods */
    uint            hasScriptFunctions   : 1;       /**< Block has non-native functions requiring namespaces */
    uint            initialized          : 1;       /**< Static initializer has run */
    uint            isInterface          : 1;       /**< Interface vs class */
    uint            isPot                : 1;       /**< Instances are based on EjsPot */
    uint            mutable              : 1;       /**< Type is mutable (has changable state) */
    uint            mutableInstances     : 1;       /**< Instances are mutable */
    uint            needFixup            : 1;       /**< Slots need fixup */
    uint            numericIndicies      : 1;       /**< Instances support direct numeric indicies */
    uint            virtualSlots         : 1;       /**< Properties are not stored in slots[] */
    
    int             endClass;                       /**< Offset in mod file for end of class */
    ushort          numInherited;                   /**< Number of inherited prototype properties */
    ushort          instanceSize;                   /**< Size of instances in bytes */
    short           sid;                            /**< Slot index into service->immutable[] */
    struct EjsModule *module;                       /**< Module owning the type - stores the constant pool */
    void            *typeData;                      /**< Type specific data */
} EjsType;


#if DOXYGEN
    /** 
        Determine if a variable is an type
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a type
        @ingroup EjsType
     */
    extern bool ejsIsType(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if a variable is a prototype object. Types store the template for instance properties in a prototype object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a prototype object.
        @ingroup EjsType
     */
    extern bool ejsIsPrototype(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsType(ejs, obj)       (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isType))
    #define ejsIsPrototype(ejs, obj)  (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isPrototype))
#endif

/** 
    Bind a native C functions to method accessors
    @description Bind a native C function to an existing javascript accessor function. Method functions are typically created
        by compiling a script file of native method definitions into a mod file. When loaded, this mod file will create
        the method properties. This routine will then bind the specified C function to the method accessor.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Type containing the function property to bind.
    @param slotNum Slot number of the method property
    @param getter Native C getter function to bind. Set to NULL if no getter.
    @param setter Native C setter function to bind. Set to NULL if no setter.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsBindAccess(Ejs *ejs, EjsAny *obj, int slotNum, void *getter, void *setter);

/** 
    Bind a constructor
    @description Bind a native C function to a type as a constructor function.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type to modify
    @param constructor Native C constructor function to bind.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC void ejsBindConstructor(Ejs *ejs, EjsType *type, void *constructor);

/** 
    Bind a native C function to a method property
    @description Bind a native C function to an existing javascript method. Method functions are typically created
        by compiling a script file of native method definitions into a mod file. When loaded, this mod file will create
        the method properties. This routine will then bind the specified C function to the method property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Type containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fn Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsBindMethod(Ejs *ejs, EjsAny *obj, int slotNum, void *fn);

/** 
    Configure a type
    @description Called by loader to configure a native type based on the mod file information
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Type to configure
    @param up Reference to a module that will own the type. Set to null if not owned by any module.
    @param baseType Base type for this type.
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param numInstanceProp Number of instance properties.
    @param attributes Attribute mask to modify how the type is initialized.
    @return The configured type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, struct EjsModule *up, EjsType *baseType, 
    int numTypeProp, int numInstanceProp, int64 attributes);

/** 
    Create an Archetype.
    @description Archetypes are used when functions are used as constructors.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function object to use as a constructor
    @param prototype Prototype object. If set to null, a new prototype is created.
    @return A new type object
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsCreateArchetype(Ejs *ejs, struct EjsFunction *fun, EjsPot *prototype);

/** 
    Create a core type object
    @description Create a new type object 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name to give the type. This name is referenced by the type and must be persistent.
        This name is not used to define the type as a global property.
    @param size Size in bytes to reserve for the type
    @param slotNum Global slot number property index
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param manager MPR manager routine for garbage collection
    @param attributes Attribute mask to modify how the type is initialized.
    @return The created type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsCreateCoreType(Ejs *ejs, EjsName qname, int size, int slotNum, int numTypeProp, void *manager, 
        int64 attributes);

/** 
    Create a type prototype
    @description This creates a prototype object from which instances are crafted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Type object
    @param numProp Number of instance properties in the prototype
    @return The prototype object
    @ingroup EjsType
    @internal
 */
PUBLIC EjsObj *ejsCreatePrototype(Ejs *ejs, EjsType *type, int numProp);

/** 
    Create a new type object
    @description Create a new type object 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Qualified name to give the type. This name is referenced by the type and must be persistent.
        This name is not used to define the type as a global property.
    @param up Reference to a module that will own the type. Set to null if not owned by any module.
    @param baseType Base type for this type.
    @param prototype Prototype object instance properties of this type.
    @param size Size of instances. This is the size in bytes of an instance object.
    @param slotNum Unique type ID for core types. For non-core types, set to -1.
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param numInstanceProp Number of instance properties.
    @param manager MPR manager routine for garbage collection
    @param attributes Attribute mask to modify how the type is initialized.
    @return The created type
    @ingroup EjsType EjsType
 */
PUBLIC EjsType *ejsCreateType(Ejs *ejs, EjsName name, struct EjsModule *up, EjsType *baseType, EjsPot *prototype,
    int slotNum, int numTypeProp, int numInstanceProp, int size, void *manager, int64 attributes);

/** 
    Define a global function
    @description Define a global public function and bind it to the C native function. This is a simple one liner
        to define a public global function. The more typical paradigm to define functions is to create a script file
        of native method definitions and and compile it. This results in a mod file that can be loaded which will
        create the function/method definitions. Then use #ejsBindMethod to associate a C function with a property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param name Function name
    @param fn C function that implements the function
    @ingroup EjsType
 */
PUBLIC int ejsDefineGlobalFunction(Ejs *ejs, EjsString *name, EjsProc fn);

/** 
    Define an instance property
    @description Define an instance property on a type. This routine should not normally be called manually. Instance
        properties are best created by creating a script file of native property definitions and then loading the resultant
        mod file.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type in which to create the instance property
    @param slotNum Instance slot number in the type that will hold the property. Set to -1 to allocate the next available
        free slot.
    @param name Qualified name for the property including namespace and name.
    @param propType Type of the instance property.
    @param attributes Integer mask of access attributes.
    @param value Initial value of the instance property.
    @return The slot number used for the property.
    @ingroup EjsType
 */
PUBLIC int ejsDefineInstanceProperty(Ejs *ejs, EjsType *type, int slotNum, EjsName name, EjsType *propType, 
    int attributes, EjsAny *value);

/** 
    Get the prototype object for an object
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @return Prototype object for the given object
    @ingroup EjsType
 */
PUBLIC EjsPot *ejsGetPrototype(Ejs *ejs, EjsAny *obj);

/** 
    Get a type
    @description Get the type installed at the given slot number. All core-types are installed a specific global slots.
        When Ejscript is built, these slots are converted into C program defines of the form: ES_TYPE where TYPE is the 
        name of the type concerned. For example, you can get the String type object via:
        @pre
        ejsGetType(ejs, ES_String)
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param slotNum Slot number of the type to retrieve. Use ES_TYPE defines. 
    @return A type object if successful or zero if the type could not be found
    @ingroup EjsType
 */
PUBLIC EjsType  *ejsGetType(Ejs *ejs, int slotNum);

/** 
    Get a type given its name
    @description Types are stored in the global object. This routine looks in the global object for the type property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param qname Qualified name of the type.
    @return The type object
    @ingroup EjsType
 */
PUBLIC EjsType *ejsGetTypeByName(Ejs *ejs, EjsName qname);

/** 
    Finalize a core type
    @description This sets the configured state for the type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name of the type
    @return The finalized type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsFinalizeCoreType(Ejs *ejs, EjsName qname);

/** 
    Finalize a script type
    @description This finalizes the type and sets the configured state for the type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name of the type
    @param size Instance size of the type
    @param manager Manager function for garbage collection
    @param attributes Type attributes
    @return The configured type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsFinalizeScriptType(Ejs *ejs, EjsName qname, int size, void *manager, int64 attributes);

/** 
    Get the type name of an object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return The string type name of the object
    @ingroup EjsType
    @internal
 */
PUBLIC EjsString *ejsGetTypeName(struct Ejs *ejs, EjsAny *obj);

/** 
    TypeOf operator
    @description This finalizes the type and sets the configured state for the type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return String type name for the "typeOf" operator
    @ingroup EjsType
    @internal
 */
PUBLIC EjsString *ejsGetTypeOf(struct Ejs *ejs, EjsAny *obj);

/*
    WARNING: this macros assumes an "ejs" variable in scope. This is done because it is such a pervasive idiom, the
    assumption is worth the benefit.
 */
#if DOXYGEN
    /** 
        Test the type of an object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @param name Textual name of the type (Not void*). For example: ejsIs(ejs, obj, Number)
        @return True if the object is of the tested type.
        @ingroup EjsType
        @internal
     */
    extern bool ejsIs(Ejs *ejs, EjsAny *obj, void *name);

    /** 
        Test the object is not null and not undefined
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @return True if the object is of a defined type
        @ingroup EjsType
        @internal
     */
    extern bool ejsIsDefined(Ejs *ejs, EjsAny *obj);

    /** 
        Cast the object to the given type name
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @param name Textual name of the type (Not void*). For example: ejsCast(ejs, obj, String)
        @return Casted object.
        @ingroup EjsType
        @internal
     */
    extern EjsAny *ejsCast(Ejs *ejs, EjsAny *obj, void *name);
#else
    #define ejsIs(ejs, obj, name) ejsIsA(ejs, obj, EST(name))
    #define ejsIsDefined(ejs, obj) (obj != 0 && !ejsIs(ejs, obj, Null) && !ejsIs(ejs, obj, Void))
    #define ejsCast(ejs, obj, name) ejsCastType(ejs, obj, ESV(name))
#endif

/** 
    Test if an variable is an instance of a given type
    @description Perform an "is a" test. This tests if a variable is a direct instance or subclass of a given base type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param target Target object to test.
    @param type Type to compare with the target
    @return True if target is an instance of "type" or an instance of a subclass of "type".
    @ingroup EjsType
 */
PUBLIC bool ejsIsA(Ejs *ejs, EjsAny *target, EjsType *type);

/** 
    Test if a type is a derived type of a given base type.
    @description Test if a type subclasses a base type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param target Target type to test.
    @param baseType Base class to see if the target subclasses it.
    @return True if target is a "baseType" or a subclass of "baseType".
    @ingroup EjsType
 */
PUBLIC bool ejsIsTypeSubType(Ejs *ejs, EjsType *target, EjsType *baseType);

/*
    Internal
 */
#define VSPACE(space) space "-" ME_VNUM
#define ejsGetVType(ejs, space, name) ejsGetTypeByName(ejs, space "-" ME_VNUM, name)
PUBLIC void     ejsDefineTypeNamespaces(Ejs *ejs, EjsType *type);
PUBLIC int      ejsFixupType(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
PUBLIC int      ejsGetTypeSize(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitializeBlockHelpers(EjsHelpers *helpers);
PUBLIC int64    ejsSetTypeAttributes(EjsType *type, int size, MprManager manager, int64 attributes);
PUBLIC void     ejsSetTypeHelpers(EjsType *type, int64 attributes);
PUBLIC void     ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName qname);
PUBLIC void     ejsTypeNeedsFixup(Ejs *ejs, EjsType *type);

/******************************** Internal Prototypes *********************************/

PUBLIC int      ejsCreateBootstrapTypes(Ejs *ejs);
PUBLIC void     ejsInitBlockType(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitNullType(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitStringType(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitTypeType(Ejs *ejs, EjsType *type);

PUBLIC void     ejsCreateArrayType(Ejs *ejs);
PUBLIC void     ejsCreateBooleanType(Ejs *ejs);
PUBLIC void     ejsCreateConfigType(Ejs *ejs);
PUBLIC void     ejsCreateErrorType(Ejs *ejs);
PUBLIC void     ejsCreateFrameType(Ejs *ejs);
PUBLIC void     ejsCreateFunctionType(Ejs *ejs);
PUBLIC void     ejsCreateIteratorType(Ejs *ejs);
PUBLIC void     ejsCreateNamespaceType(Ejs *ejs);
PUBLIC void     ejsCreateNumberType(Ejs *ejs);
PUBLIC void     ejsCreateObjectType(Ejs *ejs);
PUBLIC void     ejsCreatePathType(Ejs *ejs);
PUBLIC void     ejsCreateRegExpType(Ejs *ejs);
PUBLIC void     ejsCreateVoidType(Ejs *ejs);
PUBLIC void     ejsCreateXMLType(Ejs *ejs);
PUBLIC void     ejsCreateXMLListType(Ejs *ejs);

/*  
    Native type configuration
 */
PUBLIC void     ejsConfigureAppType(Ejs *ejs);
PUBLIC void     ejsConfigureArrayType(Ejs *ejs);
PUBLIC void     ejsConfigureBlockType(Ejs *ejs);
PUBLIC void     ejsConfigureBooleanType(Ejs *ejs);
PUBLIC void     ejsConfigureByteArrayType(Ejs *ejs);
PUBLIC void     ejsConfigureCmdType(Ejs *ejs);
PUBLIC void     ejsConfigureDateType(Ejs *ejs);
PUBLIC void     ejsConfigureSqliteTypes(Ejs *ejs);
PUBLIC void     ejsConfigureDebugType(Ejs *ejs);
PUBLIC void     ejsConfigureErrorType(Ejs *ejs);
PUBLIC void     ejsConfigureEventType(Ejs *ejs);
PUBLIC void     ejsConfigureFrameType(Ejs *ejs);
PUBLIC void     ejsConfigureGCType(Ejs *ejs);
PUBLIC void     ejsConfigureGlobalBlock(Ejs *ejs);
PUBLIC void     ejsConfigureFileType(Ejs *ejs);
PUBLIC void     ejsConfigureFileSystemType(Ejs *ejs);
PUBLIC void     ejsConfigureFunctionType(Ejs *ejs);
PUBLIC void     ejsConfigureHttpType(Ejs *ejs);
PUBLIC void     ejsConfigureIteratorType(Ejs *ejs);
PUBLIC void     ejsConfigureJSONType(Ejs *ejs);
PUBLIC void     ejsConfigureLocalCacheType(Ejs *ejs);
PUBLIC void     ejsConfigureMprLogType(Ejs *ejs);
PUBLIC void     ejsConfigureNamespaceType(Ejs *ejs);
PUBLIC void     ejsConfigureMemoryType(Ejs *ejs);
PUBLIC void     ejsConfigureMathType(Ejs *ejs);
PUBLIC void     ejsConfigureNumberType(Ejs *ejs);
PUBLIC void     ejsConfigureNullType(Ejs *ejs);
PUBLIC void     ejsConfigureObjectType(Ejs *ejs);
PUBLIC void     ejsConfigurePathType(Ejs *ejs);
PUBLIC void     ejsConfigureReflectType(Ejs *ejs);
PUBLIC void     ejsConfigureRegExpType(Ejs *ejs);
PUBLIC void     ejsConfigureStringType(Ejs *ejs);
PUBLIC void     ejsConfigureSocketType(Ejs *ejs);
PUBLIC void     ejsConfigureSystemType(Ejs *ejs);
PUBLIC void     ejsConfigureTimerType(Ejs *ejs);
PUBLIC void     ejsConfigureTypes(Ejs *ejs);
PUBLIC void     ejsConfigureUriType(Ejs *ejs);
PUBLIC void     ejsConfigureVoidType(Ejs *ejs);
PUBLIC void     ejsConfigureWorkerType(Ejs *ejs);
PUBLIC void     ejsConfigureXMLType(Ejs *ejs);
PUBLIC void     ejsConfigureXMLListType(Ejs *ejs);
PUBLIC void     ejsConfigureWebSocketType(Ejs *ejs);

PUBLIC void     ejsCreateCoreNamespaces(Ejs *ejs);
PUBLIC int      ejsCopyCoreTypes(Ejs *ejs);
PUBLIC int      ejsDefineCoreTypes(Ejs *ejs);
PUBLIC int      ejsDefineErrorTypes(Ejs *ejs);
PUBLIC void     ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType);
PUBLIC void     ejsSetSqliteMemCtx(MprThreadLocal *tls);
PUBLIC void     ejsSetSqliteTls(MprThreadLocal *tls);
PUBLIC void     ejsDefineConfigProperties(Ejs *ejs);

#if ME_COM_SQLITE
    PUBLIC int   ejs_db_sqlite_Init(Ejs *ejs, MprModule *mp);
#endif
#if ME_COM_ZLIB
    PUBLIC int   ejs_zlib_Init(Ejs *ejs, MprModule *mp);
#endif
PUBLIC int       ejs_web_Init(Ejs *ejs, MprModule *mp);

/* 
    Move some ejsWeb.h declarations here so handlers can just include ejs.h whether they are using the
    all-in-one ejs.h or the pure ejs.h
 */
PUBLIC HttpStage *ejsAddWebHandler(Http *http, MprModule *module);
PUBLIC int ejsHostHttpServer(HttpConn *conn);

/******************************************** VM **************************************************/
/**
    VM Evaluation state. 
    The VM Stacks grow forward in memory. A push is done by incrementing first, then storing. ie. *++top = value
    A pop is done by extraction then decrement. ie. value = *top--
    @ingroup Ejs
    @stability Internal
 */
typedef struct EjsState {
    struct EjsFrame     *fp;                /**< Current Frame function pointer */
    struct EjsBlock     *bp;                /**< Current block pointer */
    EjsObj              **stack;            /**< Top of stack (points to the last element pushed) */
    EjsObj              **stackBase;        /**< Pointer to start of stack mem */
    struct EjsState     *prev;              /**< Previous state */
    struct EjsNamespace *internal;          /**< Current internal namespace */
    ssize               stackSize;          /**< Stack size */
    uint                paused: 1;          /**< Garbage collection paused */
    EjsObj              *t1;                /**< Temp one for GC */
} EjsState;


/**
    Lookup State.
    @description Location information returned when looking up properties.
    @ingroup Ejs
    @stability Internal
 */
typedef struct EjsLookup {
    int             slotNum;                /**< Final slot in obj containing the variable reference */
    uint            nthBase;                /**< Property on Nth super type -- count from the object */
    uint            nthBlock;               /**< Property on Nth block in the scope chain -- count from the end */
    EjsType         *type;                  /**< Type containing property (if on a prototype obj) */
    uint            instanceProperty;       /**< Property is an instance property */
    uint            ownerIsType;            /**< Original object owning the property is a type */
    uint            useThis;                /**< Property accessible via "this." */
    EjsAny          *obj;                   /**< Final object / Type containing the variable */
    EjsAny          *originalObj;           /**< Original object used for the search */
    EjsAny          *ref;                   /**< Actual property reference */
    struct EjsTrait *trait;                 /**< Property trait describing the property */
    struct EjsName  name;                   /**< Name and namespace used to find the property */
    int             bind;                   /**< Whether to bind to this lookup */
} EjsLookup;


/**
    Interned string hash shared over all interpreters
    @ingroup Ejs
    @stability Internal
 */
typedef struct EjsIntern {
    struct EjsString    *buckets;               /**< Hash buckets and references to link chains of strings (unicode) */
    int                 size;                   /**< Size of hash */
    int                 count;                  /**< Count of entries */
    uint64              reuse;                  /**< Reuse counter */
    uint64              accesses;               /**< NUmber of accesses to string */
    MprMutex            *mutex;
} EjsIntern;

/**
    Ejscript Service structure
    @description The Ejscript service manages the overall language runtime. It 
        is the factory that creates interpreter instances via #ejsCreateVM.
    @ingroup EjsService
    @stability Internal
 */
typedef struct EjsService {
    EjsObj          *(*loadScriptLiteral)(Ejs *ejs, EjsString *script, cchar *cache);
    EjsObj          *(*loadScriptFile)(Ejs *ejs, cchar *path, cchar *cache);
    MprList         *vmlist;                /**< List of all VM interpreters */
    MprHash         *nativeModules;         /**< Set of loaded native modules */
    Http            *http;                  /**< Http service */
    uint            dontExit: 1;            /**< Prevent App.exit() from exiting */
    uint            logging: 1;             /**< Using --log */
    uint            immutableInitialized: 1;/**< Immutable types are initialized */
    uint            seqno;                  /**< Interp sequence numbers */
    EjsIntern       *intern;                /**< Interned Unicode string hash - shared over all interps */
    EjsPot          *immutable;             /**< Immutable types and special values*/
    EjsHelpers      objHelpers;             /**< Default EjsObj helpers */
    EjsHelpers      potHelpers;             /**< Default EjsPot helpers */
    EjsHelpers      blockHelpers;           /**< Default EjsBlock helpers */
    MprMutex        *mutex;                 /**< Multithread locking */
    MprSpin         *dtoaSpin[2];           /**< Dtoa thread synchronization */
} EjsService;

/*
   Internal
 */
PUBLIC EjsIntern *ejsCreateIntern(EjsService *sp);
PUBLIC int ejsInitCompiler(EjsService *sp);
PUBLIC void ejsAttention(Ejs *ejs);
PUBLIC void ejsClearAttention(Ejs *ejs);

/*********************************** Prototypes *******************************/
/**
    Create an ejs virtual machine 
    @description Create a virtual machine interpreter object to evalute Ejscript programs. Ejscript supports multiple 
        interpreters. 
    @param argc Count of command line argumements in argv
    @param argv Command line arguments
    @param flags Optional flags to modify the interpreter behavior. Valid flags are:
        @li    EJS_FLAG_COMPILER       - Interpreter will compile code from source
        @li    EJS_FLAG_NO_EXE         - Don't execute any code. Just compile.
        @li    EJS_FLAG_DOC            - Load documentation from modules
        @li    EJS_FLAG_NOEXIT         - App should service events and not exit unless explicitly instructed
    @return A new interpreter
    @ingroup Ejs
 */
PUBLIC Ejs *ejsCreateVM(int argc, cchar **argv, int flags);

/**
    Clone an ejs virtual machine 
    @description Create a virtual machine interpreter boy cloning an existing interpreter. Cloning is a fast way
        to create a new interpreter. This saves memory and speeds initialization.
    @param ejs Base VM upon which to base the new VM.
    @return A new interpreter
    @ingroup Ejs
 */
PUBLIC Ejs *ejsCloneVM(Ejs *ejs);

/**
    Set the MPR dispatcher to use for an interpreter.
    @description Interpreters serialize event activity within a dispatcher.
    @ingroup Ejs
 */
PUBLIC void ejsSetDispatcher(Ejs *ejs, MprDispatcher *dispatcher);

/**
    Destroy an interpreter
    @param ejs Interpreter to destroy
 */
PUBLIC void ejsDestroyVM(Ejs *ejs);

//  MOB
PUBLIC void ejsDestroy(Ejs *ejs);

/**
    Evaluate a file
    @description Evaluate a file containing an Ejscript. This requires linking with the Ejscript compiler library (libec). 
    @param path Filename of the script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsEvalFile(cchar *path);

/*
    Flags for LoadScript and compiling
 */
#define EC_FLAGS_BIND            0x1        /**< ejsLoad flags to bind global references and type/object properties */
#define EC_FLAGS_DEBUG           0x2        /**< ejsLoad flags to generate symbolic debugging information */
#define EC_FLAGS_MERGE           0x8        /**< ejsLoad flags to merge all output onto one output file */
#define EC_FLAGS_NO_OUT          0x10       /**< ejsLoad flags discard all output */
#define EC_FLAGS_PARSE_ONLY      0x20       /**< ejsLoad flags to only parse source. Don't generate code */
#define EC_FLAGS_THROW           0x40       /**< ejsLoad flags to throw errors when compiling. Used for eval() */
#define EC_FLAGS_VISIBLE         0x80       /**< ejsLoad flags to make global vars visible to all */
#define EC_FLAGS_DOC             0x100      /**< ejsLoad flags to parse inline doc */

/** 
    Load a script from a file
    @description This will read a script from a file, compile it and run. If the cache path argument is 
        provided, the compiled module will be saved to this path.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param script Script pathname.
    @param cache Cache pathname to save compiled module.
    @param flags Compilation control flags. Select from:
    <ul>
        <li> EC_FLAGS_BIND - Bind global references and type/object properties</li>
        <li> EC_FLAGS_DEBUG - Generate symbolic debugging information</li>
        <li> EC_FLAGS_MERGE - Merge all output onto one output file</li>
        <li> EC_FLAGS_NO_OUT - Discard all output</li>
        <li> EC_FLAGS_PARSE_ONLY - Only parse source. Don't generate code</li>
        <li> EC_FLAGS_THROW - Throw errors when compiling. Used for eval()</li>
        <li> EC_FLAGS_VISIBLE - Make global vars visible to all</li>
    </ul>
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLoadScriptFile(Ejs *ejs, cchar *script, cchar *cache, int flags);

//  TODO - rename ejsLoadScriptString
/** 
    Load a script from a string
    @description This will compile the script string and then run it. If the cache path argument is provided, 
    the compiled module will be saved to this path.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param script Script string.
    @param cache Cache pathname to save compiled module.
    @param flags Compilation control flags. Select from:
    <ul>
        <li> EC_FLAGS_BIND - Bind global references and type/object properties</li>
        <li> EC_FLAGS_DEBUG - Generate symbolic debugging information</li>
        <li> EC_FLAGS_MERGE - Merge all output onto one output file</li>
        <li> EC_FLAGS_NO_OUT - Discard all output</li>
        <li> EC_FLAGS_PARSE_ONLY - Only parse source. Don't generate code</li>
        <li> EC_FLAGS_THROW - Throw errors when compiling. Used for eval()</li>
        <li> EC_FLAGS_VISIBLE - Make global vars visible to all</li>
    </ul>
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLoadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache, int flags);

/**
    Evaluate a module
    @description Evaluate a module containing compiled Ejscript.
    @param path Filename of the module to evaluate.
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsEvalModule(cchar *path);

/**
    Evaluate a script
    @description Evaluate a script. This requires linking with the Ejscript compiler library (libec). 
    @param script Script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsEvalScript(cchar *script);

/**
    Instruct the interpreter to exit.
    @description This will instruct the interpreter to cease interpreting any further script code.
    @param ejs Interpeter object returned from #ejsCreateVM
    @param status Reserved and ignored
    @ingroup Ejs
 */
PUBLIC void ejsExit(Ejs *ejs, int status);

/**
    Get the hosting handle
    @description The interpreter can store a hosting handle. This is typically a web server object if hosted inside
        a web server
    @param ejs Interpeter object returned from #ejsCreateVM
    @return Hosting handle
    @ingroup Ejs
 */
PUBLIC void *ejsGetHandle(Ejs *ejs);

//  TODO - variable is the wrong name. ejsGetPropByName?
/** 
    Get a variable by name
    @description This looks for a property name in an object, its prototype or base classes.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to search
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return The variable 
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsGetVarByName(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);

/** 
    Lookup a variable using the current scope
    @description This looks for a property name in the current lexical scope.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLookupScope(Ejs *ejs, EjsName name, EjsLookup *lookup);

/** 
    Lookup a variable
    @description This looks for a property name in an object, its prototype or base classes. If name.space is null, 
        the variable is searched using the set of currently open namespaces. 
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to search
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLookupVar(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);

/** 
    Lookup a variable in an object (only)
    @description This looks for a property name in an object, its prototype or base classes. If name.space is null,
    the variable is searched using the set of currently open namespaces.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to search
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLookupVarWithNamespaces(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);

/**
    Run a script
    @description Run a script that has previously ben compiled by ecCompile
    @param ejs Interpeter object returned from #ejsCreateVM
    @return Zero if successful, otherwise a non-zero Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsRun(Ejs *ejs);

/** 
    Run a program.
    @description Lookup the className and run the designated method. If methodName is null, then "main" is run.
        The method should be a static method of the class.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param className Class name to search for methodName.
    @param methodName Method to run. If set to NULL, then search for "main".
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsRunProgram(Ejs *ejs, cchar *className, cchar *methodName);

/**
    Throw an exception
    @description Throw an exception object 
    @param ejs Interpeter object returned from #ejsCreateVM
    @param error Exception argument object.
    @return The exception argument for chaining.
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsThrowException(Ejs *ejs, EjsAny *error);

/** 
    Clear an exception
    @param ejs Interpreter instance returned from #ejsCreateVM
    @ingroup Ejs
    @internal
 */
PUBLIC void ejsClearException(Ejs *ejs);

/**
    Report an error message using the MprLog error channel
    @description This will emit an error message of the format:
        @li program:line:errorCode:SEVERITY: message
    @param ejs Interpeter object returned from #ejsCreateVM
    @param fmt Is an alternate printf style format to emit if the interpreter has no valid error message.
    @param ... Arguments for fmt
    @ingroup Ejs
 */
PUBLIC void ejsReportError(Ejs *ejs, char *fmt, ...);

/** 
    Enter a message into the log file
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param fmt Message format string
    @ingroup Ejs
 */
PUBLIC void ejsLog(Ejs *ejs, cchar *fmt, ...);

/*
    Internal
 */
PUBLIC void ejsApplyObjHelpers(EjsService *sp, EjsType *type);
PUBLIC void ejsApplyPotHelpers(EjsService *sp, EjsType *type);
PUBLIC void ejsApplyBlockHelpers(EjsService *sp, EjsType *type);
PUBLIC EjsAny *ejsCastOperands(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs);
PUBLIC int ejsCheckModuleLoaded(Ejs *ejs, cchar *name);
PUBLIC EjsAny *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs);
PUBLIC void ejsClearExiting(Ejs *ejs);
PUBLIC void ejsCreateObjHelpers(Ejs *ejs);
PUBLIC void ejsLockService();
PUBLIC void ejsLockVm(Ejs *ejs);
PUBLIC int  ejsParseModuleVersion(cchar *name);
PUBLIC int  ejsRedirectLogging(cchar *logSpec);
PUBLIC void ejsRedirectLoggingToFile(MprFile *file, int level);
PUBLIC void ejsSetHandle(Ejs *ejs, void *handle);
PUBLIC void ejsShowCurrentScope(Ejs *ejs);
PUBLIC void ejsShowStack(Ejs *ejs, EjsFunction *fp);
PUBLIC void ejsShowBlockScope(Ejs *ejs, EjsBlock *block);
PUBLIC void ejsUnlockService();
PUBLIC void ejsUnlockVm(Ejs *ejs);

/******************************************* Module **************************************************/
/*
    A module file may contain multiple logical modules.

    Module File Format and Layout:

    (N) Numbers are 1-3 little-endian encoded bytes using the 0x80 as the continuation character
    (S) Strings are pointers into the constant pool encoded as number offsets. Strings are UTF-8.
    (T) Types are encoded and ored with the type encoding masks below. Types are either: untyped, 
        unresolved or primitive (builtin). The relevant mask is ored into the lower 2 bits. Slot numbers and
        name tokens are shifted up 2 bits. Zero means untyped.

    ModuleHeader
        short       magic
        int         fileVersion
        int         version
        int         flags

    Module
        byte        section
        string      name
        number      version
        number      checksum
        number      constantPoolLength
        block       constantPool

    Dependencies
        byte        section
        string      moduleName
        number      minVersion
        number      maxVersion
        number      checksum
        byte        flags

    Type
        byte        section
        string      typeName
        string      namespace
        number      attributes
        number      slot
        type        baseType
        number      numStaticProperties
        number      numInstanceProperties
        number      numInterfaces
        type        Interfaces ...
        ...

    Property
        byte        section
        string      name
        string      namespace
        number      attributes
        number      slot
        type        property type

    Function
        byte        section
        string      name
        string      namespace
        number      nextSlotForSetter
        number      attributes
        byte        languageMode
        type        returnType
        number      slot
        number      argCount
        number      defaultArgCount
        number      localCount
        number      exceptionCount
        number      codeLength
        block       code        

    Exception
        byte        section
        byte        flags
        number      tryStartOffset
        number      tryEndOffset
        number      handlerStartOffset
        number      handlerEndOffset
        number      numOpenBlocks
        type        catchType

    Debug
        byte        section
        number      countOfLines
        string      fileName
        number      startLine
        number      addressOffset      
        ...

    Block
        byte        section
        string      name
        number      slot
        number      propCount

    Documentation
        byte        section
        string      text
 */

/*
    Type encoding masks
 */
#define EJS_ENCODE_GLOBAL_NOREF         0x0
#define EJS_ENCODE_GLOBAL_NAME          0x1
#define EJS_ENCODE_GLOBAL_SLOT          0x2
#define EJS_ENCODE_GLOBAL_MASK          0x3

/*
    Fixup kinds
 */
#define EJS_FIXUP_BASE_TYPE             1
#define EJS_FIXUP_INTERFACE_TYPE        2
#define EJS_FIXUP_RETURN_TYPE           3
#define EJS_FIXUP_TYPE_PROPERTY         4
#define EJS_FIXUP_INSTANCE_PROPERTY     5
#define EJS_FIXUP_LOCAL                 6
#define EJS_FIXUP_EXCEPTION             7

/*
    Number encoding uses one bit per byte plus a sign bit in the first byte
 */ 
#define EJS_ENCODE_MAX_WORD             0x07FFFFFF

/*
    Type fixup when loading a type
    @stability Internal
 */
typedef struct EjsTypeFixup
{
    int              kind;                  /* Kind of fixup */
    int              slotNum;               /* Slot of target */
    EjsObj           *target;               /* Target to fixup */
    EjsName          typeName;              /* Type name */
    int              typeSlotNum;           /* Type slot number */
} EjsTypeFixup;


/*
    State while loading modules
    @stability Internal
 */
typedef struct EjsLoadState {
    MprList         *typeFixups;            /**< Loaded types to fixup */
    int             firstModule;            /**< First module in ejs->modules for this load */
    int             flags;                  /**< Module load flags */
} EjsLoadState;

/*
    Loader callback
    @param ejs Ejs reference returned from #ejsCreateVM
    @param kind Kind of load record
 */
typedef void (*EjsLoaderCallback)(struct Ejs *ejs, int kind, ...);

/*
    Module file format version
 */
#define EJS_MODULE_VERSION      3
#define EJS_VERSION_FACTOR      1000
#define EJS_MODULE_MAGIC        0xC7DA

#if DOXYGEN
    /**
        Make an integer  version number
        @param maj Major version number
        @param min Minor version number
        @param patch Patch version number
        @return An integer version number combining major, minor and patch version numbers.
     */
    extern int EJS_MAKE_VERSION(int maj, int min, int patch);

#else
    #define EJS_MAKE_VERSION(maj, min, patch) (((((maj) * EJS_VERSION_FACTOR) + (min)) * EJS_VERSION_FACTOR) + (patch))
    #define EJS_COMPAT_VERSION(v1, v2) ((v1 / EJS_VERSION_FACTOR) == (v2 / EJS_VERSION_FACTOR))
    #define EJS_MAKE_COMPAT_VERSION(version) (version / EJS_VERSION_FACTOR * EJS_VERSION_FACTOR)
    #define EJS_MAJOR(version)      (((version / EJS_VERSION_FACTOR) / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
    #define EJS_MINOR(version)      ((version / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
    #define EJS_PATCH(version)      (version % EJS_VERSION_FACTOR)
    #define EJS_MAX_VERSION         EJS_MAKE_VERSION(EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1)
#if UNUSED
    #define EJS_VERSION             EJS_MAKE_VERSION(ME_MAJOR_VERSION, ME_MINOR_VERSION, ME_PATCH_VERSION)
#endif
#endif

#ifndef EJS_VERSION
    #define EJS_VERSION ME_VERSION
#endif

/*
    Section types
 */
#define EJS_SECT_MODULE         1           /* Module section */
#define EJS_SECT_MODULE_END     2           /* End of a module */
#define EJS_SECT_DEBUG          3           /* Module dependency */
#define EJS_SECT_DEPENDENCY     4           /* Module dependency */
#define EJS_SECT_CLASS          5           /* Class definition */
#define EJS_SECT_CLASS_END      6           /* End of class definition */
#define EJS_SECT_FUNCTION       7           /* Function */
#define EJS_SECT_FUNCTION_END   8           /* End of function definition */
#define EJS_SECT_BLOCK          9           /* Nested block */
#define EJS_SECT_BLOCK_END      10          /* End of Nested block */
#define EJS_SECT_PROPERTY       11          /* Property (variable) definition */
#define EJS_SECT_EXCEPTION      12          /* Exception definition */
#define EJS_SECT_DOC            13          /* Documentation for an element */
#define EJS_SECT_MAX            14

/*
    Psudo section types for loader callback
 */
#define EJS_SECT_START          (EJS_SECT_MAX + 1)
#define EJS_SECT_END            (EJS_SECT_MAX + 2)

/*
    Align headers on a 4 byte boundary
 */
#define EJS_HDR_ALIGN           4

/*
    File format is little-endian. All headers are aligned on word boundaries.
    @stability Internal
 */
typedef struct EjsModuleHdr {
    int32       magic;                      /* Magic number for Ejscript modules */
    int32       fileVersion;                /* Module file format version */
    int32       flags;                      /* Module flags */
} EjsModuleHdr;


/**
    Module control structure
    @defgroup EjsModule EjsModule
    @see ejsLoadModule ejsLoadModules ejsSearchForModule ejsCreateSearchPath ejsSetSearchPath
    @stability Internal
 */
typedef struct EjsModule {
    EjsString       *name;                  /**< Name of this module - basename of the filename without .mod extension */
    //  TODO - document the version format
    EjsString       *vname;                 /**< Versioned name - name with optional version suffix */
    MprMutex        *mutex;                 /**< Multithread locking */
    int             version;                /**< Made with EJS_MAKE_VERSION */
    int             minVersion;             /**< Minimum version when used as a dependency */
    int             maxVersion;             /**< Maximum version when used as a dependency */
    int             checksum;               /**< Checksum of slots and names */

    EjsConstants    *constants;             /**< Constant pool */
    EjsFunction     *initializer;           /**< Initializer method */

    //  TODO - should have isDefault bit
    uint            compiling       : 1;    /**< Module currently being compiled from source */
    uint            configured      : 1;    /**< Module types have been configured with native code */
    uint            loaded          : 1;    /**< Module has been loaded from an external file */
    uint            nativeLoaded    : 1;    /**< Backing shared library loaded */
    uint            hasError        : 1;    /**< Module has a loader error */
    uint            hasInitializer  : 1;    /**< Has initializer function */
    uint            hasNative       : 1;    /**< Has native property definitions */
    uint            initialized     : 1;    /**< Initializer has run */
    uint            visited         : 1;    /**< Module has been traversed */
    int             flags;                  /**< Loading flags */

    /*
        Module loading and residuals 
     */
    EjsLoadState    *loadState;             /**< State while loading */
    MprList         *dependencies;          /**< Module file dependencies. List of EjsModules */
    MprFile         *file;                  /**< File handle for loading and code generation */
    MprList         *current;               /**< Current stack of open objects */
    EjsFunction     *currentMethod;         /**< Current method being loaded */
    EjsBlock        *scope;                 /**< Lexical scope chain */
    EjsString       *doc;                   /**< Current doc string */
    char            *path;                  /**< Module file path name */
    int             firstGlobal;            /**< First global property */
    int             lastGlobal;             /**< Last global property + 1*/

    /*
        Used during code generation
     */
    struct EcCodeGen *code;                 /**< Code generation buffer */
    MprList         *globalProperties;      /**< List of global properties */

} EjsModule;


/*
    Internal
 */
PUBLIC int ejsCreateConstants(Ejs *ejs, EjsModule *mp, int count, ssize size, char *pool);
PUBLIC int ejsGrowConstants(Ejs *ejs, EjsModule *mp, ssize size);
PUBLIC int ejsAddConstant(Ejs *ejs, EjsModule *mp, cchar *str);

/**
    Native module initialization callback
    @param ejs Ejs reference returned from #ejsCreateVM
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsModule
 */
typedef int (*EjsNativeCallback)(Ejs *ejs);

typedef struct EjsNativeModule {
    EjsNativeCallback callback;             /* Callback to configure module native types and properties */
    char            *name;                  /* Module name */
    int             checksum;               /* Checksum expected by native code */
    int             flags;                  /* Configuration flags */
} EjsNativeModule;

/*
    Documentation string information
    Element documentation string. The loader will create if required.
    @ingroup EjsModule
    @stability Internal
 */
typedef struct EjsDoc {
    EjsString   *docString;                         /* Original doc string */
    wchar       *brief;                             /* Element brief */
    wchar       *description;                       /* Element description */
    wchar       *example;                           /* Element example */
    wchar       *requires;                          /* Element requires */
    wchar       *returns;                           /* Element returns */
    wchar       *stability;                         /* prototype, evolving, stable, mature, deprecated */
    wchar       *spec;                              /* Where specified */
    struct EjsDoc *duplicate;                       /* From @duplicate directive */
    MprList     *defaults;                          /* Parameter default values */
    MprList     *params;                            /* Function parameters */
    MprList     *options;                           /* Option parameter values */
    MprList     *events;                            /* Option parameter values */
    MprList     *see;                               /* Element see also */
    MprList     *throws;                            /* Element throws */
    EjsTrait    *trait;                             /* Back pointer to trait */
    int         deprecated;                         /* Hide doc if true */
    int         hide;                               /* Hide doc if true */
    int         cracked;                            /* Doc has been cracked and tokenized */
} EjsDoc;

/*
    Loader flags
 */
#define EJS_LOADER_STRICT     0x1                   /**< Load module code in strict mode */
#define EJS_LOADER_NO_INIT    0x2                   /**< Load module code in strict mode */
#define EJS_LOADER_ETERNAL    0x4                   /**< Make all loaded types eternal */
#define EJS_LOADER_BUILTIN    0x8                   /**< Loading builtins */
#define EJS_LOADER_DEP        0x10                  /**< Loading a dependency */
#define EJS_LOADER_RELOAD     0x20                  /**< Force a reload if already loaded */

/**
    Create a search path array. This can be used in ejsCreate.
    @description Create and array of search paths.
    @param ejs Ejs interpreter
    @param searchPath Search path string. This is a colon (or semicolon on Windows) separated string of directories.
    @return An array of search paths
    @ingroup EjsModule
 */
struct EjsArray *ejsCreateSearchPath(Ejs *ejs, cchar *searchPath);

/**
    Load a module
    @description This will emit an error message of the format:
        @li program:line:errorCode:SEVERITY: message
    @param ejs Interpeter object returned from #ejsCreateVM
    @param name Module path name
    @param maxVer Maximum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @param minVer Minimum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @param flags Module loading flags. Select from: EJS_LOADER_STRICT, EJS_LOADER_NO_INIT, EJS_LOADER_ETERNAL,
        EJS_LOADER_BUILTIN, EJS_LOADER_DEP, EJS_LOADER_RELOAD
    @return A postitive slot number or a negative MPR error code.
    @ingroup EjsModule
 */
PUBLIC int ejsLoadModule(Ejs *ejs, EjsString *name, int minVer, int maxVer, int flags);

/**
    Load modules into an interpreter
    @description Initialize an interpreter by loading modules. A list of modules to load can be provided via the "require"
        argument. If the "require" argument is set to null, then the default modules will be loaded. If "require" is 
        set to a list of module names, these will be loaded. If set to an empty list, then no modules will be loaded and
        the interpreter will be marked as an "empty" interpreter.
    @param ejs Interpreter to modify
    @param search Module search path to use. Set to NULL for the default search path.
    @param require Optional list of required modules to load. If NULL, the following modules will be loaded:
        ejs, ejs.io, ejs.events, ejs.xml, ejs.sys and ejs.unix.
    @return Zero if successful, otherwise return a negative MPR error code.
    @ingroup EjsModule
 */
PUBLIC int ejsLoadModules(Ejs *ejs, cchar *search, MprList *require);

/**
    Search for a module in the module search path.
    @param ejs Interpeter object returned from #ejsCreateVM
    @param name Module name
    @param minVer Minimum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @param maxVer Maximum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @return Path name to the module
    @ingroup EjsModule
 */
PUBLIC char *ejsSearchForModule(Ejs *ejs, cchar *name, int minVer, int maxVer);

/**
    Set the module search path
    @description Set the ejs module search path. The search path is by default set to the value of the EJSPATH
        environment directory. Ejsript will search for modules by name. The search strategy is:
        Given a name "a.b.c", scan for:
        @li File named a.b.c
        @li File named a/b/c
        @li File named a.b.c in EJSPATH
        @li File named a/b/c in EJSPATH
        @li File named c in EJSPATH

    Ejs will search for files with no extension and also search for modules with a ".mod" extension. If there is
    a shared library of the same name with a shared library extension (.so, .dll, .dylib) and the module requires 
    native code, then the shared library will also be loaded.
    @param ejs Ejs interpreter
    @param search Array of search paths
    @ingroup EjsModule
 */
PUBLIC void ejsSetSearchPath(Ejs *ejs, struct EjsArray *search);

/*
    Internal
 */
PUBLIC int ejsAddModule(Ejs *ejs, EjsModule *up);
PUBLIC int ejsAddNativeModule(Ejs *ejs, cchar *name, EjsNativeCallback callback, int checksum, int flags);
PUBLIC EjsModule *ejsCloneModule(Ejs *ejs, EjsModule *mp);
PUBLIC EjsDoc *ejsCreateDoc(Ejs *ejs, cchar *tag, void *vp, int slotNum, EjsString *docString);
PUBLIC EjsModule *ejsCreateModule(Ejs *ejs, EjsString *name, int version, EjsConstants *constants);
PUBLIC double ejsDecodeDouble(Ejs *ejs, uchar **pp);
PUBLIC int ejsDecodeInt32(Ejs *ejs, uchar **pp);
PUBLIC int64 ejsDecodeNum(Ejs *ejs, uchar **pp);
PUBLIC int ejsEncodeByteAtPos(Ejs *ejs, uchar *pos, int value);
PUBLIC int ejsEncodeDouble(Ejs *ejs, uchar *pos, double number);
PUBLIC int ejsEncodeInt32(Ejs *ejs, uchar *pos, int number);
PUBLIC int ejsEncodeNum(Ejs *ejs, uchar *pos, int64 number);
PUBLIC int ejsEncodeInt32AtPos(Ejs *ejs, uchar *pos, int value);
PUBLIC char *ejsGetDocKey(Ejs *ejs, EjsBlock *block, int slotNum, char *buf, int bufsize);
PUBLIC EjsModule *ejsLookupModule(Ejs *ejs, EjsString *name, int minVersion, int maxVersion);
PUBLIC EjsNativeModule *ejsLookupNativeModule(Ejs *ejs, cchar *name);
PUBLIC void ejsModuleReadBlock(Ejs *ejs, EjsModule *module, char *buf, int len);
PUBLIC int ejsModuleReadByte(Ejs *ejs, EjsModule *module);
PUBLIC EjsString *ejsModuleReadConst(Ejs *ejs, EjsModule *module);
PUBLIC int ejsModuleReadInt(Ejs *ejs, EjsModule *module);
PUBLIC int ejsModuleReadInt32(Ejs *ejs, EjsModule *module);
PUBLIC EjsName ejsModuleReadName(Ejs *ejs, EjsModule *module);
PUBLIC int64 ejsModuleReadNum(Ejs *ejs, EjsModule *module);
PUBLIC char *ejsModuleReadMulti(Ejs *ejs, EjsModule *mp);
PUBLIC wchar *ejsModuleReadMultiAsWide(Ejs *ejs, EjsModule *mp);
PUBLIC int ejsModuleReadType(Ejs *ejs, EjsModule *module, EjsType **typeRef, EjsTypeFixup **fixup, EjsName *typeName, 
        int *slotNum);
PUBLIC void ejsRemoveModule(Ejs *ejs, EjsModule *up);
PUBLIC void ejsRemoveModuleFromAll(EjsModule *up);
PUBLIC double ejsSwapDouble(Ejs *ejs, double a);
PUBLIC int ejsSwapInt32(Ejs *ejs, int word);
PUBLIC int64 ejsSwapInt64(Ejs *ejs, int64 word);

#ifdef __cplusplus
}
#endif

/*
    Allow user overrides
 */


#endif /* _h_EJS_CORE */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ejs.web/ejsWeb.h"
 */
/************************************************************************/

/**
    ejsWeb.h -- Header for the Ejscript Web Framework
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_WEB_h
#define _h_EJS_WEB_h 1

#include    "http.h"


/*********************************** Defines **********************************/

#define EJS_SESSION "-ejs-session-"             /**< Default session cookie string */

#ifdef  __cplusplus
extern "C" {
#endif

/*********************************** Types ************************************/

#ifndef EJS_HTTPSERVER_NAME
#define EJS_HTTPSERVER_NAME "ejs-http"
#endif

/** 
    HttpServer Class
    @description
        HttpServer objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @stability Prototype
    @defgroup EjsHttpServer EjsHttpServer
    @see EjsHttpServer ejsCloneHttpServer
 */
typedef struct EjsHttpServer {
    EjsPot          pot;                        /**< Extends Object */
    Ejs             *ejs;                       /**< Ejscript interpreter handle */
    HttpEndpoint    *endpoint;                  /**< Http endpoint object */
    struct MprSsl   *ssl;                       /**< SSL configuration */
    HttpTrace       trace[2];                   /**< Default tracing for requests */
    cchar           *connector;                 /**< Pipeline connector */
    char            *keyFile;                   /**< SSL key file */
    char            *certFile;                  /**< SSL certificate file */
    char            *protocols;                 /**< SSL protocols */
    char            *ciphers;                   /**< SSL ciphers */
    char            *ip;                        /**< Listening address */
    char            *name;                      /**< Server name */
    int             async;                      /**< Async mode */
    int             port;                       /**< Listening port */
    int             hosted;                     /**< Server being hosted inside a web server */
    struct EjsHttpServer *cloned;               /**< Server that was cloned */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsObj          *limits;                    /**< Limits object */
    EjsArray        *incomingStages;            /**< Incoming Http pipeline stages */
    EjsArray        *outgoingStages;            /**< Outgoing Http pipeline stages */
} EjsHttpServer;

/** 
    Clone a http server
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server HttpServer object
    @param deep Ignored
    @returns A new server object.
    @ingroup EjsHttpServer
*/
extern EjsHttpServer *ejsCloneHttpServer(Ejs *ejs, EjsHttpServer *server, bool deep);

/** 
    Request Class
    @description
        Request objects represent a single Http request.
    @stability Prototype
    @defgroup EjsRequest EjsRequest
    @see EjsRequest ejsCloneRequest ejsCreateRequest 
 */
typedef struct EjsRequest {
    EjsPot          pot;                /**< Base object storage */
    EjsObj          *absHome;           /**< Absolute URI to the home of the application from this request */
    struct EjsRequest *cloned;          /**< Request that was cloned */
    EjsObj          *config;            /**< Request config environment */
    HttpConn        *conn;              /**< Underlying Http connection object */
    EjsObj          *cookies;           /**< Cached cookies */
    EjsPath         *dir;               /**< Home directory containing the application */
    EjsObj          *emitter;           /**< Event emitter */
    EjsObj          *env;               /**< Request.env */
    EjsPath         *filename;          /**< Physical resource filename */
    EjsObj          *files;             /**< Files object */
    EjsString       *formData;          /**< Form data as a stable, sorted string */
    EjsObj          *headers;           /**< Headers object */
    EjsUri          *home;              /**< Relative URI to the home of the application from this request */
    EjsString       *host;              /**< Host property */
    EjsObj          *limits;            /**< Limits object */
    EjsObj          *log;               /**< Log object */
    EjsObj          *originalUri;       /**< Saved original URI */
    EjsObj          *params;            /**< Form variables + routing variables */
    EjsString       *pathInfo;          /**< PathInfo property */
    EjsNumber       *port;              /**< Port property */
    EjsString       *query;             /**< Query property */
    EjsString       *reference;         /**< Reference property */
    EjsObj          *responseHeaders;   /**< Headers object */
    EjsObj          *route;             /**< Matching route in route table */
    EjsString       *scheme;            /**< Scheme property */
    EjsString       *scriptName;        /**< ScriptName property */
    EjsHttpServer   *server;            /**< Owning server */
    EjsUri          *uri;               /**< Complete uri */
    EjsByteArray    *writeBuffer;       /**< Write buffer for capturing output */

    Ejs             *ejs;               /**< Ejscript interpreter handle */
    struct EjsSession *session;         /**< Current session */

    //  OPT - make bit fields
    int             dontAutoFinalize;   /**< Suppress auto-finalization */
    int             probedSession;      /**< Determined if a session exists */
    int             closed;             /**< Request closed and "close" event has been issued */
    int             error;              /**< Request errored and "error" event has been issued */
    int             finalized;          /**< Request has written all output data */
    int             running;            /**< Request has started */
    ssize           written;            /**< Count of data bytes written to the client */
} EjsRequest;

/** 
    Clone a request into another interpreter.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param req Original request to copy
    @param deep Ignored
    @return A new request object.
    @ingroup EjsRequest
*/
extern EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep);

/** 
    Create a new request. Create a new request object associated with the given Http connection.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server EjsHttpServer owning this request
    @param conn Http connection object
    @param dir Default directory containing web documents
    @return A new request object.
    @ingroup EjsRequest
*/
extern EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir);


/** 
    Session Class. Requests can access to session state storage via the Session class.
    @description
        Session objects represent a shared session state object into which Http Requests and store and retrieve data
        that persists beyond a single request.
    @stability Prototype
    @defgroup EjsSession EjsSession
    @see EjsSession ejsGetSession ejsDestroySession
 */
typedef struct EjsSession {
    EjsPot      pot;                /* Session properties */
    EjsString   *key;               /* Session ID key */
    EjsObj      *cache;             /* Cache store reference */
    EjsObj      *options;           /* Default write options */
    MprTicks    timeout;            /* Session inactivity timeout (msecs) */
    int         ready;              /* Data cached from store into pot */
} EjsSession;

/** 
    Get a session object for a given key. This will create a session if the given key is NULL or has expired.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param key String containing the session ID
    @param timeout Timeout to use for the session if one is created
    @param create Create a new session if an existing session cannot be found or it has expired.
    @returns A new session object.
    @ingroup EjsSession
*/
extern EjsSession *ejsGetSession(Ejs *ejs, EjsString *key, MprTicks timeout, int create);

/** 
    Destroy as session. This destroys the session object so that subsequent requests will need to establish a new session.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param session Session object created via ejsGetSession()
    @ingroup EjsSession
 */
extern int ejsDestroySession(Ejs *ejs, EjsSession *session);

/** 
    Set a session timeout
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param sp Session object
    @param lifespan Lifespan in milliseconds
    @ingroup EjsSession
*/
extern void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, MprTicks lifespan);

/******************************* Internal APIs ********************************/

extern void ejsConfigureHttpServerType(Ejs *ejs);
extern void ejsConfigureRequestType(Ejs *ejs);
extern void ejsConfigureSessionType(Ejs *ejs);
extern void ejsConfigureWebTypes(Ejs *ejs);
extern void ejsSendRequestCloseEvent(Ejs *ejs, EjsRequest *req);
extern void ejsSendRequestErrorEvent(Ejs *ejs, EjsRequest *req);

#ifdef  __cplusplus
}
#endif
#endif /* _h_EJS_WEB_h */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ejsCompiler.h"
 */
/************************************************************************/

/*
    ejsCompiler.h - Internal compiler header.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EC_COMPILER
#define _h_EC_COMPILER 1



#ifdef __cplusplus
extern "C" {
#endif

/*********************************** Defines **********************************/
/*
    Compiler validation modes. From "use standard|strict"
 */
//  TODO DOC
#define PRAGMA_MODE_STANDARD    1               /* Standard unstrict mode */
#define PRAGMA_MODE_STRICT      2               /* Strict mode */

//  TODO DOC
#define STRICT_MODE(cp)         (cp->fileState->strict)

/*
    Variable Kind bits
 */
#define KIND_CONST              0x1
#define KIND_VAR                0x2
#define KIND_LET                0x4

/*
    Phases for AST processing
 */
//  TODO DOC
#define EC_PHASE_DEFINE         0           /* Define types, functions and properties in types */
#define EC_PHASE_CONDITIONAL    1           /* Do conditional processing, hoisting and then type fixups */
#define EC_PHASE_FIXUP          2           /* Fixup type references */
#define EC_PHASE_BIND           3           /* Bind var references to slots and property types */
#define EC_AST_PHASES           4

typedef struct EcLocation {
    wchar       *source;
    char        *filename;
    int         lineNumber;
    int         column;
} EcLocation;

#define N_ARGS                  1
#define N_ASSIGN_OP             2
#define N_ATTRIBUTES            3
#define N_BINARY_OP             4
#define N_BINARY_TYPE_OP        5
#define N_BLOCK                 6
#define N_BREAK                 7
#define N_CALL                  8
#define N_CASE_ELEMENTS         9
#define N_CASE_LABEL            10
#define N_CATCH                 11
#define N_CATCH_ARG             12
#define N_CATCH_CLAUSES         13
#define N_CLASS                 14
#define N_CONTINUE              15
#define N_DASSIGN               16
#define N_DIRECTIVES            17
#define N_DO                    18
#define N_DOT                   19
#define N_END_FUNCTION          20
#define N_EXPRESSIONS           21
#define N_FIELD                 22
#define N_FOR                   23
#define N_FOR_IN                24
#define N_FUNCTION              25
#define N_GOTO                  26
#define N_HASH                  27
#define N_IF                    28
#define N_LITERAL               29
#define N_MODULE                30
#define N_NEW                   31
#define N_NOP                   32
#define N_OBJECT_LITERAL        33
#define N_PARAMETER             34
#define N_POSTFIX_OP            35
#define N_PRAGMA                36
#define N_PRAGMAS               37
#define N_PROGRAM               38
#define N_QNAME                 39
#define N_REF                   40
#define N_RETURN                41
#define N_SPREAD                42
#define N_SUPER                 43
#define N_SWITCH                44
#define N_THIS                  45
#define N_THROW                 46
#define N_TRY                   47
#define N_TYPE_IDENTIFIERS      48
#define N_UNARY_OP              49
#define N_USE_MODULE            50
#define N_USE_NAMESPACE         51
#define N_VALUE                 52
#define N_VAR                   53
#define N_VAR_DEFINITION        54
#define N_VOID                  55
#define N_WITH                  56

/*
    Ast node define
 */
#if !DOXYGEN
typedef struct EcNode   *Node;
#endif

//  TODO DOC
/*
    Structure for code generation buffers
 */
typedef struct EcCodeGen {
//  TODO DOC
    MprBuf      *buf;                           /* Code generation buffer */
    MprList     *jumps;                         /* Break/continues to patch for this code block */
    MprList     *exceptions;                    /* Exception handlers for this code block */
    EjsDebug    *debug;                         /* Source debug info */ 
    int         jumpKinds;                      /* Kinds of jumps allowed */
    int         breakMark;                      /* Stack item counter for the target for break/continue stmts */
    int         blockMark;                      /* Lexical block counter for the target for break/continue stmts */
    int         stackCount;                     /* Current stack item counter */
    int         blockCount;                     /* Current block counter */
    int         lastLineNumber;                 /* Last line for debug */
} EcCodeGen;


//  TODO DOC
typedef struct EcNode {
    char                *kindName;              /* Node kind string */
#if ME_DEBUG
    char                *tokenName;
#endif
    EjsName             qname;
    EcLocation          loc;                    /* Source code info */
    EjsBlock            *blockRef;              /* Block scope */
    EjsLookup           lookup;                 /* Lookup residuals */
    EjsNamespace        *namespaceRef;          /* Namespace reference */
    Node                left;                   /* children[0] */
    Node                right;                  /* children[1] */
    Node                typeNode;               /* Type of name */
    Node                parent;                 /* Parent node */
    MprList             *namespaces;            /* Namespaces for hoisted variables */
    MprList             *children;

    int                 kind;                   /* Kind of node */
    int                 attributes;             /* Attributes applying to this node */
    int                 tokenId;                /* Lex token */
    int                 groupMask;              /* Token group */
    int                 subId;                  /* Sub token */
    int                 slotNum;                /* Allocated slot for variable */
    int                 jumpLength;             /* Goto length for exceptions */
    int                 seqno;                  /* Unique sequence number */

    uint                blockCreated      : 1;  /* Block object has been created */
    uint                createBlockObject : 1;  /* Create the block object to contain let scope variables */
    uint                enabled           : 1;  /* Node is enabled via conditional definitions */
    int                 literalNamespace  : 1;  /* Namespace is a literal */
    uint                needThis          : 1;  /* Need to push this object */
    uint                needDupObj        : 1;  /* Need to dup the object on stack (before) */
    uint                needDup           : 1;  /* Need to dup the result (after) */
    uint                slotFixed         : 1;  /* Slot fixup has been done */
    uint                specialNamespace  : 1;  /* Using a public|private|protected|internal namespace */

    uchar               *patchAddress;          /* For code patching */
    EcCodeGen           *code;                  /* Code buffer */

    EjsName             *globalProp;            /* Set if this is a global property */
    EjsString           *doc;                   /* Documentation string */

    struct EcCompiler   *cp;                    /* Compiler instance reference */

#if ME_COMPILER_HAS_UNNAMED_UNIONS
    union {
#endif
        struct {
            Node        expression;
            EcCodeGen   *expressionCode;        /* Code buffer for the case expression */
            int         kind;
            int         nextCaseCode;           /* Goto length to the next case statements */
        } caseLabel;

        struct {
            Node        arg;                    /* Catch block argument */
        } catchBlock;

        /*
            Var definitions have one child per variable. Child nodes can be either N_NAME or N_ASSIGN_OP
         */
        struct {
            int         varKind;                /* Variable definition kind */
        } def;

        struct {
            /* Children are the catch clauses */
            Node        tryBlock;               /* Try code */
            Node        catchClauses;           /* Catch clauses */
            Node        finallyBlock;           /* Finally code */
            int         numBlocks;              /* Count of open blocks in the try block */
        } exception;

        struct {
            Node        expr;                   /* Field expression */
            Node        fieldName;              /* Field element name for objects */
            int         fieldKind;              /* value or function */
            int         index;                  /* Array index, set to -1 for objects */
            int         varKind;                /* Variable definition kind (const) */
        } field;

        struct {
            Node        resultType;             /* Function return type */
            Node        body;                   /* Function body */
            Node        parameters;             /* Function formal parameters */
            Node        constructorSettings;    /* Constructor settings */
            EjsFunction *functionVar;           /* Function variable */
            uint        operatorFn    : 1;      /* operator function */
            uint        getter        : 1;      /* getter function */
            uint        setter        : 1;      /* setter function */
            uint        call          : 1;      /* */
            uint        has           : 1;      /* */
            uint        hasRest       : 1;      /* Has rest parameter */
            uint        hasReturn     : 1;      /* Has a return statement */
            uint        isMethod      : 1;      /* Is a class method */
            uint        isConstructor : 1;      /* Is constructor method */
            uint        isDefault     : 1;      /* Is default constructor */
            uint        isExpression  : 1;      /* Is a function expression */
        } function;

        struct {
            Node        iterVar;
            Node        iterGet;
            Node        iterNext;
            Node        body;
            EcCodeGen   *initCode;
            EcCodeGen   *bodyCode;
            int         each;                   /* For each used */
        } forInLoop;

        struct {
            Node        body;
            Node        cond;
            Node        initializer;
            Node        perLoop;
            EcCodeGen   *condCode;
            EcCodeGen   *bodyCode;
            EcCodeGen   *perLoopCode;
        } forLoop;

        struct {
            Node        body;
            Node        expr;
            bool        disabled;
        } hash;

        struct {
            Node         implements;          /* Implemented interfaces */
            Node         constructor;         /* Class constructor */
            MprList      *staticProperties;   /* List of static properties */
            MprList      *instanceProperties; /* Implemented interfaces */
            MprList      *classMethods;       /* Static methods */
            MprList      *methods;            /* Instance methods */
            EjsType      *ref;                /* Type instance ref */
            EjsFunction  *initializer;        /* Initializer function */
            EjsNamespace *publicSpace;
            EjsNamespace *internalSpace;
            EjsString    *extends;            /* Class base class */
            int          isInterface;         /* This is an interface */
        } klass;

        struct {
            EjsObj      *var;               /* Special value */
            MprBuf      *data;              /* XML data */
        } literal;

        struct {
            EjsModule   *ref;               /* Module object */
            EjsString   *name;              /* Module name */
            char        *filename;          /* Module file name */
            int         version;
        } module;

        /*
            Name nodes hold a fully qualified name.
         */
        struct {
            Node        nameExpr;           /* Name expression */
            Node        qualifierExpr;      /* Qualifier expression */
            EjsObj      *nsvalue;           /* Initialization value (TODO - remove) */
            uint        instanceVar  : 1;   /* Instance or static var (if defined in class) */
            uint        isAttribute  : 1;   /* Attribute identifier "@" */
            uint        isDefault    : 1;   /* use default namespace */
            uint        isInternal   : 1;   /* internal namespace */
            uint        isLiteral    : 1;   /* use namespace "literal" */
            uint        isNamespace  : 1;   /* Name is a namespace */
            uint        isRest       : 1;   /* ... rest style args */
            uint        isType       : 1;   /* Name is a type */
            uint        letScope     : 1;   /* Variable is defined in let block scope */
            int         varKind;            /* Variable definition kind */
        } name;

        struct {
            int         callConstructors;   /* Bound type has a constructor */
        } newExpr;

        struct {
            Node        typeNode;           /* Type of object */
            int         isArray;            /* Array literal */
        } objectLiteral;

        struct {
            uint        strict;             /* Strict mode */
        } pragma;

        struct {
            MprList     *dependencies;      /* Accumulated list of dependent modules */
        } program;

        struct {
            Node        node;               /* Actual node reference */
        } ref;

        struct {
            int         blockless;          /* Function expression */
        } ret;

        struct {
            Node        cond;
            Node        thenBlock;
            Node        elseBlock;
            EcCodeGen   *thenCode;
            EcCodeGen   *elseCode;
        } tenary;

        struct {
            int         thisKind;           /* Kind of this. See EC_THIS_ flags */
        } thisNode;

        struct {
            int         minVersion;
            int         maxVersion;
        } useModule;

        struct {
            Node        object;
            Node        statement;
        } with;
#if ME_COMPILER_HAS_UNNAMED_UNIONS
    };
#endif
} EcNode;


/*
    Various per-node flags
 */
#define EC_THIS_GENERATOR       1
#define EC_THIS_CALLEE          2
#define EC_THIS_TYPE            3
#define EC_THIS_FUNCTION        4

#define EC_SWITCH_KIND_CASE     1   /* Case block */
#define EC_SWITCH_KIND_DEFAULT  2   /* Default block */

/*
    Object (and Array) literal field
 */
#define FIELD_KIND_VALUE        0x1
#define FIELD_KIND_FUNCTION     0x2

#define EC_NUM_NODES            8
#define EC_TAB_WIDTH            4

/*
    Fix clash with arpa/nameser.h
 */
#undef T_NULL

/*
    Lexical tokens (must start at 1)
    ASSIGN tokens must be +1 compared to their non-assignment counterparts.
    (Use genTokens to recreate)
    WARNING: ensure T_MOD and T_MOD_ASSIGN are adjacent. rewriteCompoundAssignment relies on this
 */
#define T_ASSIGN                    1
#define T_AT                        2
#define T_ATTRIBUTE                 3
#define T_BIT_AND                   4
#define T_BIT_AND_ASSIGN            5
#define T_BIT_OR                    6
#define T_BIT_OR_ASSIGN             7
#define T_BIT_XOR                   8
#define T_BIT_XOR_ASSIGN            9
#define T_BREAK                    10
#define T_CALL                     11
#define T_CALLEE                   12
#define T_CASE                     13
#define T_CAST                     14
#define T_CATCH                    15
#define T_CDATA_END                16
#define T_CDATA_START              17
#define T_CLASS                    18
#define T_COLON                    19
#define T_COLON_COLON              20
#define T_COMMA                    21
#define T_CONST                    22
#define T_CONTEXT_RESERVED_ID      23
#define T_CONTINUE                 24
#define T_DEBUGGER                 25
#define T_DECREMENT                26
#define T_DEFAULT                  27
#define T_DELETE                   28
#define T_DIV                      29
#define T_DIV_ASSIGN               30
#define T_DO                       31
#define T_DOT                      32
#define T_DOT_DOT                  33
#define T_DOT_LESS                 34
#define T_DOUBLE                   35
#define T_DYNAMIC                  36
#define T_EACH                     37
#define T_ELIPSIS                  38
#define T_ELSE                     39
#define T_ENUMERABLE               40
#define T_EOF                      41
#define T_EQ                       42
#define T_ERR                      43
#define T_EXTENDS                  44
#define T_FALSE                    45
#define T_FINAL                    46
#define T_FINALLY                  47
#define T_FLOAT                    48
#define T_FOR                      49
#define T_FUNCTION                 50
#define T_GE                       51
#define T_GENERATOR                52
#define T_GET                      53
#define T_GOTO                     54
#define T_GT                       55
#define T_HAS                      56
#define T_HASH                     57
#define T_ID                       58
#define T_IF                       59
#define T_IMPLEMENTS               60
#define T_IN                       61
#define T_INCLUDE                  62
#define T_INCREMENT                63
#define T_INSTANCEOF               64
#define T_INT                      65
#define T_INTERFACE                66
#define T_INTERNAL                 67
#define T_INTRINSIC                68
#define T_IS                       69
#define T_LBRACE                   70
#define T_LBRACKET                 71
#define T_LE                       72
#define T_LET                      73
#define T_LOGICAL_AND              74
#define T_LOGICAL_AND_ASSIGN       75
#define T_LOGICAL_NOT              76
#define T_LOGICAL_OR               77
#define T_LOGICAL_OR_ASSIGN        78
#define T_LOGICAL_XOR              79
#define T_LOGICAL_XOR_ASSIGN       80
#define T_LPAREN                   81
#define T_LSH                      82
#define T_LSH_ASSIGN               83
#define T_LT                       84
#define T_LT_SLASH                 85
#define T_MINUS                    86
#define T_MINUS_ASSIGN             87
#define T_MINUS_MINUS              88
#define T_MODULE                   89
#define T_MOD                      90       // WARNING sorted order manually fixed!!
#define T_MOD_ASSIGN               91
#define T_MUL                      92
#define T_MUL_ASSIGN               93
#define T_NAMESPACE                94
#define T_NATIVE                   95
#define T_NE                       96
#define T_NEW                      97
#define T_NOP                      98
#define T_NULL                     99
#define T_NUMBER                  100
#define T_NUMBER_WORD             101
#define T_OVERRIDE                102
#define T_PLUS                    103
#define T_PLUS_ASSIGN             104
#define T_PLUS_PLUS               105
#define T_PRIVATE                 106
#define T_PROTECTED               107
#define T_PROTOTYPE               108
#define T_PUBLIC                  109
#define T_QUERY                   110
#define T_RBRACE                  111
#define T_RBRACKET                112
#define T_REGEXP                  113
#define T_REQUIRE                 114
#define T_RESERVED_NAMESPACE      115
#define T_RETURN                  116
#define T_RPAREN                  117
#define T_RSH                     118
#define T_RSH_ASSIGN              119
#define T_RSH_ZERO                120
#define T_RSH_ZERO_ASSIGN         121
#define T_SEMICOLON               122
#define T_SET                     123
#define T_SLASH_GT                124
#define T_STANDARD                125
#define T_STATIC                  126
#define T_STRICT                  127
#define T_STRICT_EQ               128
#define T_STRICT_NE               129
#define T_STRING                  130
#define T_SUPER                   131
#define T_SWITCH                  132
#define T_THIS                    133
#define T_THROW                   134
#define T_TILDE                   135
#define T_TO                      136
#define T_TRUE                    137
#define T_TRY                     138
#define T_TYPE                    139
#define T_TYPEOF                  140
#define T_UINT                    141
#define T_UNDEFINED               142
#define T_USE                     143
#define T_VAR                     144
#define T_VOID                    145
#define T_WHILE                   146
#define T_WITH                    147
#define T_XML_COMMENT_END         148
#define T_XML_COMMENT_START       149
#define T_XML_PI_END              150
#define T_XML_PI_START            151
#define T_YIELD                   152

/*
    Group masks
 */
#define G_RESERVED          0x1
#define G_CONREV            0x2
#define G_COMPOUND_ASSIGN   0x4                 /* Eg. <<= */
#define G_OPERATOR          0x8                 /* Operator overload*/

/*
    Attributes (including reserved namespaces)
 */
#define A_FINAL         0x1
#define A_OVERRIDE      0x2
#define A_EARLY         0x4                     /* Early binding */
#define A_DYNAMIC       0x8
#define A_NATIVE        0x10
#define A_PROTOTYPE     0x20
#define A_STATIC        0x40
#define A_ENUMERABLE    0x40

#define EC_INPUT_STREAM "__stdin__"

struct EcStream;
typedef int (*EcStreamGet)(struct EcStream *stream);

/*
    Stream flags
 */
#define EC_STREAM_EOL       0x1                 /* End of line */

//  TODO DOC

typedef struct EcStream {
    struct EcCompiler *compiler;                /* Compiler back reference */
    EcLocation  loc;                            /* Source code debug info */
    EcLocation  lastLoc;                        /* Location info for a prior line */
    EcStreamGet getInput;                       /* Get more input callback */
    wchar       *buf;                           /* Buffer holding source file */
    wchar       *nextChar;                      /* Ptr to next input char */
    wchar       *end;                           /* Ptr to one past end of buf */
    bool        eof;                            /* At end of file */
    int         flags;                          /* Input flags */
} EcStream;


/*
    Parse source code from a file
 */
//  TODO DOC
typedef struct EcFileStream {
    EcStream    stream;
    MprFile     *file;
} EcFileStream;


/*
    Parse source code from a memory block
 */
//  TODO DOC
typedef struct EcMemStream {
    EcStream    stream;
} EcMemStream;


/*
    Parse input from the console (or file if using ejsh)
 */
//  TODO DOC
typedef struct EcConsoleStream {
    EcStream    stream;
} EcConsoleStream;


/*
    Program source input tokens
 */
//  TODO DOC
typedef struct EcToken {
    wchar       *text;                  /* Token text */
    int         length;                 /* Length of text in characters */
    int         size;                   /* Size of text in characters */
    int         tokenId;
    int         subId;
    int         groupMask;
    int         eol;                    /* At the end of the line */
    EcLocation  loc;                    /* Source code debug info */
    struct EcToken *next;               /* Putback and freelist linkage */
    EcStream    *stream;
#if ME_DEBUG
    char        *name;                  /* Debug token name */
#endif
} EcToken;


/*
    Jump types
 */
#define EC_JUMP_BREAK       0x1
#define EC_JUMP_CONTINUE    0x2
#define EC_JUMP_GOTO        0x4

//  TODO DOC
typedef struct EcJump {
    int             kind;               /* Break, continue */
    int             offset;             /* Code offset to patch */
    EcNode          *node;              /* Owning node */
} EcJump;


/*
    Current parse state. Each non-terminal production has its own state.
    Some state fields are inherited. We keep a linked list from EcCompiler.
 */
//  TODO DOC
typedef struct EcState {
    struct EcState  *next;                  /* State stack */
    uint            blockIsMethod    : 1;   /* Current function is a method */
    uint            captureBreak     : 1;   /* Capture break/continue inside a catch/finally block */
    uint            captureFinally   : 1;   /* Capture break/continue with a finally block */
    uint            conditional      : 1;   /* In branching conditional */
    uint            disabled         : 1;   /* Disable nodes below this scope */
    uint            dupLeft          : 1;   /* Dup left side */
    uint            inClass          : 1;   /* Inside a class declaration */
    uint            inFunction       : 1;   /* Inside a function declaration */
    uint            inHashExpression : 1;   /* Inside a # expression */
    uint            inInterface      : 1;   /* Inside an interface */
    uint            inMethod         : 1;   /* Inside a method declaration */
    uint            inSettings       : 1;   /* Inside constructor settings */
    uint            instanceCode     : 1;   /* Generating instance class code */
    uint            needsValue       : 1;   /* Expression must yield a value */
    uint            noin             : 1;   /* Don't allow "in" */
    uint            onLeft           : 1;   /* On the left of an assignment */
    uint            saveOnLeft       : 1;   /* Saved left of an assignment */
    uint            strict           : 1;   /* Compiler checking mode: Strict, standard*/

    int             blockNestCount;         /* Count of blocks encountered. Used by ejs shell */
    int             namespaceCount;         /* Count of namespaces originally in block. Used to pop namespaces */

    EjsModule       *currentModule;         /* Current open module definition */
    EjsType         *currentClass;          /* Current open class */
    EjsName         currentClassName;       /* Current open class name */
    EcNode          *currentClassNode;      /* Current open class */
    EjsFunction     *currentFunction;       /* Current open method */
    EcNode          *currentFunctionNode;   /* Current open method */
    EcNode          *currentObjectNode;     /* Left object in "." or "[" */
    EcNode          *topVarBlockNode;       /* Top var block node */

    EjsBlock        *letBlock;              /* Block for local block scope declarations */
    EjsBlock        *varBlock;              /* Block for var declarations */
    EjsBlock        *optimizedLetBlock;     /* Optimized let block declarations - may equal ejs->global */
    EcNode          *letBlockNode;          /* Node for the current let block */

    EjsString       *nspace;                /* Namespace for declarations */
    EjsString       *defaultNamespace;      /* Default namespace for new top level declarations. Does not propagate */

    EcCodeGen       *code;                  /* Global and function code buffer */
    EcCodeGen       *staticCodeBuf;         /* Class static level code generation buffer */
    EcCodeGen       *instanceCodeBuf;       /* Class instance level code generation buffer */

    struct EcState  *prevBlockState;        /* Block state stack */
    struct EcState  *breakState;            /* State for breakable blocks */
    struct EcState  *classState;            /* State for current class */
} EcState;


PUBLIC int      ecEnterState(struct EcCompiler *cp);
PUBLIC void     ecLeaveState(struct EcCompiler *cp);
PUBLIC EcNode   *ecLeaveStateWithResult(struct EcCompiler *cp,  struct EcNode *np);
PUBLIC int      ecResetModule(struct EcCompiler *cp, struct EcNode *np);
PUBLIC void     ecStartBreakableStatement(struct EcCompiler *cp, int kinds);


/*
    Primary compiler control structure
 */
//  TODO DOC
typedef struct EcCompiler {
    /*
        Properties ordered to make debugging easier
     */
    int         phase;                      /* Ast processing phase */
    EcState     *state;                     /* Current state */
    EcToken     *peekToken;                 /* Peek ahead token */
    EcToken     *token;                     /* Current input token */

    /*  Lexer */
    MprHash     *keywords;
    EcStream    *stream;
    EcToken     *putback;                   /* List of active putback tokens */
    char        *docToken;                  /* Last doc token */

    EcState     *fileState;                 /* Top level state for the file */
//  TODO -- these are risky and should be moved into state. A nested block, directive class etc willl modify
    EcState     *directiveState;            /* State for the current directive - used in parse and CodeGen */
    EcState     *blockState;                /* State for the current block */

    EjsLookup   lookup;                     /* Lookup residuals */
    EjsService  *vmService;                 /* VM runtime */
    Ejs         *ejs;                       /* Interpreter instance */
    MprList     *nodes;                     /* Compiled AST nodes */

    /*
        Compiler command line options
     */
    char        *certFile;                  /* Certificate to sign the module file */
    bool        debug;                      /* Run in debug mode */
    bool        doc;                        /* Include documentation strings in output */
    char        *extraFiles;                /* Extra source files to compile */

    MprList     *require;                   /* Required list of modules to pre-load */
    bool        interactive;                /* Interactive use (ejsh) */
    bool        merge;                      /* Merge all dependent modules */
    bool        bind;                       /* Don't bind properties to slots */
    bool        noout;                      /* Don't generate any module output files */
    bool        visibleGlobals;             /* Make globals visible (no namespace) */
    int         optimizeLevel;              /* Optimization factor (0-9) */
    bool        shbang;                     /* Observe #!/path as the first line of a script */
    int         warnLevel;                  /* Warning level factor (0-9) */

    int         strict;                     /* Compiler default strict mode */
    int         lang;                       /* Language compliance level: ecma|plus|fixed */
    char        *outputDir;                 /* Output directory for modules */
    char        *outputFile;                /* Output module file name override */
    MprFile     *file;                      /* Current output file handle */

    int         modver;                     /* Default module version */
    int         parseOnly;                  /* Only parse the code */
    int         strip;                      /* Strip debug symbols */
    int         tabWidth;                   /* For error reporting "^" */

    MprList     *modules;                   /* List of modules to process */
    MprList     *fixups;                    /* Type reference fixups */

    char        *errorMsg;                  /* Aggregated error messages */
    int         error;                      /* Unresolved parse error */
    int         fatalError;                 /* Any a fatal error - Can't continue */
    int         errorCount;                 /* Count of all errors */
    int         warningCount;               /* Count of all warnings */
    int         nextSeqno;                  /* Node sequence numbers */
    int         blockLevel;                 /* Level of nest in blocks */

    /*
        TODO - aggregate these into flags
     */
    int         lastOpcode;                 /* Last opcode encoded */
    int         uid;                        /* Unique identifier generator */
} EcCompiler;

/********************************** Prototypes *******************************/

//  TODO -- reorder
//  TODO DOC
PUBLIC int          ecAddModule(EcCompiler *cp, EjsModule *mp);
PUBLIC EcNode       *ecAppendNode(EcNode *np, EcNode *child);
PUBLIC int          ecAstFixup(EcCompiler *cp, struct EcNode *np);
PUBLIC EcNode       *ecChangeNode(EcCompiler *cp, EcNode *np, EcNode *oldNode, EcNode *newNode);
PUBLIC void         ecGenConditionalCode(EcCompiler *cp, EcNode *np, EjsModule *up);
PUBLIC int          ecCodeGen(EcCompiler *cp);
PUBLIC int          ecCompile(EcCompiler *cp, int argc, char **path);
PUBLIC EcCompiler   *ecCreateCompiler(struct Ejs *ejs, int flags);
PUBLIC void         ecDestroyCompiler(EcCompiler *cp);
PUBLIC void         ecInitLexer(EcCompiler *cp);
PUBLIC EcNode       *ecCreateNode(EcCompiler *cp, int kind);
PUBLIC void         ecFreeToken(EcCompiler *cp, EcToken *token);
PUBLIC char         *ecGetErrorMessage(EcCompiler *cp);
PUBLIC EjsString    *ecGetInputStreamName(EcCompiler *lp);
PUBLIC int          ecGetToken(EcCompiler *cp);
PUBLIC int          ecGetRegExpToken(EcCompiler *cp, wchar *prefix);
PUBLIC EcNode       *ecLinkNode(EcNode *np, EcNode *child);

PUBLIC EjsModule    *ecLookupModule(EcCompiler *cp, EjsString *name, int minVersion, int maxVersion);
PUBLIC int          ecLookupScope(EcCompiler *cp, EjsName name);
PUBLIC int          ecLookupVar(EcCompiler *cp, EjsAny *vp, EjsName name);
PUBLIC EcNode       *ecParseWarning(EcCompiler *cp, char *fmt, ...);
PUBLIC int          ecPeekToken(EcCompiler *cp);
PUBLIC int          ecPutSpecificToken(EcCompiler *cp, EcToken *token);
PUBLIC int          ecPutToken(EcCompiler *cp);
PUBLIC void         ecError(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, ...);
PUBLIC void         ecErrorv(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, va_list args);
PUBLIC void         ecResetInput(EcCompiler *cp);
PUBLIC EcNode       *ecResetError(EcCompiler *cp, EcNode *np, bool eatInput);
PUBLIC int          ecRemoveModule(EcCompiler *cp, EjsModule *mp);
PUBLIC void         ecResetParser(EcCompiler *cp);
PUBLIC int          ecResetModuleList(EcCompiler *cp);
PUBLIC int          ecOpenConsoleStream(EcCompiler *cp, EcStreamGet gets, cchar *contents);
PUBLIC int          ecOpenFileStream(EcCompiler *cp, cchar *path);
PUBLIC int          ecOpenMemoryStream(EcCompiler *cp, cchar *contents, ssize len);
PUBLIC void         ecCloseStream(EcCompiler *cp);
PUBLIC void         ecSetOptimizeLevel(EcCompiler *cp, int level);
PUBLIC void         ecSetWarnLevel(EcCompiler *cp, int level);
PUBLIC void         ecSetStrictMode(EcCompiler *cp, int on);
PUBLIC void         ecSetTabWidth(EcCompiler *cp, int width);
PUBLIC void         ecSetOutputDir(EcCompiler *cp, cchar *outputDir);
PUBLIC void         ecSetOutputFile(EcCompiler *cp, cchar *outputFile);
PUBLIC void         ecSetCertFile(EcCompiler *cp, cchar *certFile);
PUBLIC EcToken      *ecTakeToken(EcCompiler *cp);
PUBLIC int          ecAstProcess(struct EcCompiler *cp);
PUBLIC void         *ecCreateStream(EcCompiler *cp, ssize size, cchar *filename, void *manager);
PUBLIC void         ecSetStreamBuf(EcStream *sp, cchar *contents, ssize len);
PUBLIC EcNode       *ecParseFile(EcCompiler *cp, char *path);
PUBLIC void         ecManageStream(EcStream *sp, int flags);
PUBLIC void         ecMarkLocation(EcLocation *loc);
PUBLIC void         ecSetRequire(EcCompiler *cp, MprList *modules);


/*
    Module file creation routines.
 */
PUBLIC void     ecAddFunctionConstants(EcCompiler *cp, EjsPot *obj, int slotNum);
PUBLIC void     ecAddConstants(EcCompiler *cp, EjsAny *obj);
PUBLIC int      ecAddStringConstant(EcCompiler *cp, EjsString *sp);
PUBLIC int      ecAddCStringConstant(EcCompiler *cp, cchar *str);
PUBLIC int      ecAddNameConstant(EcCompiler *cp, EjsName qname);
PUBLIC int      ecAddDocConstant(EcCompiler *cp, cchar *tag, void *vp, int slotNum);
PUBLIC int      ecAddModuleConstant(EcCompiler *cp, EjsModule *up, cchar *str);
PUBLIC int      ecCreateModuleHeader(EcCompiler *cp);
PUBLIC int      ecCreateModuleSection(EcCompiler *cp);


/*
    Encoding emitter routines
 */
PUBLIC void      ecEncodeBlock(EcCompiler *cp, cuchar *buf, int len);
PUBLIC void      ecEncodeByte(EcCompiler *cp, int value);
PUBLIC void      ecEncodeByteAtPos(EcCompiler *cp, int offset, int value);
PUBLIC void      ecEncodeConst(EcCompiler *cp, EjsString *sp);
PUBLIC void      ecEncodeDouble(EcCompiler *cp, double value);
PUBLIC void      ecEncodeGlobal(EcCompiler *cp, EjsAny *obj, EjsName qname);
PUBLIC void      ecEncodeInt32(EcCompiler *cp, int value);
PUBLIC void      ecEncodeInt32AtPos(EcCompiler *cp, int offset, int value);
PUBLIC void      ecEncodeNum(EcCompiler *cp, int64 number);
PUBLIC void      ecEncodeName(EcCompiler *cp, EjsName qname);
PUBLIC void      ecEncodeMulti(EcCompiler *cp, cchar *str);
PUBLIC void      ecEncodeWideAsMulti(EcCompiler *cp, wchar *str);
PUBLIC void      ecEncodeOpcode(EcCompiler *cp, int value);

PUBLIC void     ecCopyCode(EcCompiler *cp, uchar *pos, int size, int dist);
PUBLIC uint     ecGetCodeOffset(EcCompiler *cp);
PUBLIC int      ecGetCodeLen(EcCompiler *cp, uchar *mark);
PUBLIC void     ecAdjustCodeLength(EcCompiler *cp, int adj);

#ifdef __cplusplus
}
#endif
#endif /* _h_EC_COMPILER */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
#endif /* ME_COM_EJS */
