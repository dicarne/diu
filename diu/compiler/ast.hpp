#ifndef AST_H_
#define AST_H_

#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "token.hpp"
using std::deque;
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
    /* data */
public:
    AST(/* args */);
    ~AST();

    unordered_map<string, UsePackageInfo> packages;
    unordered_map<string, string> OuterSymbol;

    bool error;

    void build_ast(deque<token_base *> tokens)
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
                                OuterSymbol[*os] = pkg_name;
                            }
                        }
                        else
                        {
                            for (auto newpkg = pkg.import_symbol.begin(); newpkg != pkg.import_symbol.end(); newpkg++)
                            {
                                if (old_pkg->second.import_symbol.find(*newpkg) == old_pkg->second.import_symbol.end())
                                {
                                    old_pkg->second.import_symbol.insert(*newpkg);
                                    OuterSymbol[*newpkg] = pkg_name;
                                }
                            }
                        }
                        continue;
                    }
                    else
                    {
                        error = true;
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
                        OuterSymbol[pkg_name] = pkg_name;
                        it++;
                        continue;
                    }
                    else
                    {
                        throw compile_error("pkgname should follow use");
                    }
                }
                // end handle [use]
            }
            it++;
        }
    }
};

AST::AST(/* args */)
{
}

AST::~AST()
{
}

#endif