#include "NodeMessage.hpp"

#define TO_BYTES(x) reinterpret_cast<char *>(&x), sizeof(x)

stringstream NodeMessage::serialize()
{
    stringstream ss;
    stringstream payload;
    auto s_payload = payload.str();
    auto z_payload = int(s_payload.size());
    ss.write(TO_BYTES(type));
    ss.write(TO_BYTES(id));
    ss.write(TO_BYTES(async_));
    ss.write(TO_BYTES(callbackNode.pid));
    ss.write(TO_BYTES(callbackNode.engine));
    ss.write(TO_BYTES(callbackNode.server));
    auto s_name = name.size();
    ss.write(TO_BYTES(s_name));
    ss.write(name.c_str(), s_name);
    auto z_args = args.size();
    ss.write(TO_BYTES(z_args));
    for (auto &it : args)
    {
        _serialize(&it, payload);
    }
    ss.write(TO_BYTES(z_payload));
    auto p = payload.str();
    ss.write(p.c_str(), p.size());
}

void NodeMessage::_serialize(Object *o, stringstream &ss)
{
    switch (o->type)
    {
    case ObjectRawType::Num:
    {
        ss.write(TO_BYTES(o->type));
        auto v = o->getv<double>();
        ss.write(TO_BYTES(v));
    }
    break;
    case ObjectRawType::Bool:
    {
        ss.write(TO_BYTES(o->type));
        auto v = o->getv<double>();
        ss.write(TO_BYTES(v));
    }
    break;
    case ObjectRawType::Str:
    {
        ss.write(TO_BYTES(o->type));
        auto v = o->getv<string>();
        auto sv = v.size();
        ss.write(TO_BYTES(sv));
        ss.write(v.c_str(), sv);
    }
    break;
    case ObjectRawType::Pid:
    {
        ss.write(TO_BYTES(o->type));
        auto v = o->getv<PID>();
        ss.write(TO_BYTES(v.pid));
        ss.write(TO_BYTES(v.engine));
        ss.write(TO_BYTES(v.server));
    }
    break;
    case ObjectRawType::Struct:
    {
        ss.write(TO_BYTES(o->type));
        auto v = o->getv<shared_ptr<json>>();
        auto vz = v->data.size();
        ss.write(TO_BYTES(vz));
        for (auto &it : v->data)
        {
            auto iz = it.first.size();
            ss.write(TO_BYTES(iz));
            ss.write(it.first.c_str(), iz);
            _serialize(it.second.get(), ss);
        }
    }
    break;
    case ObjectRawType::TypeSymbol:
    {
        ss.write(TO_BYTES(o->type));
        auto v = o->getv<shared_ptr<vector<string>>>();
        auto sv = v->size();
        ss.write(TO_BYTES(sv));
        for (auto s : *v)
        {
            auto sz = s.size();
            ss.write(TO_BYTES(sz));
            ss.write(s.c_str(), sz);
        }
    }
    break;
    case ObjectRawType::Array:
    {
        ss.write(TO_BYTES(o->type));
        auto v = o->getv<shared_ptr<vector<shared_ptr<Object>>>>();
        auto vz = v->size();
        ss.write(TO_BYTES(vz));
        for (auto &it : *v)
        {
            _serialize(it.get(), ss);
        }
    }
    break;
    default:
    {
        auto n = ObjectRawType::Null;
        ss.write(TO_BYTES(n));
    }
    break;
    }
}

NodeMessage *NodeMessage::deserialize(stringstream &ss)
{
    int id, async_, engine, server, arg_size;
    int64 pid;
    auto type = read<NodeMessageType>(ss);
    id = read<int>(ss);
    async_ = read<int>(ss);
    pid = read<int64>(ss);
    engine = read<int>(ss);
    server = read<int>(ss);
    auto name = read_str(ss);
    arg_size = read<int>(ss);
    auto m = new NodeMessage(type, name, id, PID(engine, pid, server), async_);
    for (size_t i = 0; i < arg_size; i++)
    {
        m->args.push_back(*_deserialize(ss));
    }

    return m;
}

shared_ptr<Object> NodeMessage::_deserialize(stringstream &ss)
{
    auto type = read<ObjectRawType>(ss);
    auto o = make_shared<Object>(type);
    switch (o->type)
    {
    case ObjectRawType::Num:
    {
        auto num = read<double>(ss);
        o->value = num;
    }
    break;
    case ObjectRawType::Bool:
    {
        auto num = read<double>(ss);
        o->value = num;
    }
    break;
    case ObjectRawType::Str:
    {
        auto s = read_str(ss);
        o->value = s;
    }
    break;
    case ObjectRawType::Pid:
    {
        auto pid = read<int64>(ss);
        auto engine = read<int>(ss);
        auto server = read<int>(ss);
        o->value = PID(engine, pid, server);
    }
    break;
    case ObjectRawType::Struct:
    {
        auto size = read<std::size_t>(ss);
        auto d = o->getv<shared_ptr<json>>();
        for (auto i = 0; i < size; i++)
        {
            auto key = read_str(ss);
            auto value = _deserialize(ss);
            d->data[key] = value;
        }
    }
    break;
    case ObjectRawType::TypeSymbol:
    {
        auto v = o->getv<shared_ptr<vector<string>>>();
        auto sv = read<std::size_t>(ss);
        for (auto i = 0; i < sv; i++)
        {
            v->push_back(read_str(ss));
        }
    }
    break;
    case ObjectRawType::Array:
    {
        auto v = o->getv<shared_ptr<vector<shared_ptr<Object>>>>();
        auto sv = read<std::size_t>(ss);
        for (auto i = 0; i < sv; i++)
        {
            v->push_back(_deserialize(ss));
        }
    }
    break;
    default:
    {
        }
    break;
    }
}