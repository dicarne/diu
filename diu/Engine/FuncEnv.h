#ifndef FUNC_ENV_H_
#define FUNC_ENV_H_
#include <memory>
#include <string>
#include <unordered_map>
#include "codes/CodeFunc.hpp"
#include "Type.hpp"
#include "NodeMessage.hpp"
#include "runtime_error.hpp"
#include "../compiler/bytecode/op_code.hpp"
#include "../compiler/compiler_type.h"
#include <stack>
#include <iostream>
#include "Node.h"
using std::cout;
using std::endl;
using std::make_shared;
using std::shared_ptr;
using std::stack;
using std::string;
using std::unordered_map;
class Node;
class FuncEnv
{
private:
    stack<shared_ptr<Object>> runtime;

    void call_another_func(string name, vector<Object> args, bool async_);
    void call_another_func(shared_ptr<Object> symbol, string name, vector<Object> args, bool async_);
    int async_index = 0;
public:
    bool completed;
    bool failed;
    bool waitting;
    int async_call = 0;
    shared_ptr<CodeFunc> code;
    unordered_map<string, shared_ptr<Object>> local_var;
    unordered_map<int, shared_ptr<Object>> waitting_async;
    shared_ptr<Object> ret;
    Node *node;
    int id = 0;
    int cur = 0;
    PID callback_node;
    int callback_id = 0;
    string name;
    FuncEnv(/* args */) {}
    ~FuncEnv() {}

    void init(shared_ptr<CodeFunc> c, shared_ptr<NodeMessage> m)
    {
        code = c;
        cur = 0;
        completed = false;
        failed = false;
        if (!code->mod.expired())
        {
            auto env = code->mod.lock();
            int index = 0;
            for (auto &a : code->args)
            {
                local_var[(*env->const_string)[a.arg_name]] = make_shared<Object>(m->args[index]);
                index++;
            }
            name = code->name;
        }
    }

    void handle_callback(Object callback)
    {
        ret = make_shared<Object>(callback);
        waitting = false;
    }

    void handle_async_callback(Object callback, Object async_id)
    {
        ret = make_shared<Object>(callback);
        auto await_id = async_id.getv<int>();
        waitting_async[await_id] = ret;
    }

    void run(int &limit);
};
#endif