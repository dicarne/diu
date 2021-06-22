#include "object.h"
shared_ptr<Object> Object::copy()
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
        p->value = std::get<shared_ptr<json>>(value);
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

void Object::set_child(string name, shared_ptr<Object> value)
{
    if (type != ObjectRawType::Struct)
    {
        throw runtime_error("Can't get child in a simple value");
    }
    auto j = getv<shared_ptr<json>>();
    j->data[name] = value->type == ObjectRawType::Struct ? value : value->copy();
}

shared_ptr<Object> Object::get_child(string name)
{
    if (type != ObjectRawType::Struct)
    {
        throw runtime_error("Can't get child in a simple value");
    }
    auto j = getv<shared_ptr<json>>();
    auto f = j->data.find(name);
    if (f == j->data.end())
    {
        throw runtime_error("Can't find this child: " + name);
    }
    return f->second;
}

string Object::to_string()
{
    stringstream ss;

    switch (type)
    {
    case ObjectRawType::Num:
        ss << getv<double>();
        break;
    case ObjectRawType::Bool:

        break;
    case ObjectRawType::Str:
        ss << "\"" << getv<string>() << "\"";
        break;
    case ObjectRawType::Pid:
    {
        auto v = std::get<PID>(value);
        ss << "\""
           << "[PID] " << v.pid << "  server: " << v.server << "\"";
    }
    break;
    case ObjectRawType::Struct:
    {
        ss << "{";
        auto j = getv<shared_ptr<json>>();
        for (auto &kv : j->data)
        {
            ss << "\"" << kv.first << "\""
               << ":" << kv.second->to_string();
        }
        ss << "}";
        break;
    }
    break;
    case ObjectRawType::TypeSymbol:
    {
        auto symbols = std::get<shared_ptr<vector<string>>>(value);
        ss << "\"";
        for (auto &s : *symbols)
        {
            ss << s << ".";
        }
        ss << "\"";
    }
    break;
    default:
        break;
    }
    return ss.str();
}
