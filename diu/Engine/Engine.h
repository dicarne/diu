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
    int64 node_index = 0;
    void AddNewNode(shared_ptr<Node> node);

public:
    Engine(int version);
    ~Engine();
    shared_ptr<CodeEngine> codes;
    void Run(string mod, string node, string func);
    void Run(string mod, string node, string func, shared_ptr<NodeMessage> message);
    shared_ptr<Node> NewNode(string mod, string node);
    void SendMessage(NodeMessage *msg);
    void RunCode();
    void load(string byecode_file);
};

#endif