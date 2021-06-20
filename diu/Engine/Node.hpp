#ifndef NODE_H
#define NODE_H

#include <queue>
#include <string>
#include <vector>
#include <memory>
#include "Type.hpp"
#include "codes/CodeNode.hpp"
#include "util/LockFreeQueue.hpp"
#include "FuncEnv.hpp"
#include <list>
#include "NodeMessage.hpp"
#include "Engine.hpp"

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

public:
    PID Pid;
    LockFreeArrayQueue<NodeMessage *> *messageBox;
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
    }

    void run_once();

    void call_another_func(FuncEnv *caller, shared_ptr<NodeMessage> msg)
    {
        // Local Call
        auto f = create_func(msg);
        waitting_callback[msg->id] = caller;
        run_func(f);
    }
    void call_another_func(FuncEnv *caller, shared_ptr<Object> symbol, NodeMessage *msg)
    {
        cout << msg->name << endl;
        // TODO:
    }
};

Node::Node()
{
    messageBox = new LockFreeArrayQueue<NodeMessage *>(100);
}

Node::~Node()
{
    handle_message = false;
    NodeMessage *p;
    while (!messageBox->isEmpty())
    {
        messageBox->pop(p);
        delete p;
    }

    delete messageBox;
}

void Node::direct_call(NodeMessage message)
{
    if (handle_message)
    {
        auto mp = new NodeMessage(message);
        messageBox->push(mp);
    }
}

void Node::run_once()
{
    this_cycle_cmd_count = limit;
    while (!messageBox->isEmpty())
    {
        this_cycle_cmd_count--;
        NodeMessage *p;
        messageBox->pop(p);
        auto sp = shared_ptr<NodeMessage>(p);
        if (sp->type == NodeMessageType::Call)
        {
            run_func(create_func(sp));
        }
        else
        {
            auto wf = waitting_callback.find(sp->id);
            if (wf != waitting_callback.end())
            {
                auto f = wf->second;
                waitting_callback.erase(sp->id);
                f->send_callback(sp->args[0]);
            }
        }
    }
    while (this_cycle_cmd_count > 0 && !run_env.empty())
    {
        auto cur_func = run_env.front();
        run_env.pop_front();
        cur_func->run(this_cycle_cmd_count);
        if (!cur_func->completed)
            run_env.push_back(cur_func);
        else
        {
            auto nodemsg = new NodeMessage();
            nodemsg->callbackNode = cur_func->callback_node;
            nodemsg->id = cur_func->id;
            nodemsg->type = NodeMessageType::Callback;
            nodemsg->args.push_back(*(cur_func->ret->copy()));
            engine->SendMessage(nodemsg);
        }
    }
}
shared_ptr<FuncEnv> Node::create_func(shared_ptr<NodeMessage> p)
{
    func_index++;

    auto fc = code_page->funcs.find(p->name);
    auto fr = make_shared<FuncEnv>();
    fr->init(fc->second, p);
    fr->id = func_index;
    fr->node = this;
    fr->callback_id = p->id;
    fr->callback_node = p->callbackNode;
    return fr;
}
void Node::run_func(shared_ptr<FuncEnv> fr)
{
    run_env.push_back(fr);
}
#endif