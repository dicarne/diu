#ifndef AST_H_
#define AST_H_

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "../token.hpp"
#include "ast_node.hpp"
#include <vector>
#include <sstream>

using std::deque;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

class UsePackageInfo
{
public:
    string packname;
    bool use_namespace;
    unordered_set<string> import_symbol;
};

class AST
{
private:
    ast_node build_node_ast(deque<token_base *> tokens);
    ast_func build_node_func(std::deque<token_base *>::iterator &it, deque<token_base *> &tokens);
    shared_ptr<ast_expr> get_next_expr(std::deque<token_base *>::iterator &it, deque<token_base *> &tokens);
    deque<token_base *> get_tokens_in_next(op_type lft, op_type rgt, std::deque<token_base *>::iterator &it, deque<token_base *> &tokens);
    shared_ptr<ast_expr> maybe_binary(shared_ptr<ast_expr> left, int my_prec, std::deque<token_base *>::iterator &it, deque<token_base *> &tokens);
    void build_statements(vector<ast_statement> &statements, deque<token_base *> &tokens);

public:
    AST(/* args */);
    ~AST();
    double engine_ver = 0;
    string module_name = "__GLOBAL__";
    unordered_map<string, UsePackageInfo> packages;
    unordered_map<string, string> outer_symbol;
    unordered_map<string, ast_node> nodes;
    void build_ast_from_tokens(deque<token_base *> tokens);
};

AST::AST(/* args */)
{
}

AST::~AST()
{
}

