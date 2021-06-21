#ifndef ENGINE_H_
#define ENGINE_H_
#include <unordered_map>
#include <memory>

#include "../conf.h"
#include "runtime_error.hpp"

#include "RemoteEngine.h"
#include "Node.h"
#include "codes/CodeEngine.hpp"
#include "../compiler/bytecode/bytecode_reader.hpp"
#include <list>
using std::list;
using std::make_shared;
using std::shared_ptr;
using std::unordered_map;

class Node;
class Engine
{
private:
    unordered_map<int, RemoteEngine> engines;
    int version;
    unordered_map<int64, shared_ptr<Node>> nodes;
    unordered_map<string, unordered_map<string, shared_ptr<Node>>> static_node;
    int64 node_index = 0;
    void AddNewNode(shared_ptr<Node> node);
    unordered_map<int64, shared_ptr<Node>> running;
    //unordered_map<int64, shared_ptr<Node>> completed;
public:
    Engine(int version);
    ~Engine();
    shared_ptr<CodeEngine> codes;
    void Run(string mod, string node, string func, bool noreply);
    void Run(string mod, string node, string func, shared_ptr<NodeMessage> message);
    void Run(PID node, shared_ptr<NodeMessage> msg);
    shared_ptr<Node> NewNode(string mod, string node);
    void SendMessage(NodeMessage *msg);
    void RunCode();
    void load(string byecode_file);
    void ActiveNode(PID pid);
};

#endif