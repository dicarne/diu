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
        auto mp = new NodeMessage(message.type, message.name, message.id, message.callbackNode, message.async_);
        for (auto &a : message.args)
        {
            mp->args.push_back(*(a.copy()));
        }

        messageBox->push(mp);
        active = true;
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
            if (sp->name == "new")
            {
                auto fc = code_page->funcs.find(p->name);
                if (fc != code_page->funcs.end() && fc->second != nullptr)
                {
                    auto newfun = create_func(code_page, sp);
                    run_env.push_back(newfun);
                }
                else
                {
                    auto nodemsg = new NodeMessage(NodeMessageType::Callback, "", sp->id, PID(sp->callbackNode), sp->async_);
                    nodemsg->args.push_back(Object(Pid));
                    engine->SendMessage(nodemsg);
                }
            }
            else
            {
                auto f = create_func(code_page, sp);
                run_func(f);
            }
        }
        else
        {
            if (!sp->async_)
            {
                auto wf = waitting_callback.find(sp->id);
                if (wf != waitting_callback.end())
                {
                    auto f = wf->second;
                    waitting_callback.erase(sp->id);
                    f->handle_callback(sp->args[0]);
                }
            }
            else
            {
                // TODO: 维护一个id到函数的映射表，通过查找加速！在完成函数后删除！
                for (auto &f : run_env)
                {
                    if (f->id == sp->id)
                    {
                        f->handle_async_callback(sp->args[0], sp->async_);
                        break;
                    }
                }
            }
        }
    }
    list<shared_ptr<FuncEnv>> waitting;
    while (this_cycle_cmd_count > 0 && !run_env.empty())
    {
        auto cur_func = run_env.front();
        run_env.pop_front();
        cur_func->run(this_cycle_cmd_count);
        if (!cur_func->completed)
        {
            if (cur_func->waitting)
                waitting.push_back(cur_func);
            else
                run_env.push_back(cur_func);
        }
        else
        {
            auto nodemsg = new NodeMessage(NodeMessageType::Callback, "", cur_func->callback_id, cur_func->callback_node, cur_func->async_call);
            if (cur_func->name == "new")
            {
                nodemsg->args.push_back(Object(Pid));
            }
            else
            {
                nodemsg->args.push_back(*(cur_func->ret->clone()));
            }

            if (nodemsg->callbackNode.pid == 0)
            {
                if (nodemsg->async_ == 0)
                {
                    auto fw = waitting_callback.find(nodemsg->id);
                    if (fw != waitting_callback.end())
                    {
                        fw->second->handle_callback(*(cur_func->ret->copy()));
                    }
                    delete nodemsg;
                }
                else
                {
                    for (auto &f : run_env)
                    {
                        if (f->id == nodemsg->id)
                        {
                            f->handle_async_callback(nodemsg->args[0], Object::make_await_o(cur_func->async_call));
                            break;
                        }
                    }
                }
                // LOCAL
            }
            else
                engine->SendMessage(nodemsg);
        }
    }
    run_env.splice(run_env.begin(), waitting);
    if (run_env.empty() && messageBox->isEmpty())
    {
        active = false;
    }
}

shared_ptr<FuncEnv> Node::create_func(shared_ptr<CodeNode> cp, shared_ptr<NodeMessage> p)
{
    func_index++;

    auto fc = cp->funcs.find(p->name);
    if (fc == cp->funcs.end())
    {
        throw runtime_error("Can not find func: " + p->name + " in " + cp->name);
    }
    auto fr = make_shared<FuncEnv>();
    fr->id = func_index;
    fr->node = this;
    fr->callback_id = p->id;
    fr->callback_node = p->callbackNode;
    fr->async_call = p->async_;
    fr->init(fc->second, p);
    return fr;
}

void Node::run_func(shared_ptr<FuncEnv> fr)
{
    run_env.push_back(fr);
}