void AST::build_ast_from_tokens(deque<token_base *> tokens)
{
    for (auto it = tokens.begin(); it != tokens.end();)
    {
        auto t = *it;
        if (t->get_type() == token_types::keyword)
        {
            auto ktype = static_cast<token_keyword *>(t)->type;
            // handle [from]
            if (ktype == keyword_type::from_)
            {
                auto pkg = UsePackageInfo();
                it++;
                // handle pkgname
                if ((*it)->get_type() == token_types::name || (*it)->get_type() == token_types::string_l)
                {
                    auto pkg_name = (*it)->get_type() == token_types::name ? static_cast<token_name *>(*it)->name : static_cast<token_string *>(*it)->content;
                    pkg.packname = pkg_name;
                    it++;
                    // handle [use]
                    if ((*it)->get_type() == token_types::keyword && static_cast<token_keyword *>(*it)->type == keyword_type::use_pkg_)
                    {
                        it++;
                    }
                    else
                    {
                        throw compile_error("[use] should follow [from pkgname]", (*it)->line_num);
                    }
                    bool multi_import_symbol = false;
                    // handle [{]
                    if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::llb_)
                    {
                        multi_import_symbol = true;
                        it++;
                    }
                    if (!multi_import_symbol)
                    {
                        if ((*it)->get_type() == token_types::name)
                        {
                            pkg.import_symbol.insert(static_cast<token_name *>(*it)->name);
                            goto Complete_Import;
                        }
                        else
                        {
                            throw compile_error("import symbol name should follow [use]", (*it)->line_num);
                        }
                    }
                    else
                    {
                        while ((*it)->get_type() == token_types::name)
                        {
                            pkg.import_symbol.insert(static_cast<token_name *>(*it)->name);
                            it++;
                        }
                        if (!((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::lrb_))
                        {
                            throw compile_error("[}] should at the end of [from ... use {...}] statement", (*it)->line_num);
                        }
                        goto Complete_Import;
                    }
                Complete_Import:
                    it++;
                    auto old_pkg = packages.find(pkg_name);
                    if (old_pkg == packages.end())
                    {
                        packages[pkg_name] = pkg;
                        for (auto os = pkg.import_symbol.begin(); os != pkg.import_symbol.end(); os++)
                        {
                            outer_symbol[*os] = pkg_name;
                        }
                    }
                    else
                    {
                        for (auto newpkg = pkg.import_symbol.begin(); newpkg != pkg.import_symbol.end(); newpkg++)
                        {
                            if (old_pkg->second.import_symbol.find(*newpkg) == old_pkg->second.import_symbol.end())
                            {
                                old_pkg->second.import_symbol.insert(*newpkg);
                                outer_symbol[*newpkg] = pkg_name;
                            }
                        }
                    }
                    continue;
                }
                else
                {
                    throw compile_error("A pkgname should follow [from].", (*it)->line_num);
                }
            }
            // end handle [from]

            // handle [use]
            if (ktype == keyword_type::use_pkg_)
            {
                it++;
                if ((*it)->get_type() == token_types::name || (*it)->get_type() == token_types::string_l)
                {
                    auto pkg_name = (*it)->get_type() == token_types::name ? static_cast<token_name *>(*it)->name : static_cast<token_string *>(*it)->content;
                    auto pkg = UsePackageInfo();
                    pkg.packname = pkg_name;
                    auto old_pkg = packages.find(pkg_name);
                    if (old_pkg == packages.end())
                    {
                        pkg.use_namespace = true;
                        packages[pkg_name] = pkg;
                    }
                    else
                    {
                        old_pkg->second.use_namespace = true;
                    }
                    outer_symbol[pkg_name] = pkg_name;
                    it++;
                    continue;
                }
                else
                {
                    throw compile_error("pkgname should follow use", (*it)->line_num);
                }
            }
            // end handle [engine]
            if (ktype == keyword_type::engine_)
            {
                it++;
                if (engine_ver != 0)
                {
                    throw compile_error("[engine] should only appear once", (*it)->line_num);
                }
                if ((*it)->get_type() == token_types::number)
                {
                    std::stringstream stream(static_cast<token_number *>(*it)->content);
                    stream >> engine_ver;
                    it++;
                    continue;
                }
                else
                {
                    throw compile_error("a version number should behind [engine]", (*it)->line_num);
                }
            }
            if (ktype == keyword_type::moudle_)
            {
                it++;
                if (module_name != "__GLOBAL__")
                {
                    throw compile_error("[module] should only appear once", (*it)->line_num);
                }
                if ((*it)->get_type() == token_types::string_l)
                {
                    module_name = static_cast<token_string *>(*it)->content;
                    it++;
                    continue;
                }
                else if ((*it)->get_type() == token_types::name)
                {
                    module_name = static_cast<token_name *>(*it)->name;
                    it++;
                    continue;
                }
                else
                {
                    throw compile_error("a module string should behind [module]", (*it)->line_num);
                }
            }

            // handle [node]
            if (ktype == keyword_type::node_)
            {
                it++;
                string nodename;
                if ((*it)->get_type() == token_types::name)
                {
                    nodename = static_cast<token_name *>(*it)->name;
                    it++;
                }
                else
                {
                    throw compile_error("a name should follod [node]", (*it)->line_num);
                }
                if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::llb_)
                {
                    auto node = build_node_ast(get_tokens_in_next(op_type::llb_, op_type::lrb_, it, tokens));
                    node.node_name = nodename;
                    nodes[nodename] = node;
                    continue;
                }
                else
                {
                    throw compile_error("[{] should follow [node name]", (*it)->line_num);
                }
            }
            // end handle [node]
        }
        throw compile_error("not allowed token", (*it)->line_num);
        it++;
    }
}

ast_node AST::build_node_ast(deque<token_base *> tokens)
{
    ast_node node;
    for (auto it = tokens.begin(); it != tokens.end();)
    {
        if ((*it)->get_type() == token_types::keyword && static_cast<token_keyword *>(*it)->type == keyword_type::fn_)
        {
            // handle [fn]
            it++;
            auto fn = build_node_func(it, tokens);
            node.funcs[fn.name] = fn;
            continue;
        }
        if ((*it)->get_type() == token_types::keyword && static_cast<token_keyword *>(*it)->type == keyword_type::static_)
        {
            it++;
            if ((*it)->get_type() == token_types::keyword && static_cast<token_keyword *>(*it)->type == keyword_type::fn_)
            {
                // handle [static fn]
                it++;
                auto static_fn = build_node_func(it, tokens);
                static_fn.is_static = true;
                node.funcs[static_fn.name] = static_fn;
                continue;
            }
            else
            {
                //
                throw compile_error("[fn] should follow [static]", (*it)->line_num);
            }
        }
        it++;
    }

    return node;
}

