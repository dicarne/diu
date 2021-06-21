#ifndef COMPILE_BYTECODE_H_
#define COMPILE_BYTECODE_H_
#include "../ast/ast.hpp"
#include "bytecode_writer.hpp"
#include <vector>
using std::vector;
const int op_size = 4;
class compile_bytecode
{
private:
    shared_ptr<AST> ast;
    string path;
    shared_ptr<bytecode_writer> writer;

    unordered_map<string, int> const_value_index;
    unordered_map<string, int> const_number_value_index;
    unordered_map<string, int> pkg_import_index;
    unordered_map<string, int> symbol_import_index;
    unordered_map<string, int> node_index;

    int get_const_index(string name)
    {
        auto ni = const_value_index.find(name);
        int node_name_index;
        if (ni == const_value_index.end())
        {
            node_name_index = writer->write_const_string(name);
            const_value_index[name] = node_name_index;
            return node_name_index;
        }
        else
            return ni->second;
    }
    int get_const_double_index(string name)
    {
        auto ni = const_number_value_index.find(name);
        int node_name_index;
        if (ni == const_number_value_index.end())
        {
            node_name_index = writer->write_const_double(name);
            const_number_value_index[name] = node_name_index;
            return node_name_index;
        }
        else
            return ni->second;
    }

public:
    compile_bytecode(shared_ptr<AST> ast, shared_ptr<bytecode_writer> writer) : ast(ast), writer(writer)
    {
        writer->being_new_module_file();
    }
    ~compile_bytecode() {}

