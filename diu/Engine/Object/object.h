#ifndef OBJECT_H_
#define OBJECT_H_
#include "../Type.hpp"
#include "json.h"
#include <variant>
using std::variant;

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
    Await,      // !
};
class json;
class Object;
typedef variant<int, PID, shared_ptr<json>, double, bool, string, shared_ptr<vector<string>>> var;
class FuncEnv;
class Object
{
private:
    static string encode_string(string str);

public:
    var value;
    ObjectRawType type;

    Object(ObjectRawType type)
    {
        this->type = type;
        if (type == ObjectRawType::Struct)
        {
            value = make_shared<json>();
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
    Object(const char str[])
    {
        this->type = ObjectRawType::Str;
        value = string(str);
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
    }
    Object(const Object &ano)
    {
        value = ano.value;
        type = ano.type;
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

    shared_ptr<Object> copy();
    shared_ptr<Object> clone();

    bool as_bool()
    {
        if (type == ObjectRawType::Num)
        {
            if (std::get<double>(value) == 0)
                return false;
        }
        else if (type == ObjectRawType::Null)
        {
            return false;
        }
        else if (type == ObjectRawType::Str)
        {
            return getv<string>() == "";
        }

        return true;
    }

    void set_child(string name, shared_ptr<Object> value);

    shared_ptr<Object> get_child(string name);

    string to_string();

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

    string get_str_or_default(string default_)
    {
        if (type == ObjectRawType::Str)
            return getv<string>();
        else
            return default_;
    }

    double get_num()
    {
        if (type == ObjectRawType::Num)
            return getv<double>();
        else
            return 0;
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