ast_func AST::build_node_func(std::deque<token_base *>::iterator &it, deque<token_base *> &tokens)
{
    ast_func fun;
    if ((*it)->get_type() != token_types::name)
    {
        throw compile_error("a funcname should follow [fn]", (*it)->line_num);
    }
    fun.name = static_cast<token_name *>(*it)->name;
    it++;
    // args
    if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::slb_)
    {
        it++;
        while (!((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::srb_))
        {
            ast_func_arg arg;
            if ((*it)->get_type() == token_types::name)
            {
                arg.arg_name = static_cast<token_name *>(*it)->name;
                it++;
            }
            else
            {
                throw compile_error("arg name should appear in func def", (*it)->line_num);
            }
            if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::pair_)
            {
                it++;
            }
            else
            {
                throw compile_error("[:] should appear in func def", (*it)->line_num);
            }
            if ((*it)->get_type() == token_types::name)
            {
                arg.type_name = static_cast<token_name *>(*it)->name;
                it++;
            }
            else
            {
                throw compile_error("arg type should appear in func def", (*it)->line_num);
            }
            fun.args.push_back(arg);
            if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::com_)
            {
                it++;
            }
            else
            {
                if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::srb_)
                {
                    break;
                }
                throw compile_error("[,] should appear in func def", (*it)->line_num);
            }
        }
        it++;
    }
    else
    {
        throw compile_error("[(] should follow func def", (*it)->line_num);
    }
    auto func_body_tokens = get_tokens_in_next(op_type::llb_, op_type::lrb_, it, tokens);
    build_statements(fun.statements, func_body_tokens);
    if (fun.statements.size() == 0 || fun.statements[fun.statements.size() - 1].statemen_type != ast_statement::type::ret)
    {
        ast_statement ret;
        ret.statemen_type = ast_statement::type::ret;
        ret.expr = make_shared<ast_expr>();
        ret.expr->expr_type = ast_expr::type::instance_num;
        ret.expr->ins_value = "0";
        fun.statements.push_back(ret);
    }
    return fun;
}

