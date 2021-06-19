#ifndef AST_RUNNER_H_
#define AST_RUNNER_H_
#include "ast.hpp"

#include <iostream>
using std::cout;
using std::endl;

class ast_runner
{
private:
    shared_ptr<AST> ast;

public:
    ast_runner(shared_ptr<AST> ast) : ast(ast) {}
    ~ast_runner() {}
    void run()
    {
        cout << endl;
        cout << "[Packages]" << endl;
        cout << "============" << endl;
        for (auto kv = ast->packages.begin(); kv != ast->packages.end(); kv++)
        {
            cout << kv->first << ": " << endl;
            if (kv->second.use_namespace)
            {
                cout << "\t"
                     << "global " << kv->first << endl;
            }
            for (auto p = kv->second.import_symbol.begin(); p != kv->second.import_symbol.end(); p++)
            {
                cout << "\t" << (*p) << endl;
            }
        }
        cout << endl;
        cout << "[Nodes]" << endl;
        cout << "============" << endl;
        for (auto kv = ast->nodes.begin(); kv != ast->nodes.end(); kv++)
        {
            cout << "[ node " << kv->second.node_name << " ]" << endl;
            for (auto fkv = kv->second.funcs.begin(); fkv != kv->second.funcs.end(); fkv++)
            {
                cout << "[ func " << fkv->second.name << " ]" << endl;
                if (fkv->second.is_static)
                {
                    cout << "static" << endl;
                }
                cout << "args:" << endl;
                for (auto ag = fkv->second.args.begin(); ag != fkv->second.args.end(); ag++)
                {
                    cout << ag->arg_name << "\t:\t" << ag->type_name << endl;
                }
                cout << "statement:" << endl;
                run_statements(fkv->second.statements);
            }
        }
    }

    void run_statements(vector<ast_statement> &statements)
    {
        for (auto ss = statements.begin(); ss != statements.end(); ss++)
        {
            if (ss->statemen_type == ast_statement::type::assign)
            {
                auto &ass = ss->assign;
                if (ass->newsymbol)
                {
                    cout << "[VAR] " << ass->name << endl;
                }
                bool ins = false;
                if (ass->expr->expr_type == ast_expr::type::instance_num || ass->expr->expr_type == ast_expr::type::instance_string)
                {
                    ins = true;
                }
                else
                {
                    run_expr(ass->expr);
                }

                cout << "[LET] ";
                for (auto i = ass->object_chain.begin(); i != ass->object_chain.end(); i++)
                {
                    cout << *i << ".";
                }
                cout << ass->name << " [=] ";

                if (ins)
                {
                    if (ass->expr->expr_type == ast_expr::type::instance_num)
                    {
                        cout << " [NUMI] " << ass->expr->ins_value << endl;
                    }
                    if (ass->expr->expr_type == ast_expr::type::instance_string)
                    {
                        cout << " [STRI] " << ass->expr->ins_value << endl;
                    }
                }
                else
                {
                    cout << "[TOP]" << endl;
                }
            }
            if (ss->statemen_type == ast_statement::type::expr)
            {
                run_expr(ss->expr);
            }
            if (ss->statemen_type == ast_statement::type::ret)
            {
                run_expr(ss->expr);
                cout << "[RET] [TOP]" << endl;
            }

            cout << "------------" << endl;
        }
    }

    void run_expr(shared_ptr<ast_expr> expr)
    {
        switch (expr->expr_type)
        {
        case ast_expr::type::instance_string:
            cout << "[PUSH] [STRI] " << expr->ins_value << endl;
            break;
        case ast_expr::type::instance_num:
            cout << "[PUSH] [NUMI] " << expr->ins_value << endl;
            break;
        case ast_expr::type::object_chain:
            cout << "[PUSH] [OBJV] ";
            for (auto i = expr->caller.begin(); i != expr->caller.end(); i++)
            {
                cout << *i << ".";
            }
            cout << endl;
            break;
        case ast_expr::type::func_call:
            for (auto i = expr->args.begin(); i != expr->args.end(); i++)
            {
                run_expr(*i);
            }
            cout << "[FUNC] ";
            for (auto i = expr->caller.begin(); i != expr->caller.end(); i++)
            {
                cout << *i << ".";
            }
            cout << " " << expr->func_name;
            cout << " [CALL] " << expr->args.size();
            cout << endl;
            break;
        case ast_expr::type::expressions:
            run_expr(expr->left);
            run_expr(expr->right);
            cout << "[OP] ";
            switch (expr->op)
            {
            case op_type::add_:
                cout << "[+] ";
                break;
            case op_type::sub_:
                cout << "[-] ";
                break;
            case op_type::mul_:
                cout << "[*] ";
                break;
            case op_type::div_:
                cout << "[/] ";
                break;
            case op_type::mod_:
                cout << "[%] ";
                break;
            case op_type::gt_:
                cout << "[>] ";
                break;
            case op_type::lt_:
                cout << "[<] ";
                break;
            case op_type::ge_:
                cout << "[>=] ";
                break;
            case op_type::le_:
                cout << "[<=] ";
                break;
            default:
                cout << "[OP] ";
                break;
            }
            cout << "2";
            cout << endl;
            break;
        default:
            break;
        }
    }
};

#endif