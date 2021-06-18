#ifndef TOKEN_H_
#define TOKEN_H_
#include "compiler_type.hpp"
#include <string>
using std::string;

class token_base
{
private:
    /* data */
public:
    token_base(/* args */) {}
    ~token_base() {}
    virtual string what() { return "base token"; }
    virtual token_types get_type() { return token_types::none; }
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