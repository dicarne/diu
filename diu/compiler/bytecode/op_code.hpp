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

};

enum class const_value_type : byte
{
    UNKNOWN = 0,
    INT = 1,
    DOUBLE = 2,
    STRING = 3
};
#endif