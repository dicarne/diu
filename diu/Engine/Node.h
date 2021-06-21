#ifndef NODE_H
#define NODE_H

#include <queue>
#include <string>
#include <vector>
#include <memory>
#include "Type.hpp"
#include "codes/CodeNode.hpp"
#include "util/LockFreeQueue.hpp"
#include "FuncEnv.h"
#include <list>
#include "NodeMessage.hpp"
#include "Engine.h"
#include <iostream>

using std::list;
using std::make_shared;
using std::queue;
using std::string;
using std::vector;
class Engine;
class Node
{

private:
    bool handle_message;
    shared_ptr<CodeNode> code_page;
    int this_cycle_cmd_count = 0;
    shared_ptr<FuncEnv> create_func(shared_ptr<NodeMessage> p);
    void run_func(shared_ptr<FuncEnv> p);
    list<shared_ptr<FuncEnv>> run_env;
    const int limit = 200;
    int func_index = 0;
    unordered_map<int, FuncEnv *> waitting_callback;
    LockFreeArrayQueue<NodeMessage *> *messageBox;

public:
    PID Pid;
    bool active = true;
    Engine *engine;
    Node();
    ~Node();

    void direct_call(NodeMessage message);
    void load_code(shared_ptr<CodeNode> new_code_page)
    {
        code_page = new_code_page;
    }
    void init()
    {
        active = true;
    }

    void push_massage(NodeMessage *msg);
    void run_once();

    // Local
    void call_another_func(FuncEnv *caller, shared_ptr<NodeMessage> msg);
    // Remote
    void call_another_func(FuncEnv *caller, shared_ptr<Object> symbol, shared_ptr<NodeMessage> msg);
};

#endif