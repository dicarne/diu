#ifndef AST_H_
#define AST_H_

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "token.hpp"
#include "ast_node.hpp"

using std::deque;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::unordered_set;

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

public:
    AST(/* args */);
    ~AST();

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
                        throw compile_error("[use] should follow [from pkgname]");
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
                            throw compile_error("import symbol name should follow [use]");
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
                            throw compile_error("[}] should at the end of [from ... use {...}] statement");
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
                    throw compile_error("A pkgname should follow [from].");
                }
            }
            // end handle [from]

            // handle [use]
            if (ktype == keyword_type::use_pkg_)
            {
                it++;
                if ((*it)->get_type() == token_types::name)
                {
                    auto pkg_name = static_cast<token_name *>(*it)->name;
                    auto pkg = UsePackageInfo();
                    pkg.packname = pkg_name;
                    auto old_pkg = packages.find(pkg_name);
                    if (old_pkg == packages.end())
                    {
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
                    throw compile_error("pkgname should follow use");
                }
            }
            // end handle [use]

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
                    throw compile_error("a name should follod [node]");
                }
                if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::llb_)
                {
                    token_base::dump_tokens("[before build node ast]", tokens);
                    auto node = build_node_ast(get_tokens_in_next(op_type::llb_, op_type::lrb_, it, tokens));
                }
                else
                {
                    throw compile_error("[{] should follow [node name]");
                }
            }
            // end handle [node]
        }
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
                throw compile_error("[fn] should follow [static]");
            }
        }
        it++;
    }

    return node;
}

ast_func AST::build_node_func(std::deque<token_base *>::iterator &it, deque<token_base *> &tokens)
{
    token_base::dump_tokens("[in build node func]", tokens);
    ast_func fun;
    if ((*it)->get_type() != token_types::name)
    {
        throw compile_error("a funcname should follow [fn]");
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
                throw compile_error("arg name should appear in func def");
            }
            if ((*it)->get_type() == token_types::op && static_cast<token_op *>(*it)->type == op_type::pair_)
            {
                it++;
            }
            else
            {
                throw compile_error("[:] should appear in func def");
            }
            if ((*it)->get_type() == token_types::name)
            {
                arg.type_name = static_cast<token_name *>(*it)->name;
                it++;
            }
            else
            {
                throw compile_error("arg type should appear in func def");
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
                throw compile_error("[,] should appear in func def");
            }
        }
        it++;
    }
    else
    {
        throw compile_error("[(] should follow func def");
    }
    // body
    auto func_body_tokens = get_tokens_in_next(op_type::llb_, op_type::lrb_, it, tokens);
    for (auto fi = func_body_tokens.begin(); fi != func_body_tokens.end();)
    {
        if ((*fi)->get_type() == token_types::keyword)
        {
            auto token = static_cast<token_keyword *>(*fi);
            if (token->type == keyword_type::let_)
            {
                // let statement
                ast_statement stat;
                ast_assign let_assign;
                fi++;
                if ((*fi)->get_type() == token_types::name)
                {
                    let_assign.name = static_cast<token_name *>(*fi)->name;
                    let_assign.newsymbol = true;
                    fi++; // eat =
                    if (!((*fi)->get_type() == token_types::op && static_cast<token_op *>(*fi)->type == op_type::asi_))
                    {
                        throw compile_error("= should follow [let]");
                    }
                    fi++;
                    // handle remain statement
                    let_assign.expr = get_next_expr(fi, func_body_tokens);
                    stat.statemen_type = ast_statement::type::assign;
                    stat.assign = let_assign;
                    fun.atatements.push_back(stat);
                }
            }
            else
            {
                // Another keywords
                // ...not yet
            }
        }
        if ((*fi)->get_type() == token_types::name)
        {
            auto unk_name = static_cast<token_name *>(*fi)->name;
            fi++;
            if ((*fi)->get_type() == token_types::op)
            {
                auto nop = static_cast<token_op *>(*fi);
                if (nop->type == op_type::asi_)
                {
                    // A = b
                    fi++;
                    ast_statement stat;
                    ast_assign let_assign;
                    // handle remain statement
                    let_assign.expr = get_next_expr(fi, func_body_tokens);
                    stat.statemen_type = ast_statement::type::assign;
                    stat.assign = let_assign;
                    fun.atatements.push_back(stat);
                }
                else if (nop->type == op_type::slb_)
                {
                    // A(... func call
                    fi--;
                    ast_statement stat;
                    auto expr = get_next_expr(fi, func_body_tokens);
                    stat.statemen_type = ast_statement::type::assign;
                    stat.expr = expr;
                    fun.atatements.push_back(stat);
                }
                else
                {
                    throw compile_error("unknown op after name");
                }
            }
            else
            {
                throw compile_error("what follow name??");
            }
        }
    }

    return fun;
}
#include <iostream>
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
            std::cout << (*it)->dump() << " ";
            it++;
        }
    }
    std::cout << std::endl;
    if (llb_count > 0)
    {
        throw compile_error("no enough [}] after [{]");
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
    auto expr = make_shared<ast_expr>();
    if ((*it)->get_type() == token_types::name)
    {
        expr->expr_type = ast_expr::type::object_chain;
        expr->caller.push_back(static_cast<token_name *>(*it)->name);

        // handle a.b.c
        // handle a.b.c:f
        while (it != tokens.end())
        {
            it++;
            if ((*it)->get_type() == token_types::op)
            {
                auto sit = static_cast<token_op *>(*it);
                if (sit->type == op_type::dot_)
                {
                    it++;
                    if ((*it)->get_type() == token_types::name)
                    {
                        expr->caller.push_back(static_cast<token_name *>(*it)->name);
                        continue;
                    }
                    else
                    {
                        throw compile_error("a name should follow [.]");
                    }
                }
                if (sit->type == op_type::pair_)
                {
                    it++;
                    if ((*it)->get_type() == token_types::name)
                    {
                        expr->func_name = static_cast<token_name *>(*it)->name;
                        expr->expr_type = ast_expr::type::func_call;
                        break;
                    }
                    else
                    {
                        throw compile_error("a name should follow [:]");
                    }
                }
                break;
            }
            break;
        }

        // handle func call
        if (expr->expr_type == ast_expr::type::func_call)
        {
            it++;
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
                throw compile_error("[(] should follow a func call");
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
    throw compile_error("can not get next expr");
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