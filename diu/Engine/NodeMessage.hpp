#ifndef NODE_MESSAGE_H_
#define NODE_MESSAGE_H_
#include <string>
#include <vector>
using std::string;
using std::vector;
#include "Type.hpp"

enum class NodeMessageType {
    Call, Callback
};
struct NodeMessage
{
    NodeMessageType type;
    string name;
    int id;
    PID callbackNode;
    vector<Object> args;
};

#endif