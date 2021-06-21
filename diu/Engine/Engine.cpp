#include "Engine.h"

Engine::Engine(int version)
{
    this->version = version;
    this->codes = make_shared<CodeEngine>();
    ;
}

Engine::~Engine()
{
}

void Engine::AddNewNode(shared_ptr<Node> node)
{
    node_index++;
    node->Pid = PID(version, node_index);
    nodes[node->Pid.pid] = node;
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
    AddNewNode(node);
    node->engine = this;
    node->load_code(codenode->second);
    node->init();
    return node;
}

void Engine::Run(string mod, string node, string func)
{
    auto nodei = NewNode(mod, node);
    NodeMessage m;
    m.name = func;
    m.id = 0;
    m.callbackNode = PID(0, 0);
    nodei->direct_call(m);
}
void Engine::Run(string mod, string node, string func, shared_ptr<NodeMessage> message)
{

    if (message->name == "new")
    {
        auto nodei = NewNode(mod, node);
        auto reply = new NodeMessage();
        reply->type = NodeMessageType::Callback;
        reply->id = message->id;
        reply->callbackNode = message->callbackNode;
        reply->name = message->name;
        reply->args.push_back(Object(nodei->Pid));
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
        NodeMessage m;
        m.name = message->name;
        m.id = message->id;
        m.callbackNode = message->callbackNode;
        m.args = message->args;
        n->direct_call(m);
    }
}
void Engine::Run(PID node, shared_ptr<NodeMessage> msg)
{
    auto find_n = nodes.find(node.pid);
    if (find_n != nodes.end())
    {
        auto reply = new NodeMessage();
        reply->type = NodeMessageType::Call;
        reply->id = msg->id;
        reply->callbackNode = msg->callbackNode;
        reply->name = msg->name;
        reply->args = msg->args;
        find_n->second->messageBox->push(reply);
    }
}
void Engine::SendMessage(NodeMessage *msg)
{
    auto cb = msg->callbackNode;
    auto find_n = nodes.find(cb.pid);
    if (find_n != nodes.end())
    {
        find_n->second->messageBox->push(msg);
    }
}

void Engine::RunCode()
{
    int index = 10;
    while (index-- > 0)
    {
        for (auto &kv : nodes)
        {
            kv.second->run_once();
        }
    }
}

void Engine::load(string byecode_file)
{
    bytecode_reader br(byecode_file);
    br.read_all(this->codes);
}