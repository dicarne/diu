#include "Node.h"
#include "FuncEnv.h"

Node::Node()
{
    messageBox = new LockFreeArrayQueue<NodeMessage *>(100);
    handle_message = true;
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
            nodemsg->id = cur_func->callback_id;
            nodemsg->type = NodeMessageType::Callback;
            nodemsg->args.push_back(*(cur_func->ret->copy()));
            if (nodemsg->callbackNode.pid == 0)
            {
                // LOCAL
                auto fw = waitting_callback.find(nodemsg->id);
                if (fw != waitting_callback.end())
                {
                    fw->second->handle_callback(*(cur_func->ret->copy()));
                }
                waitting_callback.erase(nodemsg->id);
                delete nodemsg;
            }
            else
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

 void Node::call_another_func(FuncEnv *caller, shared_ptr<Object> symbol, shared_ptr<NodeMessage> msg)
    {
        auto chain = std::get<shared_ptr<vector<string>>>(symbol->value);
        if (chain->size() == 1 && (*chain)[0] == "sys")
        {
            if (msg->name == "print")
            {
                std::cout << msg->args[0].to_string() << std::endl;
            }
            std::cout << msg->name << std::endl;
            // TODO:
            caller->waitting = false;
            caller->ret = make_shared<Object>(0);
        }
    }