#include "object.h"
Object::Ptr Object::copy()
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
        auto &newsymbols = std::get<shared_ptr<vector<string>>>(p->value);
        for (auto s : *symbols)
        {
            newsymbols->push_back(s);
        }
    }
    break;
    case ObjectRawType::Array:
    {
        p->value = std::get<shared_ptr<var_array_value>>(value);
    }
    break;
    case ObjectRawType::Await:
    {
        p->value = std::get<int>(value);
    }
    break;
    default:
        break;
    }
    return p;
}

void Object::set_child(string name, Object::Ptr value)
{
    if (type == ObjectRawType::Null)
    {
        type == ObjectRawType::Struct;
        this->value = make_shared<json>();
    }
    if (type != ObjectRawType::Struct)
    {
        throw runtime_error("Can't get child in a simple value");
    }
    auto j = getv<shared_ptr<json>>();
    j->data[name] = value->type == ObjectRawType::Struct ? value : value->copy();
}

Object::Ptr Object::get_child(string name)
{
    if (type == ObjectRawType::Null)
    {
        type == ObjectRawType::Struct;
        this->value = make_shared<json>();
    }
    if (type != ObjectRawType::Struct)
    {
        throw runtime_error("Can't get child in a simple value");
    }
    auto j = getv<shared_ptr<json>>();
    auto f = j->data.find(name);
    if (f == j->data.end())
    {
        auto n = make_shared<Object>(ObjectRawType::Struct);
        j->data[name] = n;
        return n;
    }
    return f->second;
}

Object::Ptr Object::get_child(double name)
{
    if (type != ObjectRawType::Array)
    {
        throw runtime_error("Can't get element outside array");
    }
    auto j = getv<shared_ptr<var_array_value>>();
    auto i = int(name);
    if (i >= j->size())
    {
        throw runtime_error("array: out of range!");
    }
    return (*j)[i];
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
        ss << "\"" << encode_string(getv<string>()) << "\"";
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
        bool add_com = false;
        for (auto &kv : j->data)
        {
            if (add_com)
                ss << ",";
            ss << "\"" << kv.first << "\""
               << ":" << kv.second->to_string();
            add_com = true;
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
    case ObjectRawType::Array:
    {
        ss << "[";
        auto arr = getv<shared_ptr<var_array_value>>();
        for (auto i = 0; i < arr->size(); i++)
        {
            if (i != arr->size() - 1)
            {
                ss << (*arr)[i]->to_string();
                ss << ",";
            }
            else
            {
                ss << (*arr)[i]->to_string();
            }
        }

        ss << "]";
    }
    break;
    case ObjectRawType::Null:
    {
        ss << "null";
    }
    break;
    default:
        break;
    }
    return ss.str();
}

string Object::encode_string(string str)
{
    stringstream ss;
    for (auto s : str)
    {
        if (s == '"')
        {
            ss << '\\' << '"';
        }
        else
        {
            ss << s;
        }
    }
    return ss.str();
}

Object::Ptr Object::clone()
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
        p->value = std::get<shared_ptr<json>>(value)->clone();
    }
    break;
    case ObjectRawType::TypeSymbol:
    {
        auto symbols = std::get<shared_ptr<vector<string>>>(value);
        auto &newsymbols = std::get<shared_ptr<vector<string>>>(p->value);
        for (auto s : *symbols)
        {
            newsymbols->push_back(s);
        }
    }
    break;
    case ObjectRawType::Array:
    {
        auto arr = getv<shared_ptr<var_array_value>>();
        auto pv = p->getv<shared_ptr<var_array_value>>();
        for (auto i = 0; i < arr->size(); i++)
        {
            pv->push_back((*arr)[i]->clone());
        }
    }
    break;
    case ObjectRawType::Await:
    {
        p->value = std::get<int>(value);
    }
    break;
    default:
        break;
    }
    return p;
}

Object::Ptr Object::make_array_by_stack(stack<Object::Ptr> &s)
{
    auto p = make_shared<Object>(ObjectRawType::Array);
    auto v = p->getv<shared_ptr<var_array_value>>();
    while (!s.empty())
    {
        v->push_back(s.top());
        s.pop();
    }
    return p;
}