void Node::call_another_func(FuncEnv *caller, Object::Ptr symbol, shared_ptr<NodeMessage> msg)
{
    if (symbol->type == ObjectRawType::TypeSymbol)
    {
        auto chain = std::get<shared_ptr<vector<string>>>(symbol->value);
        msg->callbackNode = Pid;
        if (chain->size() == 1 && (*chain)[0] == "sys")
        {
            if (msg->name == "print")
            {
                std::cout << msg->args[0].to_string() << std::endl;
            }
            if (msg->name == "PID")
            {
                caller->waitting = false;
                caller->ret = make_shared<Object>(Pid);
                return;
            }
            if (msg->name == "exit")
            {
                caller->ret = make_shared<Object>(0);
                engine->Exit();
                return;
            }
            caller->waitting = false;
            caller->ret = make_shared<Object>(0);
            return;
        }
        if (chain->size() == 1 && (*chain)[0] == "O")
        {
            if (msg->name == "new")
            {
                caller->waitting = false;
                caller->ret = make_shared<Object>(ObjectRawType::Struct);
                return;
            }
            caller->waitting = false;
            caller->ret = make_shared<Object>(0);
            return;
        }

        // Remote call
        auto v = msg->args;
        msg->args = vector<Object>();
        for (auto &iv : v)
        {
            msg->args.push_back(*(iv.clone()));
        }
        if (chain->size() == 1)
        {

            auto mod = caller->code->mod;
            auto another_node = mod->nodes->find((*chain)[0]);
            if (another_node != mod->nodes->end())
            {
                if (!msg->async_)
                    waitting_callback[msg->id] = caller;

                engine->Run(mod->module_name, (*chain)[0], msg->name, msg);
                return;
            }
            else
            {
                auto pkg = mod->outer_symbol_pkg_map->find((*chain)[0]);
                if (pkg != mod->outer_symbol_pkg_map->end())
                {
                    auto pkgname = pkg->second;
                    chain->insert(chain->begin(), pkgname);
                }
                else
                {
                    throw runtime_error("Remote call error here");
                }
            }
        }
        if (chain->size() == 2)
        {
            if (!msg->async_)
                waitting_callback[msg->id] = caller;

            auto mod = caller->code->mod;
            auto pkg = (*chain)[0];

            auto modules = engine->codes->modules;
            auto fmod = modules.find(pkg);
            if (fmod != modules.end())
            {
                auto try_find_node = fmod->second->node_to_code_page.find((*chain)[1]);
                if (try_find_node != fmod->second->node_to_code_page.end() &&
                    try_find_node->second->mod->engine_version == engine->get_version() &&
                    try_find_node->second->funcs[msg->name] && try_find_node->second->funcs[msg->name]->is_static)
                {
                    run_func(create_func(try_find_node->second, msg));
                }
                else
                {
                    if (msg->name != "new")
                        throw runtime_error("Can't run dynamic function without an instance: " + pkg);
                    engine->Run(pkg, (*chain)[1], msg->name, msg);
                }
                return;
            }
            else
            {
                throw runtime_error("Can't find module pkg: " + pkg);
            }
        }
    }
    else if (symbol->type == ObjectRawType::Pid)
    {
        if (!msg->async_)
            waitting_callback[msg->id] = caller;
        msg->callbackNode = Pid;
        engine->Run(symbol->getv<PID>(), msg);
        return;
    }
    else
    {
        throw runtime_error("Unknown func call method!");
    }
}
void Node::call_another_func(FuncEnv *caller, shared_ptr<NodeMessage> msg)
{
    // auto fun = msg->name;
    // auto ff= code_page->funcs.find(fun);
    // if(ff==code_page->funcs.end()){
    //     // Remote call
    //     call_another_func(caller, )
    // }
    // Local Call
    auto f = create_func(caller->code->node, msg);
    if (!msg->async_)
        waitting_callback[msg->id] = caller;
    else
    {
        f->async_call = msg->async_;
    }
    run_func(f);
}

void Node::push_massage(NodeMessage *msg)
{
    messageBox->push(msg);
    if (active == false)
    {
        active = true;
        engine->ActiveNode(Pid);
    }
}
