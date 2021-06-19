#ifndef BYTECODE_READER_H
#define BYTECODE_READER_H
#include <string>
using std::string;
#include <fstream>
#include "op_code.hpp"
#include "bytestream.hpp"

#include <iostream>
using std::cout;
using std::endl;

class bytecode_reader
{
private:
    string path;
    std::ifstream stream;
    char buffer[9];
public:
    bytecode_reader(string path)
    {
        stream = std::ifstream(path, std::ios::binary);
        auto magic = read<unsigned int>();
        if(magic != _DIU_MAGIC) {
            throw load_error("This is not diu file.");
        }
    }
    ~bytecode_reader() {}

    template <typename T>
    T read()
    {
        stream.get(buffer, sizeof(T) + 1);
        return *(reinterpret_cast<T *>(buffer));
    }

    void readall() {
        auto version = read<int>();
        auto const_count = read<int>();
    }
};

#endif