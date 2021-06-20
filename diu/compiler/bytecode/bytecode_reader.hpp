#ifndef BYTECODE_READER_H
#define BYTECODE_READER_H
#include <string>
using std::string;
#include <fstream>
#include "op_code.hpp"
#include "bytestream.hpp"
#include <unordered_map>
#include <iostream>
#include "../../Engine/codes/CodeModule.hpp"
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
    char buffer[20];

public:
    double version;
    bytecode_reader(string path)
    {
        stream = std::ifstream(path, std::ios::binary);
        this->path = path;
        auto magic = read<unsigned int>();
        version = read<double>();
        if (magic != _DIU_MAGIC)
        {
            throw load_error("This is not diu file.");
        }
    }

    ~bytecode_reader() {}

    template <typename T>
    T read()
    {
        stream.read(buffer, sizeof(T));
        return *(reinterpret_cast<T *>(buffer));
    }

    shared_ptr<CodeModule> readall()
    {
        auto mod = make_shared<CodeModule>();
        mod->module_name = path;

        mod->engine_version = version;

        auto const_count = read<int>();
        mod->const_string = make_shared<unordered_map<int, string>>();
        mod->const_double = make_shared<unordered_map<int, double>>();
        mod->const_type = make_shared<vector<byte>>();

        cout << "-----[CONST]-----" << endl;
        for (auto i = 0; i < const_count; i++)
        {
            auto const_type_ = const_value_type(read<byte>());
            mod->const_type->push_back(byte(const_type_));
            switch (const_type_)
            {
            case const_value_type::DOUBLE:
            {
                auto d = read<double>();
                (*mod->const_double)[i] = d;

                cout << "[D] ";
                cout << d << endl;
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
                (*mod->const_string)[i] = str;

                cout << str << endl;
            }
            break;
            default:
                cout << "[?] " << endl;
                throw load_error("unknown op code!");
                break;
            }
        }

        cout << "-----[PACKAGE]-----" << endl;

        auto package_count = read<int>();
        mod->package_index_map = make_shared<vector<int>>();
        for (auto i = 0; i < package_count; i++)
        {
            auto pkg_index = read<int>();
            mod->package_index_map->push_back(pkg_index);

            cout << (*mod->const_string)[pkg_index] << endl;
        }
        cout << "-----[SYMBOL]-----" << endl;

        mod->outer_symbol_pkg_map = make_shared<unordered_map<int, int>>();
        auto symbol_count = read<int>();
        for (auto i = 0; i < symbol_count; i++)
        {
            auto symbol_name_index = read<int>();
            auto package_index = read<int>();
            (*mod->outer_symbol_pkg_map)[symbol_name_index] = package_index;

            cout << (*mod->const_string)[symbol_name_index] << " : " << (*mod->const_string)[(*mod->package_index_map)[package_index]] << endl;
        }

        mod->nodes = make_shared<unordered_map<string, shared_ptr<CodeNode>>>();
        auto node_count = read<int>();
        for (auto i = 0; i < node_count; i++)
        {
            auto nodeptr = make_shared<CodeNode>();
            nodeptr->mod = mod;

            auto node_name_index = read<int>();
            auto func_count = read<int>();

            cout << "======[NODE  " << (*mod->const_string)[node_name_index] << "  ]======" << endl;
            for (auto j = 0; j < func_count; j++)
            {
                auto funcptr = make_shared<CodeFunc>();
                funcptr->node = nodeptr;
                funcptr->mod = mod;

                auto func_name_index = read<int>();
                auto is_static = read<byte>();
                auto args_count = read<byte>();

                cout << (*mod->const_string)[func_name_index] << endl;

                for (auto k = 0; k < args_count; k++)
                {
                    auto arg_name_index = read<int>();
                    auto arg_type_name_index = read<int>();
                    CodeFunc::arg arg;
                    arg.arg_name = arg_name_index;
                    arg.arg_type = arg_type_name_index;
                    funcptr->args.push_back(arg);

                    cout << "[ARG] " << (*mod->const_string)[arg_name_index] << " : " << (*mod->const_string)[arg_type_name_index] << endl;
                }
                cout << "-------------------" << endl;
                auto statement = read<int>();
                for (auto i = 0; i < statement; i += 4)
                {
                    auto op = opcode(read<opcode_type>());
                    auto info = read<unsigned char>();
                    auto cmd = read<int16_t>();
                    opcomd ocmd;
                    ocmd.op = op;
                    ocmd.info = info;
                    ocmd.data = cmd;
                    funcptr->codes.push_back(ocmd);

                    switch (op)
                    {
                    case opcode::LET:
                        cout << "LET\t\t\t" << 0 << " " << 0 << endl;
                        break;
                    case opcode::LET_C:
                        cout << "LET_C\t\t\t" << 0 << " " << 0 << endl;
                        break;
                    case opcode::VAR:
                        cout << "VAR\t\t\t" << 0 << " " << (*mod->const_string)[cmd] << endl;
                        break;
                    case opcode::LOAD:
                        cout << "LOAD\t\t\t" << 0 << " " << (*mod->const_string)[cmd] << endl;
                        break;
                    case opcode::VAR_FIND:
                        cout << "VAR_FIND\t\t" << 0 << " " << (*mod->const_string)[cmd] << endl;
                        break;
                    case opcode::VAR_FIND_C:
                        cout << "VAR_FIND_C\t\t" << 0 << " " << (*mod->const_string)[cmd] << endl;
                        break;
                    case opcode::FUNC_CALL_LOCAL:
                        cout << "FUNC_CALL_LOCAL\t\t" << info << " " << (*mod->const_string)[cmd] << endl;
                        break;
                    case opcode::FUNC_CALL_BY_NAME:
                        cout << "FUNC_CALL_BY_NAME\t" << info << " " << (*mod->const_string)[cmd] << endl;
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
                    case opcode::WAIT_FUNC_CALL:
                        cout << "WAIT_FUNC_CALL\t" << 0 << " " << cmd << endl;
                        break;
                    default:
                        cout << "??\t\t\t"
                             << " " << int(op) << " " << (int)info << " " << (int)cmd << endl;
                        break;
                    }
                }
                cout << "-------------------" << endl;
                nodeptr->funcs[(*mod->const_string)[func_name_index]] = funcptr;
            }
            (*mod->nodes)[(*mod->const_string)[node_name_index]] = nodeptr;
        }
        return mod;
    }
};

#endif