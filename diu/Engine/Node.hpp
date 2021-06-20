#ifndef TYPE_H
#define TYPE_H

#include <queue>
#include <string>
#include <vector>
#include "Type.hpp"
#include "codes/CodeNode.hpp"
using std::queue;
using std::string;
using std::vector;

class NodeMessage
{
public:
    string name;
    int id;
    PID callbackNode;
    vector<Object> args;
};

class Node
{

private:
    queue<NodeMessage> messageBox;

public:
    PID Pid;
    Node();
    ~Node();
    shared_ptr<CodeNode> code_page;
    void call(NodeMessage message);
};

Node::Node()
{
}

Node::~Node()
{
}

void Node::call(NodeMessage message)
{
}
#endif