void AST::build_statements(vector<ast_statement> &statements, deque<token_base *> &func_body_tokens)
{
    // body

    for (auto fi = func_body_tokens.begin(); fi != func_body_tokens.end();)
    {
        bool RUN_FUNC_FLAG = false;
        if ((*fi)->get_type() == token_types::keyword)
        {
            auto token = static_cast<token_keyword *>(*fi);
            if (token->type == keyword_type::let_)
            {
                // let statement
                ast_statement stat;
                auto let_assign = make_shared<ast_assign>();
                fi++;
                if ((*fi)->get_type() == token_types::name)
                {
                    let_assign->name = static_cast<token_name *>(*fi)->name;
                    let_assign->newsymbol = true;
                    fi++; // eat =
                    if (!((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::asi_))
                    {
                        throw compile_error("= should follow [let]", (*fi)->line_num);
                    }
                    fi++;
                    // handle remain statement
                    let_assign->expr = get_next_expr(fi, func_body_tokens);
                    stat.statemen_type = ast_statement::type::assign;
                    stat.assign = let_assign;
                    statements.push_back(stat);
                    continue;
                }
            }
            else if (token->type == keyword_type::return_)
            {
                // return statement
                ast_statement stat;
                stat.statemen_type = ast_statement::type::ret;
                fi++;
                stat.expr = get_next_expr(fi, func_body_tokens);
                statements.push_back(stat);
                continue;
            }
            else if (token->type == keyword_type::if_)
            {
                fi++;
                ast_statement stat;
                stat.statemen_type = ast_statement::type::if_;
                if (!((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::slb_))
                {
                    throw compile_error("[(] should follow if", (*fi)->line_num);
                }
                auto cond = get_tokens_in_next(op_type::slb_, op_type::srb_, fi, func_body_tokens);
                stat.if_ = make_shared<ast_if>();
                auto cond_if = cond.begin();
                stat.if_->cond = get_next_expr(cond_if, cond);

                if (!((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::llb_))
                {
                    throw compile_error("[{] should follow if(...)", (*fi)->line_num);
                }
                auto if_true_body = get_tokens_in_next(op_type::llb_, op_type::lrb_, fi, func_body_tokens);
                build_statements(*(stat.if_->if_true), if_true_body);

                while (fi != func_body_tokens.end() && (*fi)->get_type() == token_types::keyword && static_cast<token_keyword *>(*fi)->type == keyword_type::elif_)
                {
                    fi++;
                    ast_if elif_;

                    if (!((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::slb_))
                    {
                        throw compile_error("[(] should follow elif", (*fi)->line_num);
                    }
                    auto el_cond = get_tokens_in_next(op_type::slb_, op_type::srb_, fi, func_body_tokens);
                    auto el_cond_it = el_cond.begin();
                    elif_.cond = get_next_expr(el_cond_it, el_cond);

                    if (!((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::llb_))
                    {
                        throw compile_error("[{] should follow elif(...)", (*fi)->line_num);
                    }
                    auto el_body = get_tokens_in_next(op_type::llb_, op_type::lrb_, fi, func_body_tokens);
                    build_statements(*(elif_.if_true), el_body);
                    stat.if_->else_if->push_back(elif_);
                }
                if (fi != func_body_tokens.end() && (*fi)->get_type() == token_types::keyword && static_cast<token_keyword *>(*fi)->type == keyword_type::else_)
                {
                    fi++;
                    if (!((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::llb_))
                    {
                        throw compile_error("[{] should follow else", (*fi)->line_num);
                    }
                    auto else_body = get_tokens_in_next(op_type::llb_, op_type::lrb_, fi, func_body_tokens);
                    build_statements(*(stat.if_->if_false), else_body);
                }

                statements.push_back(stat);
                continue;
            }
            else if (token->type == keyword_type::while_)
            {
                fi++;
                ast_statement stat;
                stat.statemen_type = ast_statement::type::while_;
                auto cond = get_tokens_in_next(op_type::slb_, op_type::srb_, fi, func_body_tokens);
                stat.while_ = make_shared<ast_while>();
                auto expr_it = cond.begin();
                stat.while_->cond = get_next_expr(expr_it, cond);
                auto body = get_tokens_in_next(op_type::llb_, op_type::lrb_, fi, func_body_tokens);
                stat.while_->statements = make_shared<vector<ast_statement>>();
                build_statements(*(stat.while_->statements), body);
                statements.push_back(stat);
                continue;
            }
            else if (token->type == keyword_type::run_)
            {
                fi++;
                RUN_FUNC_FLAG = true;
                goto BgeinFuncCall;
            }
            else
            {
                // Another keywords
                // ...not yet
            }
        }
        if ((*fi)->get_type() == token_types::name)
        {
        BgeinFuncCall:
            auto start = fi;
            vector<string> name_chain;
            string name = "";
            while ((*fi)->get_type() != token_types::op)
            {
                auto unk_name = static_cast<token_name *>(*fi)->name;
                name_chain.push_back(unk_name);
                fi++;
                if ((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::dot_)
                {
                    fi++;
                }
            }
            if (name_chain.size() == 1)
            {
                name = name_chain[0];
                name_chain.clear();
            }

            if ((*fi)->get_type() == token_types::op)
            {
                auto nop = static_cast<token_op *>(*fi);
                if (nop->type == op_type::asi_)
                {
                    // A = b
                    fi++;
                    ast_statement stat;
                    auto let_assign = make_shared<ast_assign>();
                    // handle remain statement
                    let_assign->expr = get_next_expr(fi, func_body_tokens);
                    stat.statemen_type = ast_statement::type::assign;
                    let_assign->object_chain = name_chain;
                    let_assign->name = name;
                    stat.assign = let_assign;
                    statements.push_back(stat);
                    continue;
                }
                else if (nop->type == op_type::slb_ || nop->type == op_type::pair_)
                {
                    // A(... func call
                    fi = start;
                    ast_statement stat;
                    auto expr = get_next_expr(fi, func_body_tokens);
                    if (RUN_FUNC_FLAG)
                    {
                        expr->expr_type = ast_expr::type::func_call_run;
                    }
                    stat.statemen_type = ast_statement::type::expr;
                    stat.expr = expr;
                    statements.push_back(stat);
                    continue;
                }
                else
                {
                    throw compile_error("unknown op after name", (*fi)->line_num);
                }
            }
            else
            {
                throw compile_error("what follow name??", (*fi)->line_num);
            }
        }
        std::cout << (*fi)->dump() << std::endl;
        fi++;
    }
}

// current it should on left op
deque<token_base *> AST::get_tokens_in_next(op_type lft, op_type rgt, std::deque<token_base *>::iterator &it, deque<token_base *> &tokens)
{
    auto llb_count = 1;
    deque<token_base *> in_node_token;
    it++;
    while (llb_count > 0 && it != tokens.end())
    {
        if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == lft)
        {
            auto t = *static_cast<token_op *>(*it);
            llb_count++;
        }
        if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == rgt)
        {
            auto t = *static_cast<token_op *>(*it);
            llb_count--;
        }
        if (llb_count == 0)
        {
            it++;
            break;
        }
        else
        {
            in_node_token.push_back(*it);
            it++;
        }
    }
    if (llb_count > 0)
    {
        throw compile_error("no enough [}] after [{]", (*it)->line_num);
    }
    //in_node_token.pop_back();
    return in_node_token;
}
// a+b
// a+1
// "a"+b
// a +(f(c,d)+n.f1(c,d))*2
shared_ptr<ast_expr> AST::get_next_expr(std::deque<token_base *>::iterator &it, deque<token_base *> &tokens)
{
    if (it == tokens.end())
    {
        it--;
        throw compile_error("expr should be complete", (*it)->line_num);
    }
    auto expr = make_shared<ast_expr>();
    expr->expr_type = ast_expr::type::expressions;
    bool RUN_FLAG = false;
    if ((*it)->get_type() == token_types::keyword && static_cast<token_keyword *>(*it)->type == keyword_type::run_)
    {
        RUN_FLAG = true;
        it++;
    }
    if ((*it)->get_type() == token_types::name)
    {
        expr->expr_type = ast_expr::type::object_chain;
        expr->caller.push_back(static_cast<token_name *>(*it)->name);
        it++;

        // handle a.b.c
        // handle a.b.c:f
        while (it != tokens.end())
        {
            if ((*it)->get_type() == token_types::op)
            {
                auto sit = static_cast<token_op *>(*it);
                if (sit->type == op_type::dot_)
                {
                    it++;
                    if ((*it)->get_type() == token_types::name)
                    {
                        expr->caller.push_back(static_cast<token_name *>(*it)->name);
                        it++;
                        continue;
                    }
                    else
                    {
                        throw compile_error("a name should follow [.]", (*it)->line_num);
                    }
                }
                if (sit->type == op_type::pair_)
                {
                    it++;
                    if ((*it)->get_type() == token_types::name)
                    {
                        expr->func_name = static_cast<token_name *>(*it)->name;
                        expr->expr_type = RUN_FLAG ? ast_expr::type::func_call_run : ast_expr::type::func_call;
                        break;
                    }
                    else
                    {
                        throw compile_error("a name should follow [:]", (*it)->line_num);
                    }
                }
                break;
            }
            break;
        }

        if (it != tokens.end() && expr->caller.size() == 1)
        {
            auto t = (*it)->get_type();
            auto s = static_cast<token_op *>(*it)->type;
            if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::slb_)
            {
                expr->expr_type = RUN_FLAG ? ast_expr::type::func_call_run : ast_expr::type::func_call;
                it--;
            }
        }

        // handle func call
        if (expr->expr_type == ast_expr::type::func_call || expr->expr_type == ast_expr::type::func_call_run)
        {
            it++;
            auto type = (*it)->get_type();
            if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::slb_)
            {
                it++;
                while (!((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::srb_))
                {
                    expr->args.push_back(get_next_expr(it, tokens));
                    if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::com_)
                    {
                        it++;
                        continue;
                    }
                    else if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::srb_)
                    {
                        break;
                    }
                }
                it++;
            }
            else
            {
                throw compile_error("[(] should follow a func call", (*it)->line_num);
            }
        }

        return maybe_binary(expr, 0, it, tokens);
    }
    else if ((*it)->get_type() == token_types::number)
    {
        expr->expr_type = ast_expr::type::instance_num;
        expr->ins_value = static_cast<token_number *>(*it)->content;
        it++;
        return maybe_binary(expr, 0, it, tokens);
    }
    else if ((*it)->get_type() == token_types::string_l)
    {
        expr->expr_type = ast_expr::type::instance_string;
        expr->ins_value = static_cast<token_string *>(*it)->content;
        it++;
        return maybe_binary(expr, 0, it, tokens);
    }
    else if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::slb_)
    {
        auto inLb = get_tokens_in_next(op_type::slb_, op_type::srb_, it, tokens);
        auto ii = inLb.begin();
        return get_next_expr(ii, inLb);
    }
    else if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::sub_)
    {
        expr->expr_type = ast_expr::type::instance_num;
        expr->ins_value = "0";
        return maybe_binary(expr, 0, it, tokens);
    }
    else if ((*it)->get_type() == token_types::keyword && static_cast<token_keyword *>(*it)->type == keyword_type::await_)
    {
        expr->expr_type = ast_expr::type::await_call;
        it++;
        if ((*it)->get_type() != token_types::name)
        {
            throw compile_error("a name should follow [await]", (*it)->line_num);
        }
        if ((*it)->get_type() == token_types::name)
        {
            expr->caller.push_back(static_cast<token_name *>(*it)->name);
            it++;
            // handle a.b.c
            // handle a.b.c:f
            while (it != tokens.end())
            {
                if ((*it)->get_type() == token_types::op)
                {
                    auto sit = static_cast<token_op *>(*it);
                    if (sit->type == op_type::dot_)
                    {
                        it++;
                        if ((*it)->get_type() == token_types::name)
                        {
                            expr->caller.push_back(static_cast<token_name *>(*it)->name);
                            it++;
                            continue;
                        }
                        else
                        {
                            throw compile_error("a name should follow [.]", (*it)->line_num);
                        }
                    }
                    break;
                }
                break;
            }
            return maybe_binary(expr, 0, it, tokens);
        }
    }
    throw compile_error("can not get next expr", (*it)->line_num);
    return expr;
}

shared_ptr<ast_expr> AST::maybe_binary(shared_ptr<ast_expr> left, int my_prec, std::deque<token_base *>::iterator &it, deque<token_base *> &tokens)
{
    if (it == tokens.end())
        return left;
    if ((*it)->get_type() == token_types::op)
    {
        auto o = static_cast<token_op *>(*it);
        auto hisp = compiler_type::PRECEDENCE.find(o->type);
        auto tok = hisp != compiler_type::PRECEDENCE.end();
        if (!tok)
        {
            return left;
        }

        if (hisp->second > my_prec)
        {
            it++;
            auto his = make_shared<ast_expr>();
            his->expr_type = ast_expr::type::expressions;
            his->op = o->type;
            his->left = left;
            his->right = maybe_binary(get_next_expr(it, tokens), hisp->second, it, tokens);

            return maybe_binary(his, my_prec, it, tokens);
        }
        else
        {
            return left;
        }
        // throw compile_error("unknown op in expr");
    }
    else
    {
        return left;
    }
}

#endif