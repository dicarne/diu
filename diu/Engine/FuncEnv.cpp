#include "FuncEnv.h"
void FuncEnv::call_another_func(string name, vector<Object> args, bool async_)
{
    if (!async_)
        waitting = true;
    else
        async_index++;
    auto m = make_shared<NodeMessage>(NodeMessageType::Call, name, id, PID(0, 0), async_ ? async_index : 0);
    m->args = args;
    node->call_another_func(this, m);
}
void FuncEnv::call_another_func(shared_ptr<Object> symbol, string name, vector<Object> args, bool async_)
{
    if (!async_)
        waitting = true;
    else
        async_index++;
    auto m = make_shared<NodeMessage>(NodeMessageType::Call, name, id, PID(), async_ ? async_index : 0);
    m->args = args;

    node->call_another_func(this, symbol, m);
}

void FuncEnv::run(int &limit)
{
    if (failed)
        return;

    auto env = code->mod;

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
                local_var[name] = runtime.top()->copy();
                runtime.pop();
            }
            break;
            case opcode::LET_C:
            {
                auto target = runtime.top();
                runtime.pop();
                auto value = runtime.top();
                runtime.pop();
                target->set(value->type, value->value);
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
            case opcode::VAR_FIND_D:
            {
                auto topv = runtime.top();
                runtime.pop();
                auto pre_obj = runtime.top();
                runtime.pop();
                if (topv->type == ObjectRawType::Str)
                {
                    runtime.push(pre_obj->get_child(topv->getv<string>()));
                }
                else if (topv->type == ObjectRawType::Num)
                {
                    runtime.push(pre_obj->get_child(topv->getv<double>()));
                }
                else
                {
                    throw runtime_error("Only str or num can be index of object");
                }
            }
            break;
            case opcode::FUNC_CALL_LOCAL_RUN:
            case opcode::FUNC_CALL_LOCAL:
            {
                bool async_ = c.op == opcode::FUNC_CALL_LOCAL_RUN;
                auto name = (*env->const_string)[c.data];
                auto argcount = int(c.info);
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
                call_another_func(name, args, async_);
                if (async_)
                    runtime.push(Object::make_await(async_index));
                cur++;
                return;
            }
            break;
            case opcode::FUNC_CALL_BY_NAME_RUN:
            case opcode::FUNC_CALL_BY_NAME:
            {
                bool async_ = c.op == opcode::FUNC_CALL_BY_NAME_RUN;
                auto name = (*env->const_string)[c.data];
                auto argcount = int(c.info);
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
                call_another_func(lastsymbol->copy(), name, args, async_);
                if (async_)
                    runtime.push(Object::make_await(async_index));
                cur++;
                return;
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
                cur += short(c.data);
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
            case opcode::MAKE_ARRAY:
            {
                stack<shared_ptr<Object>> elements_stack;
                for (auto i = 0; i < c.data; i++)
                {
                    elements_stack.push(runtime.top());
                    runtime.pop();
                }
                runtime.push(Object::make_array_by_stack(elements_stack));
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
                case op_type::le_:
                {
                    auto b = runtime.top();
                    runtime.pop();
                    auto a = runtime.top();
                    runtime.pop();
                    auto c = Object::le(this, a, b);
                    runtime.push(c);
                }
                break;
                case op_type::lt_:
                {
                    auto b = runtime.top();
                    runtime.pop();
                    auto a = runtime.top();
                    runtime.pop();
                    auto c = Object::lt(this, a, b);
                    runtime.push(c);
                }
                break;
                case op_type::ge_:
                {
                    auto b = runtime.top();
                    runtime.pop();
                    auto a = runtime.top();
                    runtime.pop();
                    auto c = Object::ge(this, a, b);
                    runtime.push(c);
                }
                break;
                case op_type::gt_:
                {
                    auto b = runtime.top();
                    runtime.pop();
                    auto a = runtime.top();
                    runtime.pop();
                    auto c = Object::gt(this, a, b);
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
                if (c.data == 0)
                {
                    if (waitting)
                        return;
                    else
                    {
                        // 将函数返回值压入栈
                        runtime.push(ret);
                    }
                }
                else
                {
                    auto await_ = runtime.top();
                    auto a_id = await_->getv<int>();
                    auto f_await = waitting_async.find(a_id);
                    if (f_await != waitting_async.end())
                    {
                        runtime.pop();
                        runtime.push(f_await->second);
                    }
                    else
                    {
                        return;
                    }
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
void FuncEnv::init(shared_ptr<CodeFunc> c, shared_ptr<NodeMessage> m)
{
    code = c;
    cur = 0;
    completed = false;
    failed = false;

    auto env = code->mod;
    int index = 0;
    for (auto &a : code->args)
    {
        local_var[(*env->const_string)[a.arg_name]] = make_shared<Object>(m->args[index]);
        index++;
    }
    name = code->name;

    local_var["this"] = node->self;
}