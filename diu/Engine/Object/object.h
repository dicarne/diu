#ifndef OBJECT_H_
#define OBJECT_H_
#include "../Type.hpp"
#include "json.h"
#include <stack>
#include <variant>
using std::stack;
using std::variant;
enum ObjectRawType : unsigned char
{
    Null = 0, // !
    Pid,      // !
    Int,      // !
    Num,      // !
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
typedef vector<shared_ptr<Object>> var_array_value;
typedef variant<int, PID, shared_ptr<json>, double, bool, string, shared_ptr<vector<string>>, shared_ptr<var_array_value>> var;
class FuncEnv;
class Object
{
private:
    static string encode_string(string str);

public:
    typedef shared_ptr<Object> Ptr;
    var value;
    ObjectRawType type;

    Object(ObjectRawType type)
    {
        this->type = type;
        if (type == ObjectRawType::Struct)
        {
            value = make_shared<json>();
        }
        else if (type == ObjectRawType::TypeSymbol)
        {
            value = make_shared<vector<string>>();
        }
        else if (type == ObjectRawType::Null)
        {
            value = 0.0;
        }
        else if (type == ObjectRawType::Array)
        {
            value = make_shared<vector<Object::Ptr>>();
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
        value = d ? 1.0 : 0.0;
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
    void set(Object::Ptr obj)
    {
        this->type = obj->type;
        this->value = obj->value;
    }
    Object(const Object &ano)
    {
        value = ano.value;
        type = ano.type;
    }
    static Object::Ptr make_await(int async_index)
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

    Object::Ptr copy();
    Object::Ptr clone();

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

    static Object::Ptr make_array_by_stack(stack<Object::Ptr> &s);

    void set_child(string name, Object::Ptr value);

    Object::Ptr get_child(string name);
    Object::Ptr get_child(double name);

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

    bool try_convert_to(double &out)
    {
        if (type == ObjectRawType::Num || type == ObjectRawType::Bool || type == ObjectRawType::Null)
        {
            out = getv<double>();
            return true;
        }
        return false;
    }

    bool try_convert_to(string &out)
    {
        if (type == ObjectRawType::Str)
        {
            out = getv<string>();
            return true;
        }
        if (type == ObjectRawType::Num)
        {
            stringstream ss;
            ss << getv<double>();
            out = ss.str();
            return true;
        }
        return false;
    }

    static Object::Ptr add(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na + nb);
        }
        string sa, sb;
        if (a->try_convert_to(sa) && b->try_convert_to(sb))
        {
            return make_shared<Object>(sa + sb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr sub(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na - nb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr mul(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na * nb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr div(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na / nb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr mod(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(double(int(na) % int(nb)));
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr equ(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na == nb);
        }
        string sa, sb;
        if (a->try_convert_to(sa) && b->try_convert_to(sb))
        {
            return make_shared<Object>(sa == sb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr neq(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na != nb);
        }
        string sa, sb;
        if (a->try_convert_to(sa) && b->try_convert_to(sb))
        {
            return make_shared<Object>(sa != sb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr le(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na <= nb);
        }
        string sa, sb;
        if (a->try_convert_to(sa) && b->try_convert_to(sb))
        {
            return make_shared<Object>(sa <= sb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr lt(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na < nb);
        }
        string sa, sb;
        if (a->try_convert_to(sa) && b->try_convert_to(sb))
        {
            return make_shared<Object>(sa < sb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr ge(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na >= nb);
        }
        string sa, sb;
        if (a->try_convert_to(sa) && b->try_convert_to(sb))
        {
            return make_shared<Object>(sa >= sb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
    static Object::Ptr gt(FuncEnv *ctx, Object::Ptr a, Object::Ptr b)
    {
        double na, nb;
        if (a->try_convert_to(na) && b->try_convert_to(nb))
        {
            return make_shared<Object>(na > nb);
        }
        string sa, sb;
        if (a->try_convert_to(sa) && b->try_convert_to(sb))
        {
            return make_shared<Object>(sa > sb);
        }
        throw runtime_error("NOT SUPPORT DIFFERENT VALUE TYPE OP YET!");
        return make_shared<Object>(0);
    }
};
#endif