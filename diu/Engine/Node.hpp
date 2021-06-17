#ifndef TYPE_H
#define TYPE_H

#include <queue>
#include <string>
#include <vector>
#include "Type.hpp"
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
    Node(PID pid);
    ~Node();
};

Node::Node(PID pid) : Pid(pid)
{
}

Node::~Node()
{
}
#endif