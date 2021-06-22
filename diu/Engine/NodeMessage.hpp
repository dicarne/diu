#ifndef NODE_MESSAGE_H_
#define NODE_MESSAGE_H_
#include <string>
#include <vector>
using std::string;
using std::vector;
#include "Type.hpp"
#include "Object/object.h"

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
    int async_ = 0;
    PID callbackNode;
    vector<Object> args;
    //NodeMessage() {}
    NodeMessage(NodeMessageType type, string name, int id, PID callbackNode, int async_)
        : type(type), name(name), id(id), callbackNode(callbackNode), async_(async_)
    {
    }
};

#endif