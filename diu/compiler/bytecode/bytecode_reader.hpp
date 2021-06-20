#ifndef BYTECODE_READER_H
#define BYTECODE_READER_H
#include <string>
using std::string;
#include <fstream>
#include "op_code.hpp"
#include "bytestream.hpp"
#include <unordered_map>
#include <iostream>
using std::cout;
using std::endl;
using std::unordered_map;
#include <vector>
using std::vector;


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
        stream.seekg(0, std::ios::end);
        len = stream.tellg();
        stream.seekg(0, std::ios::beg);

        auto magic = read<unsigned int>();
        if (magic != _DIU_MAGIC)
        {
            throw load_error("This is not diu file.");
        }
    }
    int len;
    ~bytecode_reader() {}

    template <typename T>
    T read()
    {
        stream.read(buffer, sizeof(T));
        return *(reinterpret_cast<T *>(buffer));
    }

    void readall()
    {
        auto version = read<double>();
        auto const_count = read<int>();
        vector<string> const_string;

        cout << "-----[CONST]-----" << endl;
        for (auto i = 0; i < const_count; i++)
        {
            auto const_type_ = const_value_type(read<byte>());
            switch (const_type_)
            {
            case const_value_type::DOUBLE:
            {
                cout << "[D] ";
                cout << read<double>() << endl;
            }
            break;
            case const_value_type::STRING:
            {
                cout << "[S] ";
                auto size = read<unsigned int>();
                auto _buffer = new char[size + 1];
                stream.get(_buffer, size + 1);
                auto str = string(_buffer);
                delete[] _buffer;
                const_string.push_back(str);
                cout << str << endl;
            }
            break;
            default:
                cout << "[?] " << endl;
                break;
            }
        }
        cout << "-----[PACKAGE]-----" << endl;
        auto package_count = read<int>();
        vector<int> package_index_map;
        for (auto i = 0; i < package_count; i++)
        {
            auto pkg_index = read<int>();
            package_index_map.push_back(pkg_index);
            cout << const_string[pkg_index] << endl;
        }
        cout << "-----[SYMBOL]-----" << endl;
        auto symbol_count = read<int>();
        for (auto i = 0; i < symbol_count; i++)
        {
            auto symbol_name_index = read<int>();
            auto package_index = read<int>();
            cout << const_string[symbol_name_index] << " : " << const_string[package_index_map[package_index]] << endl;
        }
        auto node_count = read<int>();
        for (auto i = 0; i < node_count; i++)
        {
            auto node_name_index = read<int>();
            auto func_count = read<int>();
            cout << "======[NODE  " << const_string[node_name_index] << "  ]======" << endl;
            for (auto j = 0; j < func_count; j++)
            {
                auto func_name_index = read<int>();
                auto is_static = read<byte>();
                auto args_count = read<byte>();
                cout << const_string[func_name_index] << endl;
                for (auto k = 0; k < args_count; k++)
                {
                    auto arg_name_index = read<int>();
                    auto arg_type_name_index = read<int>();
                    cout << "[ARG] " << const_string[arg_name_index] << " : " << const_string[arg_type_name_index] << endl;
                }
                cout << "-------------------" << endl;
                auto statement = read<int>();
                for (auto i = 0; i < statement; i += 4)
                {
                    auto op = opcode(read<opcode_type>());
                    auto info = read<unsigned char>();
                    auto cmd = read<int16_t>();
                    switch (op)
                    {
                    case opcode::LET:
                        cout << "LET\t\t\t" << 0 << " " << 0 << endl;
                        break;
                    case opcode::LET_C:
                        cout << "LET_C\t\t\t" << 0 << " " << 0 << endl;
                        break;
                    case opcode::VAR:
                        cout << "VAR\t\t\t" << 0 << " " << const_string[cmd] << endl;
                        break;
                    case opcode::LOAD:
                        cout << "LOAD\t\t\t" << 0 << " " << const_string[cmd] << endl;
                        break;
                    case opcode::VAR_FIND:
                        cout << "VAR_FIND\t\t" << 0 << " " << const_string[cmd] << endl;
                        break;
                    case opcode::VAR_FIND_C:
                        cout << "VAR_FIND_C\t\t" << 0 << " " << const_string[cmd] << endl;
                        break;
                    case opcode::FUNC_CALL_LOCAL:
                        cout << "FUNC_CALL_LOCAL\t\t" << info << " " << const_string[cmd] << endl;
                        break;
                    case opcode::FUNC_CALL_BY_NAME:
                        cout << "FUNC_CALL_BY_NAME\t" << info << " " << const_string[cmd] << endl;
                        break;
                    case opcode::RETURN:
                        cout << "RETURN\t\t" << endl;
                        break;
                    case opcode::CALC_OP:
                        cout << "CALC_OP\t\t\t"
                             << "-"
                             << " " << 0 << endl;
                        break;
                    case opcode::JUMP:
                        cout << "JUMP\t\t\t" << 0 << " " << cmd << endl;
                        break;
                    case opcode::JUMP_NIF:
                        cout << "JUMP_NIF\t\t" << 0 << " " << cmd << endl;
                        break;
                    default:
                        cout << "??\t\t\t"
                             << " " << int(op) << " " << (int)info << " " << (int)cmd << endl;
                        break;
                    }
                }
                cout << "-------------------" << endl;
            }
        }
    }
};

#endif