    void run()
    {
        get_const_double_index("0");
        writer->set_meta_data(ast->engine_ver, ast->module_name);
        for (auto &kv : ast->packages)
        {
            auto pkg = kv.second.packname;
            if (const_value_index.find(pkg) == const_value_index.end())
            {
                const_value_index[pkg] = writer->write_const_string(pkg);
            }
            for (auto &it : kv.second.import_symbol)
            {
                if (const_value_index.find(it) == const_value_index.end())
                {
                    const_value_index[it] = writer->write_const_string(it);
                }
            }
        }

        for (auto &kv : ast->packages)
        {
            auto pkg = kv.second.packname;
            pkg_import_index[pkg] = writer->reg_package(const_value_index[pkg]);
        }

        for (auto &it : ast->outer_symbol)
        {
            auto sy = it.first;
            auto pk = it.second;
            symbol_import_index[sy] = writer->reg_symbol(const_value_index[sy], pkg_import_index[pk]);
        }

        for (auto &node : ast->nodes)
        {
            writer->begin_new_node(get_const_index(node.second.node_name));

            // 写入有几个函数
            writer->write_node<int>(node.second.funcs.size());
            for (auto &fn : node.second.funcs)
            {
                auto ni = get_const_index(fn.second.name);
                writer->write_node(ni);
                writer->write_node<byte>(fn.second.is_static ? 1 : 0);
                writer->write_node<byte>(fn.second.args.size());
                for (auto &arg : fn.second.args)
                {
                    writer->write_node<int>(get_const_index(arg.arg_name));
                    writer->write_node<int>(get_const_index(arg.type_name));
                }

                auto stream = run_statements(fn.second.statements).str();
                writer->write_node<int>(stream.size());
                writer->write_node_stream(stream);
            }
        }
    }

private:
    stringstream run_statements(std::vector<ast_statement> &statements)
    {
        stringstream stream;
        for (auto ss = statements.begin(); ss != statements.end(); ss++)
        {
            if (ss->statemen_type == ast_statement::type::assign)
            {
                auto &ass = ss->assign;
                if (ass->newsymbol)
                {
                    write_op(stream, opcode::VAR, 0, get_const_index(ass->name));
                    //cout << "[VAR] " << ass->name << endl;
                }
                bool ins = false;
                if (ass->expr->expr_type == ast_expr::type::instance_num || ass->expr->expr_type == ast_expr::type::instance_string)
                {
                    ins = true;
                }
                else
                {
                    run_expr(stream, ass->expr);
                }
                if (ins)
                {
                    if (ass->expr->expr_type == ast_expr::type::instance_num)
                    {
                        write_op(stream, opcode::LOAD, 0, get_const_double_index(ass->expr->ins_value));
                        //cout << "[LOAD] to [NUMI] " << ass->expr->ins_value << " at " << get_const_double_index(ass->expr->ins_value) << endl;
                    }
                    if (ass->expr->expr_type == ast_expr::type::instance_string)
                    {
                        write_op(stream, opcode::LOAD, 0, get_const_index(ass->expr->ins_value));
                        //cout << "[LOAD] to [STRI] " << ass->expr->ins_value << " at " << get_const_index(ass->expr->ins_value) << endl;
                    }
                }

                if (ass->name == "")
                {
                    for (auto i = 0; i < ass->object_chain.size() - 1; i++)
                    {
                        if (i == 0)
                        {
                            write_op(stream, opcode::VAR_FIND, 0, get_const_index(ass->object_chain[i]));
                            //cout << "[VAR_FIND] " + ass->object_chain[i] << " " << get_const_index(ass->object_chain[i]) << endl;
                        }
                        else
                        {
                            write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(ass->object_chain[i]));
                            //cout << "[VAR_FIND_C] " + ass->object_chain[i] << " " << get_const_index(ass->object_chain[i]) << endl;
                        }
                    }
                    //write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(ass->object_chain[ass->object_chain.size() - 1]));
                    write_op(stream, opcode::LET_C, 0, get_const_index(ass->object_chain[ass->object_chain.size() - 1]));
                    //cout << "[LET_C] " << ass->object_chain[ass->object_chain.size() - 1] << " = [TOP]" << endl;
                }
                else
                {
                    //write_op(stream, opcode::VAR_FIND, 0, get_const_index(ass->name));
                    write_op(stream, opcode::LET, 0, get_const_index(ass->name));
                    //cout << "[LET] " << ass->name << " at " << get_const_index(ass->name) << " = [TOP]" << endl;
                }
            }
            if (ss->statemen_type == ast_statement::type::expr)
            {
                run_expr(stream, ss->expr);
            }
            if (ss->statemen_type == ast_statement::type::ret)
            {
                run_expr(stream, ss->expr);
                write_op(stream, opcode::RETURN, 0, 0);
                //cout << "[RET] [TOP]" << endl;
            }
            if (ss->statemen_type == ast_statement::type::if_)
            {
                run_expr(stream, ss->if_->cond);
                auto body_size = 0;
                // IF N TRUE ->
                body_size += op_size;
                //cout << "[IF] [TOP]" << endl;

                // if true body
                auto if_true_states = run_statements(*(ss->if_->if_true)).str();
                body_size += if_true_states.size();

                // JUMP TO END ->
                body_size += op_size;
                // write_op(stream, opcode::JUMP, 0, _if_final_end);
                // -----------

                auto _next_elif = body_size;
                vector<string> elif_conds;
                vector<string> elif_bodys;
                for (auto i = ss->if_->else_if->begin(); i != ss->if_->else_if->end(); i++)
                {
                    stringstream ss;
                    run_expr(ss, i->cond);
                    auto cond_s = ss.str();
                    elif_conds.push_back(cond_s);
                    body_size += cond_s.size();
                    // IF N TRUE ->
                    body_size += op_size;
                    //cout << "[ELIF] [TOP]" << endl;
                    auto sb = run_statements(*(i->if_true)).str();
                    body_size += sb.size();
                    //cout << "[ELIF] [END]" << endl;
                    elif_bodys.push_back(sb);
                    body_size += op_size;
                    // JUMP TO END ->
                }
                string else_body;
                if (ss->if_->if_false->size() > 0)
                {
                    //cout << "[ELSE]" << endl;
                    auto s = run_statements(*(ss->if_->if_false));
                    else_body = s.str();
                    body_size += else_body.size();
                    //cout << "[ELSE] [END]" << endl;
                }
                // -------------------------
                write_op(stream, opcode::JUMP_NIF, 0, _next_elif / 4 - 1); // ok
                stream.write(if_true_states.c_str(), if_true_states.size());
                write_op(stream, opcode::JUMP, 0, (body_size - _next_elif) / 4); // ok

                body_size -= _next_elif;
                for (auto i = 0; i < ss->if_->else_if->size(); i++)
                {
                    stream.write(elif_conds[i].c_str(), elif_conds[i].size());
                    body_size -= elif_conds[i].size();
                    body_size -= op_size;
                    body_size -= elif_bodys[i].size();
                    body_size -= op_size;
                    write_op(stream, opcode::JUMP_NIF, 0, (elif_bodys[i].size() / 4) + 1);
                    stream.write(elif_bodys[i].c_str(), elif_bodys[i].size());
                    write_op(stream, opcode::JUMP, 0, body_size / 4); // ok
                }
                stream.write(else_body.c_str(), else_body.size());
            }
            if (ss->statemen_type == ast_statement::type::while_)
            {
                stringstream sw;
                run_expr(sw, ss->while_->cond);
                auto se = sw.str();
                stream.write(se.c_str(), se.size());
                auto body_size = se.size();
                body_size += op_size;

                //cout << "[WHILE] [TOP]" << endl;
                auto body = run_statements(*(ss->while_->statements)).str();
                body_size += body.size();
                body_size += op_size;
                //cout << "[WHILE] [END]" << endl;
                // -----------------------
                // while (jump nif)
                // ...
                // jump back
                write_op(stream, opcode::JUMP_NIF, 0, (body_size - se.size() - 1) / 4);
                stream.write(body.c_str(), body.size());
                write_op(stream, opcode::JUMP, 0, -int(body_size) / 4);
            }

            //cout << "------------" << endl;
        }
        return stream;
    }

    void run_expr(stringstream &stream, shared_ptr<ast_expr> expr)
    {
        switch (expr->expr_type)
        {
        case ast_expr::type::instance_string:
            write_op(stream, opcode::LOAD, 0, get_const_index(expr->ins_value));
            //cout << "[LOAD] [STRI] " << expr->ins_value << " at " << get_const_index(expr->ins_value) << endl;
            break;
        case ast_expr::type::instance_num:
            write_op(stream, opcode::LOAD, 0, get_const_double_index(expr->ins_value));
            //cout << "[LOAD] [NUMI] " << expr->ins_value << " at " << get_const_double_index(expr->ins_value) << endl;
            break;
        case ast_expr::type::await_call:
            for (auto i = 0; i < expr->caller.size(); i++)
            {
                if (i == 0)
                {
                    write_op(stream, opcode::VAR_FIND, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
                else
                {
                    write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND_C] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
            }
            write_op(stream, opcode::WAIT_FUNC_CALL, 0, 1);
            break;
        case ast_expr::type::object_chain:

            for (auto i = 0; i < expr->caller.size(); i++)
            {
                if (i == 0)
                {
                    write_op(stream, opcode::VAR_FIND, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
                else
                {
                    write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND_C] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
            }
            break;
        case ast_expr::type::func_call_run:
        case ast_expr::type::func_call:
            for (auto i = expr->args.begin(); i != expr->args.end(); i++)
            {
                run_expr(stream, *i);
            }
            if (expr->func_name != "")
            {
                for (auto i = 0; i < expr->caller.size(); i++)
                {
                    if (i == 0)
                    {
                        write_op(stream, opcode::VAR_FIND, 0, get_const_index(expr->caller[i]));
                        //cout << "[VAR_FIND]" << expr->caller[i] << " at " << get_const_index(expr->caller[i]) << endl;
                    }
                    else
                    {
                        write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(expr->caller[i]));
                        //cout << "[VAR_FIND_C]" << expr->caller[i] << " at " << get_const_index(expr->caller[i]) << endl;
                    }
                }
                if (expr->expr_type == ast_expr::type::func_call)
                {
                    write_op(stream, opcode::FUNC_CALL_BY_NAME, expr->args.size(), get_const_index(expr->func_name));
                    write_op(stream, opcode::WAIT_FUNC_CALL, 0, 0);
                }
                else
                {
                    write_op(stream, opcode::FUNC_CALL_BY_NAME_RUN, expr->args.size(), get_const_index(expr->func_name));
                }
            }
            else
            {
                if (expr->expr_type == ast_expr::type::func_call)
                {
                    write_op(stream, opcode::FUNC_CALL_LOCAL, expr->args.size(), get_const_index(expr->caller[0]));
                    write_op(stream, opcode::WAIT_FUNC_CALL, 0, 0);
                }
                else
                {
                    write_op(stream, opcode::FUNC_CALL_LOCAL_RUN, expr->args.size(), get_const_index(expr->caller[0]));
                }
            }

            break;
        case ast_expr::type::expressions:
            run_expr(stream, expr->left);
            run_expr(stream, expr->right);
            write_op(stream, opcode::CALC_OP, char(expr->op), 0);
            //cout << "[OP] ";
            /*
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
            */
            break;
        default:
            break;
        }
    }

    void write_op(stringstream &s, opcode code, char info, int16_t data)
    {
        s.write(reinterpret_cast<char *>(&code), sizeof(code));
        s.write(reinterpret_cast<char *>(&info), sizeof(info));
        s.write(reinterpret_cast<char *>(&data), sizeof(data));
    }
};
#endif