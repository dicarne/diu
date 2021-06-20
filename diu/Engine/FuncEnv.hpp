#ifndef FUNC_ENV_H_
#define FUNC_ENV_H_
#include <memory>
#include <string>
#include <unordered_map>
#include "codes/CodeFunc.hpp"
#include "Type.hpp"
#include "NodeMessage.hpp"
#include "runtime_error.hpp"
#include "../compiler/bytecode/op_code.hpp"
#include "../compiler/compiler_type.hpp"
#include <stack>
#include <iostream>
#include "Node.hpp"
using std::cout;
using std::endl;
using std::make_shared;
using std::shared_ptr;
using std::stack;
using std::string;
using std::unordered_map;
class Node;
class FuncEnv
{
private:
    stack<shared_ptr<Object>> runtime;

    void call_another_func(string name, vector<Object> args)
    {
        waitting = true;
        auto m = make_shared<NodeMessage>();
        m->name = name;
        m->id = id;
        m->args = args;
    }
    void call_another_func(shared_ptr<Object> symbol, string name, vector<Object> args)
    {
        waitting = true;
        auto m = make_shared<NodeMessage>();
        m->name = name;
        m->id = id;
        m->args = args;
    }

public:
    bool completed;
    bool failed;
    bool waitting;
    shared_ptr<CodeFunc> code;
    unordered_map<string, shared_ptr<Object>> local_var;
    shared_ptr<Object> ret;
    Node *node;
    int id;
    int cur;
    PID callback_node;
    int callback_id;
    FuncEnv(/* args */) {}
    ~FuncEnv() {}
    void send_callback(Object obj)
    {
        waitting = false;
        runtime.push(make_shared<Object>(obj));
    }
    void init(shared_ptr<CodeFunc> c, shared_ptr<NodeMessage> m)
    {
        code = c;
        cur = 0;
        completed = false;
        failed = false;
        if (!code->mod.expired())
        {
            auto env = code->mod.lock();
            int index = 0;
            for (auto &a : code->args)
            {
                local_var[(*env->const_string)[a.arg_name]] = make_shared<Object>(m->args[index]);
                index++;
            }
        }
    }

    void handle_callback(Object callback)
    {
        runtime.push(make_shared<Object>(callback));
        waitting = false;
    }

