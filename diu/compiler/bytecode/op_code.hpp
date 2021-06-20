#ifndef OPCODE_H_
#define OPCODE_H_
typedef unsigned char byte;
typedef byte opcode_type;

typedef unsigned short int ushort;

inline unsigned int _DIU_MAGIC = 0xD1676A25;
enum class opcode : opcode_type
{
    NONE = 0,
    VERSION = 1,
    LET = 8,                // 将栈顶赋值给名字索引对应的变量 LET 0 NAME_INDEX
    LET_C = 9,              // 将栈顶赋值给栈顶名字索引对应的变量 LET 0 NAME_INDEX （查找对象
    VAR = 10,               // 声明一个新变量，将变量名加入局部变量表中，并申请一个新的空间 VAR 0 NAME_INDEX
    LOAD = 11,              // 将一个常量读取，按照类型初始化，并加入栈顶 LOAD 0 NAME_INDEX
    VAR_FIND = 12,          // 根据名字，查找符号，若找到，则加入栈顶 VAR_FIND 0 NAME_INDEX
    VAR_FIND_C = 13,        // 根据名字，在栈顶对象内查找此名字，若为对象，则加入栈顶 VAR_FIND_C 0 NAME_INDEX
    FUNC_CALL_LOCAL = 14,   // 根据名字，用栈顶N个元素进行函数调用 FUNC_CALL_LOCAL ARG_COUNT NAME_INDEX
    FUNC_CALL_BY_NAME = 15, // 根据名字，在栈顶的指定函数，用栈顶N个元素进行函数调用 FUNC_CALL_BY_NAME ARG_COUNT NAME_INDEX
    RETURN = 16,            // RETUEN 0 0
    CALC_OP = 17,           // 计算符号 CALC_OP OPCODE
    JUMP = 18,              // JUMP 0 OFFSET
    JUMP_NIF = 19,          // JUMP_NIF 0 OFFSET
    WAIT_FUNC_CALL = 20,    // 等待函数返回
};

struct opcomd
{
    opcode op;
    byte info;
    ushort data;
};
// if false ->
// -if true
// ...
// jump end
// if false -> // elif

enum class const_value_type : byte
{
    UNKNOWN = 0,
    INT = 1,
    DOUBLE = 2,
    STRING = 3
};
#endif