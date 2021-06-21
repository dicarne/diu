#ifndef NODE_MESSAGE_H_
#define NODE_MESSAGE_H_
#include <string>
#include <vector>
using std::string;
using std::vector;
#include "Type.hpp"

enum class NodeMessageType
{
    Call,
    Callback
};
struct NodeMessage
{
    NodeMessageType type = NodeMessageType::Call;
    string name = "";
    int id = 0;
    bool noreply = false;
    PID callbackNode;
    vector<Object> args;
    //NodeMessage() {}
    NodeMessage(NodeMessageType type, string name, int id, PID callbackNode, bool noreply)
        : type(type), name(name), id(id), callbackNode(callbackNode), noreply(noreply)
    {
    }
};

#endif