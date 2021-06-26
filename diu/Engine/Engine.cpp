#include "Engine.h"

RunEngine::RunEngine()
{
    running = make_shared<LockFreeArrayQueue<Node *>>(10000);
}

void RunEngine::run_once()
{
    while (true && alive)
    {
        if (!running->isEmpty())
        {
            Node *node;
            running->pop(node);
            node->run_once();
            running->push(node);
        }
    }
}

class NodeExec
{
public:
    shared_ptr<RunEngine> node;
    NodeExec(shared_ptr<RunEngine> node) : node(node) {}
    void run()
    {
        node->run_once();
    }
};

Engine::Engine()
{
    //this->version = version;
    this->codes = make_shared<CodeEngine>();
    auto thread_count = 4;
    this->thread_pool = make_shared<ThreadPool>(thread_count);
    this->thread_pool->init();

    for (auto i = 0; i < thread_count; i++)
    {
        auto e = make_shared<RunEngine>();
        run_engines.push_back(e);
        NodeExec exec(e);
        thread_pool->submit(std::mem_fn(&NodeExec::run), exec);
    }
}

double Engine::get_version() { return engine_version_get(); }

Engine::~Engine()
{
}

void Engine::AddNewNode(shared_ptr<Node> node)
{
    node->active = false;
    node_index++;
    node->Pid = PID(version, node_index);
    nodes[node->Pid.pid] = node;
    run_engines[engine_index]->running->push(node.get());
    engine_index = (engine_index + 1) % run_engines.size();
    //running[node->Pid.pid] = node;
}

shared_ptr<Node> Engine::NewNode(string mod, string node_name)
{
    auto codemod = codes->modules.find(mod);
    if (codemod == codes->modules.end())
        throw runtime_error("can't find module " + mod);
    auto codenode = codemod->second->node_to_code_page.find(node_name);
    if (codenode == codemod->second->node_to_code_page.end())
        throw runtime_error("can't find node " + node_name + " in module " + mod);
    auto node = make_shared<Node>();
    node->engine = this;
    node->load_code(codenode->second);
    AddNewNode(node);
    node->init();
    return node;
}

void Engine::Run(string mod, string node, string func)
{
    auto nodei = NewNode(mod, node);
    auto newm = make_shared<NodeMessage>(NodeMessageType::Call, "new", 0, PID(0, 0), false);
    Run(nodei->Pid, newm);
    auto m = make_shared<NodeMessage>(NodeMessageType::Call, func, 0, PID(0, 0), false);
    Run(nodei->Pid, m);
    //nodei->direct_call(m);
}
void Engine::Run(string mod, string node, string func, shared_ptr<NodeMessage> message)
{
    if (message->name == "new")
    {
        auto nodei = NewNode(mod, node);
        Run(nodei->Pid, message);
    }
    else
    {
        // try find old:
        auto old_module = static_node.find(mod);
        if (old_module == static_node.end())
        {
            if (codes->modules.find(mod) != codes->modules.end())
            {
                static_node[mod] = unordered_map<string, shared_ptr<Node>>();
            }
            else
            {
                throw runtime_error("Can't find module: " + mod);
            }
        }
        auto &module_collection = old_module != static_node.end() ? old_module->second : static_node[mod];
        auto node_static_f = module_collection.find(node);
        if (node_static_f == module_collection.end())
        {
            module_collection[node] = NewNode(mod, node);
        }
        auto n = node_static_f != module_collection.end() ? node_static_f->second : module_collection[node];
        auto m = new NodeMessage(NodeMessageType::Call, message->name, message->id, message->callbackNode, message->async_);
        for (auto &a : message->args)
        {
            m->args.push_back(*(a.clone()));
        }
        n->push_massage(m);
    }
}

void Engine::Run(PID node, shared_ptr<NodeMessage> msg)
{
    auto find_n = nodes.find(node.pid);
    if (find_n != nodes.end())
    {
        auto reply = new NodeMessage(NodeMessageType::Call, msg->name, msg->id, msg->callbackNode, msg->async_);
        reply->args = msg->args;
        find_n->second->push_massage(reply);
    }
}

void Engine::SendMessage(NodeMessage *msg)
{
    auto cb = msg->callbackNode;
    auto find_n = nodes.find(cb.pid);
    if (find_n != nodes.end())
    {
        find_n->second->push_massage(msg);
    }
}

void Engine::RunCode()
{
    Run(main_module, main_node, main_func);
    thread_pool->shutdown();
    // int index = 10;
    // bool all_complete = false;
    // while (!all_complete)
    // {
    //     //all_complete = true;
    //     for (auto &kv : running)
    //     {
    //         if (kv.second->active)
    //         {
    //             NodeExec exec(kv.second);
    //             thread_pool->submit(std::mem_fn(&NodeExec::run), exec);
    //         }
    //         //kv.second->run_once();
    //         //all_complete &= !kv.second->active;
    //     }
    // }
}

void Engine::Load(string byecode_file)
{
    bytecode_reader br(byecode_file);
    br.read_all(this->codes);
}

void Engine::ActiveNode(PID pid)
{
}

#include <fstream>
void Engine::Config(string config_file_path)
{
    std::stringstream ss;
    std::ifstream fs(config_file_path);
    while (!fs.eof())
    {
        auto c = fs.get();
        if (c == -1)
            break;
        ss << char(c);
    }
    auto s = ss.str();
    auto conf = json::from(s);
    auto main = conf->get_child("main");
    if (main->as_bool())
    {
        main_module = main->get_child("module")->get_str_or_default("__GLOBAL__");
        main_node = main->get_child("node")->get_str_or_default("Main");
        main_func = main->get_child("func")->get_str_or_default("main");
    }
}

void Engine::Exit()
{
    for (auto &r : run_engines)
    {
        r->alive = false;
    }
}