#ifndef NODE_MESSAGE_H_
#define NODE_MESSAGE_H_
#include <string>
#include <vector>
#include <sstream>
using std::string;
using std::stringstream;
using std::vector;
#include "Type.hpp"
#include "Object/object.h"

enum class NodeMessageType : unsigned char
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

    stringstream serialize();
    static NodeMessage *deserialize(stringstream &ss);

private:
    void _serialize(Object *o, stringstream &ss);
    static Object::Ptr _deserialize(stringstream &ss);
    template <typename T>
    static T read(stringstream &stream)
    {
        char buffer[16];
        stream.read(buffer, sizeof(T));
        return *(reinterpret_cast<T *>(buffer));
    }

    static string read_str(stringstream &ss)
    {
        auto size = read<std::size_t>(ss);
        char *buffer = new char[size + 1];
        ss.read(buffer, size + 1);
        string s(buffer);
        delete[] buffer;
        return s;
    }
};

#endif