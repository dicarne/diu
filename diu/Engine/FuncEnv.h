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
#include "Object/object.h"

using std::cout;
using std::endl;
using std::make_shared;
using std::shared_ptr;
using std::stack;
using std::string;
using std::unordered_map;
class Node;
class Object;
class FuncEnv
{
private:
    shared_ptr<stack<Object::Ptr>> runtime;
    stack<shared_ptr<stack<Object::Ptr>>> stacks;
    void call_another_func(string name, vector<Object> args, bool async_);
    void call_another_func(Object::Ptr symbol, string name, vector<Object> args, bool async_);
    int async_index = 0;
public:
    bool completed;
    bool failed;
    bool waitting;
    int async_call = 0;
    shared_ptr<CodeFunc> code;
    unordered_map<string, Object::Ptr> local_var;
    unordered_map<int, Object::Ptr> waitting_async;
    Object::Ptr ret;
    Node *node;
    int id = 0;
    int cur = 0;
    PID callback_node;
    int callback_id = 0;
    string name;
    FuncEnv(/* args */) {}
    ~FuncEnv() {}

    void init(shared_ptr<CodeFunc> c, shared_ptr<NodeMessage> m);
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