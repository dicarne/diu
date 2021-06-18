#ifndef TYPE_H_
#define TYPE_H_
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <variant>
using std::shared_ptr;
using std::string;
using std::variant;
using std::unordered_map;
using std::vector;
typedef unsigned long long int64;
class PID
{
private:
    /* data */
public:
    int engine;
    int64 pid;
    int server;
    PID() : engine(-1), pid(-1) {}
    PID(int engine, int64 pid) : engine(engine), pid(pid)
    {
    }
};

class Type
{
public:
    std::string name;
    shared_ptr<Type> parent = nullptr;
};

class TypeManager
{
public:
    unordered_map<string, shared_ptr<Type>> types;
    void reg_type(Type type, string parent)
    {
        if (parent != "")
        {
            auto par = types.find(parent);
            if (par == types.end())
            {
                auto p = std::make_shared<Type>();
                p->name = parent;
                types[parent] = p;

                auto my = types.find(type.name);
                if (my == types.end())
                {
                    auto self = std::make_shared<Type>(type);
                    self->parent = p;
                    types[type.name] = self;
                }
                else
                {
                    my->second->parent = p;
                }
            }
            else
            {
                auto my = types.find(type.name);
                if (my == types.end())
                {
                    auto p = std::make_shared<Type>(type);
                    p->parent = par->second;
                    types[type.name] = p;
                }
                else
                {
                    my->second->parent = par->second;
                }
            }
        }
        else
        {
            auto my = types.find(type.name);
            if (my == types.end())
            {
                types[type.name] = std::make_shared<Type>(type);
            }
        }
    }

    shared_ptr<Type> find_type(string type_name)
    {
        return types[type_name];
    }
};

enum ObjectRawType
{
    Null,
    Pid,
    Int,
    Num,
    Bool,
    Map,
    Array,
    Str,
    Struct
};

class Object;
typedef variant<int, PID, double, bool, string, vector<shared_ptr<Object>>, shared_ptr<Object>, unordered_map<string, shared_ptr<Object>>> var;

class Object
{
private:
    /* data */
public:
    var value;
    ObjectRawType type;
    Object(ObjectRawType type)
    {
        this->type = type;
    }
    void Set(ObjectRawType type, var value)
    {
        this->type = type;
        this->value = value;
    }
};
#endif