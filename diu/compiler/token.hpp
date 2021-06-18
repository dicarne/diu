#ifndef TOKEN_H_
#define TOKEN_H_
#include "compiler_type.hpp"
#include <string>
#include <deque>
using std::string;

#include <iostream>

class token_base
{
private:
    /* data */
public:
    token_base(/* args */) {}
    ~token_base() {}
    virtual string what() { return "base token"; }
    virtual token_types get_type() { return token_types::none; }
    virtual string dump() { return "T"; }
    static void dump_tokens(string message, std::deque<token_base *> &tokens)
    {
        std::cout << message << std::endl;
        for (auto it = tokens.begin(); it != tokens.end(); it++)
        {
            std::cout << (*it)->dump() << " ";
        }
        std::cout << std::endl;
    }
};

class token_string : public token_base
{
private:
    /* data */
public:
    string content;
    token_string(string content) : content(content) {}
    ~token_string() {}
    virtual string what() { return content; }
    virtual token_types get_type() { return token_types::string_l; }
    virtual string dump() { return content; }
};

class token_number : public token_base
{
private:
    /* data */
public:
    string content;
    token_number(string content) : content(content) {}
    ~token_number() {}
    virtual string what() { return content; }
    virtual token_types get_type() { return token_types::number; }
    virtual string dump() { return content; }
};

class token_name : public token_base
{
private:
    /* data */
public:
    string name;
    token_name(string name) : name(name) {}
    ~token_name() {}
    virtual string what() { return name; }
    virtual token_types get_type() { return token_types::name; }
    virtual string dump() { return name; }
};

class token_keyword : public token_base
{
private:
    /* data */
public:
    keyword_type type;
    token_keyword(keyword_type type) : type(type) {}
    ~token_keyword() {}
    virtual string what() { return "keyword"; }
    virtual token_types get_type() { return token_types::keyword; }
    virtual string dump() { return "[K]"; }
};

class token_op : public token_base
{
private:
    /* data */
public:
    op_type type;
    token_op(op_type type) : type(type) {}
    ~token_op() {}
    virtual string what() { return "op"; }
    virtual token_types get_type() { return token_types::op; }
    virtual string dump()
    {
        switch (type)
        {
        case op_type::llb_:
            return "{";
        case op_type::lrb_:
            return "}";
        case op_type::asi_:
            return "=";
        default:
            break;
        }
        return "[O]";
    }
};

class token_op_begin
{
private:
    /* data */
public:
    token_op_begin(/* args */) {}
    ~token_op_begin() {}
};

class token_op_end
{
private:
    /* data */
public:
    token_op_end(/* args */) {}
    ~token_op_end() {}
};
#endif