    void run(int &limit)
    {
        if (failed)
            return;
        if (!code->mod.expired())
        {
            auto env = code->mod.lock();

            try
            {
                while (limit > 0)
                {
                    limit--;
                    auto c = code->codes[cur];
                    switch (c.op)
                    {
                    case opcode::LET:
                    {
                        auto name = (*env->const_string)[c.data];
                        local_var[name] = runtime.top();
                        runtime.pop();
                    }
                    break;
                    case opcode::LET_C:
                    {
                        auto name = (*env->const_string)[c.data];
                        auto pre_obj = runtime.top();
                        runtime.pop();
                        auto value = runtime.top();
                        runtime.pop();
                        pre_obj->get_child(name)->set(value);
                    }
                    break;
                    case opcode::VAR:
                    {
                        auto name = (*env->const_string)[c.data];
                        local_var[name] = make_shared<Object>(ObjectRawType::Null);
                    }
                    break;
                    case opcode::LOAD:
                    {
                        auto type = const_value_type((*env->const_type)[c.data]);
                        switch (type)
                        {
                        case const_value_type::DOUBLE:
                        {
                            auto v = (*env->const_double)[c.data];
                            auto p = make_shared<Object>(v);
                            runtime.push(p);
                        }
                        break;
                        case const_value_type::STRING:
                        {
                            auto v = (*env->const_string)[c.data];
                            auto p = make_shared<Object>(v);
                            runtime.push(p);
                        }
                        break;
                        default:
                            break;
                        }
                    }
                    break;
                    case opcode::VAR_FIND:
                    {
                        auto name = (*env->const_string)[c.data];
                        auto f = local_var.find(name);
                        if (f != local_var.end())
                        {
                            // 局部变量
                            runtime.push(local_var[name]);
                        }
                        else
                        {
                            auto o = make_shared<Object>(ObjectRawType::TypeSymbol);
                            o->symbol_find(name);
                            runtime.push(o);
                        }
                    }
                    break;
                    case opcode::VAR_FIND_C:
                    {
                        auto name = (*env->const_string)[c.data];
                        auto pre_obj = runtime.top();
                        runtime.pop();
                        if (pre_obj->type != ObjectRawType::TypeSymbol)
                        {
                            runtime.push(pre_obj->get_child(name));
                        }
                        else
                        {
                            pre_obj->symbol_find(name);
                            runtime.push(pre_obj);
                        }
                    }
                    break;
                    case opcode::FUNC_CALL_LOCAL:
                    {
                        auto name = (*env->const_string)[c.data];
                        auto argcount = int((*env->const_double)[c.info]);
                        stack<shared_ptr<Object>> st;
                        for (auto i = 0; i < argcount; i++)
                        {
                            st.push(runtime.top());
                            runtime.pop();
                        }
                        vector<Object> args;
                        for (auto i = 0; i < argcount; i++)
                        {
                            args.push_back(*(st.top()->copy()));
                            st.pop();
                        }
                        call_another_func(name, args);
                    }
                    break;
                    case opcode::FUNC_CALL_BY_NAME:
                    {
                        auto name = (*env->const_string)[c.data];
                        auto argcount = int((*env->const_double)[c.info]);
                        auto lastsymbol = runtime.top();
                        runtime.pop();
                        stack<shared_ptr<Object>> st;
                        for (auto i = 0; i < argcount; i++)
                        {
                            st.push(runtime.top());
                            runtime.pop();
                        }
                        vector<Object> args;
                        for (auto i = 0; i < argcount; i++)
                        {
                            args.push_back(*(st.top()->copy()));
                            st.pop();
                        }
                        call_another_func(lastsymbol->copy(), name, args);
                    }
                    break;
                    case opcode::RETURN:
                    {
                        auto cond = runtime.top();
                        runtime.pop();
                        ret = cond;
                        completed = true;
                        return;
                    }
                    break;
                    case opcode::JUMP:
                    {
                        cur += c.data;
                    }
                    break;
                    case opcode::JUMP_NIF:
                    {
                        auto cond = runtime.top();
                        runtime.pop();
                        if (!cond->as_bool())
                            cur += c.data;
                    }
                    break;
                    case opcode::CALC_OP:
                    {
                        auto o = op_type(c.info);
                        switch (o)
                        {
                        case op_type::add_:
                        {
                            auto b = runtime.top();
                            runtime.pop();
                            auto a = runtime.top();
                            runtime.pop();
                            auto c = Object::add(this, a, b);
                            runtime.push(c);
                        }
                        break;
                        case op_type::sub_:
                        {
                            auto b = runtime.top();
                            runtime.pop();
                            auto a = runtime.top();
                            runtime.pop();
                            auto c = Object::sub(this, a, b);
                            runtime.push(c);
                        }
                        break;
                        case op_type::mul_:
                        {
                            auto b = runtime.top();
                            runtime.pop();
                            auto a = runtime.top();
                            runtime.pop();
                            auto c = Object::mul(this, a, b);
                            runtime.push(c);
                        }
                        break;
                        case op_type::div_:
                        {
                            auto b = runtime.top();
                            runtime.pop();
                            auto a = runtime.top();
                            runtime.pop();
                            auto c = Object::div(this, a, b);
                            runtime.push(c);
                        }
                        break;
                        case op_type::mod_:
                        {
                            auto b = runtime.top();
                            runtime.pop();
                            auto a = runtime.top();
                            runtime.pop();
                            auto c = Object::mod(this, a, b);
                            runtime.push(c);
                        }
                        break;
                        case op_type::equ_:
                        {
                            auto b = runtime.top();
                            runtime.pop();
                            auto a = runtime.top();
                            runtime.pop();
                            auto c = Object::equ(this, a, b);
                            runtime.push(c);
                        }
                        break;
                        case op_type::neq_:
                        {
                            auto b = runtime.top();
                            runtime.pop();
                            auto a = runtime.top();
                            runtime.pop();
                            auto c = Object::neq(this, a, b);
                            runtime.push(c);
                        }
                        break;

                        default:
                            break;
                        }
                    }
                    break;
                    case opcode::WAIT_FUNC_CALL:
                    {
                        if (waitting)
                            return;
                        else
                        {
                            //TODO: 将函数返回值压入栈
                        }
                    }
                    break;
                    default:
                        break;
                    }

                    cur++;
                }
            }
            catch (const runtime_error &e)
            {
                failed = true;
                completed = true;
                std::cout << e.what() << endl;
                return;
            }
        }
    }
};
#endif