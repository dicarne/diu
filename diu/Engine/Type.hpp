#ifndef TYPE_H_
#define TYPE_H_
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <sstream>
#include "runtime_error.hpp"
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::unordered_map;
using std::variant;
using std::vector;
typedef unsigned long long int64;
struct PID
{
    int engine = 0;
    int64 pid = 0;
    int server = 0;
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
    Null, // !
    Pid,  // !
    Int,  // !
    Num,  // !
    Bool,
    Map,
    Array,
    Str,        // !
    Struct,     // !
    TypeSymbol, // !
    Await
};

class Object;
typedef variant<int, PID, double, bool, string, shared_ptr<vector<string>>, vector<shared_ptr<Object>>, shared_ptr<Object>, unordered_map<string, shared_ptr<Object>>> var;
class FuncEnv;
class Object
{
private:
    /* data */
public:
    var value;
    ObjectRawType type;
    shared_ptr<unordered_map<string, shared_ptr<Object>>> table;
    Object(ObjectRawType type)
    {
        this->type = type;
        if (type == ObjectRawType::Struct)
        {
            table = make_shared<unordered_map<string, shared_ptr<Object>>>();
        }
        else
        {
            table = nullptr;
        }
        if (type == ObjectRawType::TypeSymbol)
        {
            value = make_shared<vector<string>>();
        }
    }
    Object(double d)
    {
        this->type = ObjectRawType::Num;
        value = d;
    }
    Object(bool d)
    {
        this->type = ObjectRawType::Num;
        value = d ? 1 : 0;
    }
    Object(int d)
    {
        this->type = ObjectRawType::Num;
        value = double(d);
    }
    Object(PID pid)
    {
        this->type = ObjectRawType::Pid;
        value = pid;
    }
    Object(string str)
    {
        this->type = ObjectRawType::Str;
        value = str;
    }
    void set(ObjectRawType type, var value)
    {
        this->type = type;
        this->value = value;
    }
    void set(shared_ptr<Object> obj)
    {
        this->type = obj->type;
        this->value = obj->value;
        this->table = obj->table;
    }
    Object(const Object &ano)
    {
        value = ano.value;
        type = ano.type;
        this->table = ano.table;
    }
    static shared_ptr<Object> make_await(int async_index)
    {
        auto o = make_shared<Object>(ObjectRawType::Await);
        o->value = async_index;
        return o;
    }
    static Object make_await_o(int async_index)
    {
        auto o = Object(ObjectRawType::Await);
        o.value = async_index;
        return o;
    }

    shared_ptr<Object> copy()
    {
        auto p = make_shared<Object>(type);
        switch (type)
        {
        case ObjectRawType::Num:
            p->value = std::get<double>(value);
            break;
        case ObjectRawType::Bool:
            p->value = std::get<double>(value);
            break;
        case ObjectRawType::Str:
            p->value = std::get<string>(value);
            break;
        case ObjectRawType::Pid:
            p->value = std::get<PID>(value);
            break;
        case ObjectRawType::Struct:
        {
            for (auto &kv : (*table))
            {
                (*p->table)[kv.first] = kv.second->copy();
            }
        }
        break;
        case ObjectRawType::TypeSymbol:
        {
            auto symbols = std::get<shared_ptr<vector<string>>>(value);
            auto newsymbols = std::get<shared_ptr<vector<string>>>(p->value);
            for (auto &s : *symbols)
            {
                newsymbols->push_back(s);
            }
        }
        break;
        default:
            break;
        }
        return p;
    }

    bool as_bool()
    {
        if (type == ObjectRawType::Num)
        {
            if (std::get<double>(value) == 0)
                return false;
        }
        return true;
    }

