#include "json.h"
#include "../../compiler/lexer.h"
#include <sstream>
#include <stack>

Object::Ptr json::from(string str)
{
    lexer l;
    auto tokens = l.process_string(str, charset::utf8);
    auto it = tokens.begin();
    return parse(tokens, it);
}

Object::Ptr json::parse(std::deque<token_base *> &tokens, std::deque<token_base *>::iterator &it)
{
    auto it_type = (*it)->get_type();
    if (it_type == token_types::op)
    {
        auto it_op = static_cast<token_op *>(*it);
        // {
        if (it_op->type == op_type::llb_)
        {
            auto o = make_shared<Object>(ObjectRawType::Struct);
            it++;
            while (true)
            {
                auto r = parse(tokens, it);
                if (r->type != ObjectRawType::Str)
                {
                    throw runtime_error("json: object key should be string");
                }
                auto t = (*it)->get_type();
                auto t2 = static_cast<token_string *>(*it);
                if (!((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::pair_))
                {
                    throw runtime_error("json: [:] should follow object key");
                }
                it++;
                auto value = parse(tokens, it);
                o->set_child(r->getv<string>(), value);
                if (!((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::com_))
                {
                    it++;
                    break;
                }
                it++;
            }
            return o;
        }
        else if (it_op->type == op_type::mlb_)
        {
            it++;
            stack<Object::Ptr> array_;
            while (true)
            {
                auto r = parse(tokens, it);
                if (r == nullptr)
                {
                    auto it_type_ = (*it)->get_type();
                    auto it_op_ = static_cast<token_op *>(*it);
                    if (it_op_->type != op_type::mrb_)
                    {
                        throw runtime_error("] should behind array define");
                    }
                    break;
                }
                array_.push(r);
                if ((*it)->get_type() == token_types::op)
                {
                    if (static_cast<token_op *>(*it)->type == op_type::com_)
                    {
                        it++;
                        continue;
                    }
                    if (static_cast<token_op *>(*it)->type == op_type::mrb_)
                    {
                        break;
                    }
                    throw runtime_error("] should behind array define");
                }
            }
            return Object::make_array_by_stack(array_);
        }
    }
    else if (it_type == token_types::string_l)
    {
        auto it_str = static_cast<token_string *>(*it);
        it++;
        return make_shared<Object>(decode_string(it_str->content));
    }
    else if (it_type == token_types::number)
    {
        auto it_str = static_cast<token_number *>(*it);
        std::stringstream ss(it_str->content);
        double d;
        ss >> d;
        it++;
        return make_shared<Object>(d);
    }
    else if (it_type == token_types::name)
    {
        auto it_name = static_cast<token_name *>(*it)->name;
        if (it_name == "null")
        {
            it++;
            return make_shared<Object>(ObjectRawType::Null);
        }
    }
    return nullptr;
}
string json::decode_string(string str)
{
    stringstream ss;
    bool escape = false;
    for (auto c : str)
    {
        if (c == '\\')
        {
            if (escape)
            {
                ss << "\\";
                escape = false;
            }
            else
            {
                escape = true;
            }
        }
        else if (c == '"')
        {
            ss << '"';
            escape = false;
        }
        else if (c == 'n')
        {
            if (escape)
            {
                ss << '\n';
                escape = false;
            }
            else
                ss << 'n';
        }
        else if (c == 't')
        {
            if (escape)
            {
                ss << '\t';
                escape = false;
            }
            else
                ss << 't';
        }
        else
        {
            ss << c;
        }
    }
    return ss.str();
}

shared_ptr<json> json::clone()
{
    auto j = make_shared<json>();
    for (auto &kv : data)
    {
        j->data[kv.first] = kv.second->clone();
    }
    return j;
}