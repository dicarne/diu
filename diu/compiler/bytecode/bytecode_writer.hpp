#ifndef BYTECODE_WRITER_H_
#define BYTECODE_WRITER_H_
#include <deque>
#include <string>
#include <sstream>
#include <fstream>
#include "op_code.hpp"
#include "bytestream.hpp"

using std::deque;
using std::string;
using std::stringstream;

class bytecode_writer
{
private:
    stringstream metadata;
    stringstream constvalue;
    stringstream packges;
    stringstream tmp;
    std::ofstream file;
    double engine_version;
    void make_metadata();
    int const_count;
    int pkg_count;

    template <typename T>
    void write_metadata(T value)
    {
        metadata.write(reinterpret_cast<char *>(&value), sizeof(value));
    }

    template <typename T>
    void write_file(T value)
    {
        file.write(reinterpret_cast<char *>(&value), sizeof(value));
    }

    template <typename T>
    void write_const(T value)
    {
        constvalue.write(reinterpret_cast<char *>(&value), sizeof(value));
    }

    void write_file(string value)
    {
        file << value;
    }

public:
    string path;
    bytecode_writer(string path)
    {
        this->path = path;
    }
    ~bytecode_writer();
    void set_meta_data(double engine_version)
    {
        this->engine_version = engine_version;
    }

    void make();

    // 返回常量索引
    int write_const_double(string num)
    {
        const_count++;
        tmp.str("");
        tmp << num;
        double d;
        tmp >> d;

        write_const(const_value_type::DOUBLE);
        write_const(d);
        return const_count - 1;
    }

    // 返回常量索引
    int write_const_int(string num)
    {
        const_count++;
        tmp.str("");
        tmp << num;
        int d;
        tmp >> d;

        write_const(const_value_type::INT);
        write_const(d);
        return const_count - 1;
    }

    // 返回常量索引
    int write_const_string(string num)
    {
        const_count++;
        write_const(const_value_type::STRING);
        unsigned int size = num.size();
        write_const(size);
        constvalue << num;
        return const_count - 1;
    }

    void reg_package(int name_index) {
        packges.write(reinterpret_cast<char *>(&name_index), sizeof(name_index));
        pkg_count++;
    }
};

bytecode_writer::~bytecode_writer()
{
}

void bytecode_writer::make_metadata()
{
    metadata.str("");
    write_metadata(_DIU_MAGIC);     // 用于判断是否是diu的二进制文件
    write_metadata(engine_version); // 执行引擎版本
}

void bytecode_writer::make()
{
    file = std::ofstream(path, std::ios::binary);

    make_metadata();
    file << metadata.str();

    write_file(const_count);
    file << constvalue.str();

    write_file(pkg_count);
    file << packges.str();

    file.close();
}

#endif

// MAGIC_NUM                        int(4)
// VERSION                          double(8)
// CONST VALUE COUNT                int(4)
// -------------CONST VALUE---------------
// CONST TYPE                       byte(1)
// LENGTH(?)                        uint(4)
// DATA                             int(4) double(8) string(n)
// .............. *N .....................
// PACKAGE_COUNT                    int(4)
// ------------IMPORT PACKAGE-------------
// PACKANGE_NAME_[INDEX]            int(4)
// .............. *N .....................
// ------------IMPORT SYMBOL--------------
// SYMBOL_NAME_[INDEX]              int(4)
// PACKAGE_IMPORT_[INDEX]           int(4)
// .............. *N .....................
// NODE COUNT                       int(4)
// -----------------NODE--------------------
// .----------------FUNC DEF------------------.
// | FUNC_NAME_[INDEX ]               int(4)  |
// | STATIC                           byte(1) |
// | ARGS COUNT                       byte(1) |
// | ---------------ARGS----------------------|
// | PARAM_NAME_[INDEX]               int(4)  |
// | PARAM_TYPE_NAME_[INDEX]          int(4)  |
// |.............. *N ........................|
// | --------------STATEMENT------------------|
// | OP CODE                    opcode_type(1)|
// | OP BODY                          int(4)  |
// |.............. *N ........................|
// |------------------------------------------|
// .............. *N .......................