    string to_string()
    {
        stringstream ss;
        switch (type)
        {
        case ObjectRawType::Num:
        {
            auto v = std::get<double>(value);
            ss << v;
        }
        break;
        case ObjectRawType::Bool:
        {
            auto v = std::get<double>(value);
            ss << v;
        }
        break;
        case ObjectRawType::Str:
        {
            auto v = std::get<string>(value);
            ss << v;
        }
        break;
        case ObjectRawType::Pid:
        {
            auto v = std::get<PID>(value);
            ss << "[PID] " << v.pid << "  server: " << v.server;
        }
        break;
        case ObjectRawType::Struct:
        {
            ss << "[STRUCT]";
        }
        break;
        case ObjectRawType::TypeSymbol:
        {
            auto symbols = std::get<shared_ptr<vector<string>>>(value);
            for (auto &s : *symbols)
            {
                ss << s << ".";
            }
        }
        break;
        default:
            break;
        }
        return ss.str();
    }

    shared_ptr<Object> get_child(string name)
    {
        if (type != ObjectRawType::Struct)
        {
            throw runtime_error("Can't get child in a simple value");
        }
        auto f = table->find(name);
        if (f == table->end())
        {
            throw runtime_error("Can't find this child: " + name);
        }
        return f->second;
    }

    void symbol_find(string name)
    {
        if (type != ObjectRawType::TypeSymbol)
        {
            throw runtime_error("Can't find symbol in a simple value");
        }
        auto chain = std::get<shared_ptr<vector<string>>>(value);
        chain->push_back(name);
    }

    template <typename T>
    T getv()
    {
        return std::get<T>(this->value);
    }

    static shared_ptr<Object> add(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(std::get<double>(a->value) + std::get<double>(b->value));
        }
        if (a->type == ObjectRawType::Str && b->type == ObjectRawType::Str)
        {
            return make_shared<Object>(std::get<string>(a->value) + std::get<string>(b->value));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> sub(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(std::get<double>(a->value) - std::get<double>(b->value));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> mul(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(std::get<double>(a->value) * std::get<double>(b->value));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> div(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(std::get<double>(a->value) / std::get<double>(b->value));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> mod(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(double(int(std::get<double>(a->value)) % int(std::get<double>(b->value))));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> equ(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(double(std::get<double>(a->value) == std::get<double>(b->value) ? 1 : 0));
        }
        if (a->type == ObjectRawType::Str && b->type == ObjectRawType::Str)
        {
            return make_shared<Object>(double(std::get<string>(a->value) == std::get<string>(b->value) ? 1 : 0));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> neq(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(double(std::get<double>(a->value) != std::get<double>(b->value) ? 1 : 0));
        }
        if (a->type == ObjectRawType::Str && b->type == ObjectRawType::Str)
        {
            return make_shared<Object>(double(std::get<string>(a->value) != std::get<string>(b->value) ? 1 : 0));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> le(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(double(std::get<double>(a->value) <= std::get<double>(b->value) ? 1 : 0));
        }
        if (a->type == ObjectRawType::Str && b->type == ObjectRawType::Str)
        {
            return make_shared<Object>(double(std::get<string>(a->value) <= std::get<string>(b->value) ? 1 : 0));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> lt(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(double(std::get<double>(a->value) < std::get<double>(b->value) ? 1 : 0));
        }
        if (a->type == ObjectRawType::Str && b->type == ObjectRawType::Str)
        {
            return make_shared<Object>(double(std::get<string>(a->value) < std::get<string>(b->value) ? 1 : 0));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> ge(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(double(std::get<double>(a->value) >= std::get<double>(b->value) ? 1 : 0));
        }
        if (a->type == ObjectRawType::Str && b->type == ObjectRawType::Str)
        {
            return make_shared<Object>(double(std::get<string>(a->value) >= std::get<string>(b->value) ? 1 : 0));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static shared_ptr<Object> gt(FuncEnv *ctx, shared_ptr<Object> a, shared_ptr<Object> b)
    {
        if (a->type == ObjectRawType::Num && b->type == ObjectRawType::Num)
        {
            return make_shared<Object>(double(std::get<double>(a->value) > std::get<double>(b->value) ? 1 : 0));
        }
        if (a->type == ObjectRawType::Str && b->type == ObjectRawType::Str)
        {
            return make_shared<Object>(double(std::get<string>(a->value) > std::get<string>(b->value) ? 1 : 0));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
};
#endif