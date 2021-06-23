#include "compiler_type.h"

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
    //{op_type::asi_, 1},
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
    {"await", keyword_type::await_},
    {"return", keyword_type::return_},
    {"if", keyword_type::if_},
    {"elif", keyword_type::elif_},
    {"else", keyword_type::else_},
    {"while", keyword_type::while_},
    {"engine", keyword_type::engine_},
    {"module", keyword_type::moudle_},
    {"run", keyword_type::run_},
    {"await", keyword_type::await_}};
