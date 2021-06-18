#ifndef COMPILER_TYPE_H_
#define COMPILER_TYPE_H_
#include <unordered_set>
#include <unordered_map>
#include <exception>
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
enum class op_type
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

compiler_type::compiler_type(/* args */)
{
}

compiler_type::~compiler_type()
{
}

const unordered_set<char32_t> compiler_type::signals = {
    '+', '-', '*', '/', '%', '^', ',', '.', '>', '<', '=', '&', '|', '!', '(', ')', '[', ']', '{', '}', ':',
    '?', ';'};

const unordered_map<op_type, int> compiler_type::PRECEDENCE = {
    {op_type::asi_, 1},
    {op_type::or_, 2},
    {op_type::and_, 3},
    {op_type::lt_, 7},
    {op_type::gt_, 7},
    {op_type::le_, 7},
    {op_type::ge_, 7},
    {op_type::equ_, 7},
    {op_type::neq_, 7},
    {op_type::add_, 10},
    {op_type::sub_, 10},
    {op_type::mul_, 20},
    {op_type::div_, 20},
    {op_type::mod_, 20},
};

const unordered_map<std::string, op_type> compiler_type::op_map = {
    {"+", op_type::add_},
    {"+=", op_type::addasi_},
    {"-", op_type::sub_},
    {"-=", op_type::subasi_},
    {"*", op_type::mul_},
    {"*=", op_type::mulasi_},
    {"/", op_type::div_},
    {"/=", op_type::divasi_},
    {"%", op_type::mod_},
    {"%=", op_type::modasi_},
    {"^", op_type::pow_},
    {"^=", op_type::powasi_},
    {">", op_type::gt_},
    {"<", op_type::lt_},
    {"=", op_type::asi_},
    //{":=", op_type::lnkasi_},
    //{"&", op_type::addr_},
    //{"|", op_type::error_},
    {"&&", op_type::and_},
    {"||", op_type::or_},
    {"!", op_type::not_},
    {"==", op_type::equ_},
    {"!=", op_type::neq_},
    {">=", op_type::ge_},
    {"<=", op_type::le_},
    {"(", op_type::slb_},
    {")", op_type::srb_},
    {"[", op_type::mlb_},
    {"]", op_type::mrb_},
    {"{", op_type::llb_},
    {"}", op_type::lrb_},
    {",", op_type::com_},
    {".", op_type::dot_},
    {"()", op_type::esb_},
    {"[]", op_type::emb_},
    {"{}", op_type::elb_},
    {"++", op_type::inc_},
    {"--", op_type::dec_},
    {":", op_type::pair_},
    {"?", op_type::choice_},
    //{"->", op_type::arrow_},
    //{"..", op_type::to_},
    //{"...", op_type::vargs_}
};

const unordered_map<std::string, keyword_type> compiler_type::keyword_map = {
    {"from", keyword_type::from_},
    {"use", keyword_type::use_pkg_},
    {"fn", keyword_type::fn_},
    {"node", keyword_type::node_},
    {"let", keyword_type::let_},
    {"static", keyword_type::static_},
    //{"new", keyword_type::new_},
    {"await", keyword_type::await_}};

class compile_error final : public std::exception
{
    string mWhat = "Compile Error";

public:
    compile_error(string message) : mWhat(message) {}
    const char *what() const noexcept override
    {
        return this->mWhat.c_str();
    }
};

#endif
