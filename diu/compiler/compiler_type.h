#ifndef COMPILER_TYPE_H_
#define COMPILER_TYPE_H_
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <sstream>
#include <string>
using std::string;
using std::unordered_map;
using std::unordered_set;
enum token_types
{
    none,
    error,
    number,
    op,
    identifier,
    string_l,
    comment,
    name,
    keyword
};
enum class op_type : char
{
    add_,
    addasi_,
    sub_,
    subasi_,
    mul_,
    mulasi_,
    div_,
    divasi_,
    mod_,
    modasi_,
    pow_,
    powasi_,
    gt_,
    lt_,
    asi_,
    and_,
    or_,
    not_,
    equ_,
    neq_,
    ge_,
    le_,
    slb_,
    srb_,
    mlb_,
    mrb_,
    llb_,
    lrb_,
    com_,
    dot_,
    esb_,
    emb_,
    elb_,
    inc_,
    dec_,
    pair_,
    choice_
};

enum class keyword_type
{
    from_,
    use_pkg_,
    fn_,
    node_,
    let_,
    static_,
    new_,
    await_,
    return_,
    if_,
    elif_,
    else_,
    while_,
    engine_,
    moudle_,
    run_
};

class compiler_type
{
private:
    /* data */
public:
    static const unordered_set<char32_t> signals;
    static const unordered_map<std::string, op_type> op_map;
    static const unordered_map<std::string, keyword_type> keyword_map;
    static const unordered_map<op_type, int> PRECEDENCE;
    static bool
    issignal(char32_t ch)
    {
        return signals.count(ch) > 0;
    }

    static bool isnumber(char32_t ch)
    {
        if (ch >= '0' && ch <= '9')
            return true;
        return false;
    }

    static bool isempty(char32_t c)
    {
        if (c == ' ' || c == '\n' || c == '\t')
            return true;
        return false;
    }

    compiler_type(/* args */);
    ~compiler_type();
};

class compile_error final : public std::exception
{
    string mWhat = "Compile Error";
    int line;
    string str;

public:
    compile_error(string message, int line) : mWhat(message), line(line)
    {
        std::stringstream ss;
        ss << "line " << line << ": " << mWhat;
        str = ss.str();
    }
    const char *what() const noexcept override
    {

        return str.c_str();
    }
};

#endif
