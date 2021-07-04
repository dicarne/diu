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

                auto stream = run_statements(fn.second.statements)->compile();
                writer->write_node<int>(stream.size());
                writer->write_node_stream(stream);
            }
        }
    }

private:
    class context
    {
        struct codeline
        {
            opcode code;
            char info;
            int16_t data;
        };

    public:
        typedef shared_ptr<context> Ptr;
        stringstream stream;
        std::deque<codeline> codes;
        string compile()
        {
            for (auto &it : codes)
            {
                stream.write(reinterpret_cast<char *>(&it.code), sizeof(it.code));
                stream.write(reinterpret_cast<char *>(&it.info), sizeof(it.info));
                stream.write(reinterpret_cast<char *>(&it.data), sizeof(it.data));
            }
            return stream.str();
        }
        void write_op(opcode code, char info, int16_t data)
        {
            codeline codel;
            codel.code = code;
            codel.info = info;
            codel.data = data;
            codes.push_back(std::move(codel));
        }
        void append(context::Ptr another)
        {
            if (another == nullptr)
                return;
            for (auto &it : another->codes)
            {
                codes.push_back(it);
            }
        }
        static context::Ptr New()
        {
            return make_shared<context>();
        }
        size_t size()
        {
            return codes.size() * 4;
        }
    };

    context::Ptr run_statements(std::vector<ast_statement> &statements)
    {
        context::Ptr stream = context::New();
        write_op(stream, opcode::STACK_OP, 0, 0);
        for (auto ss = statements.begin(); ss != statements.end(); ss++)
        {
            if (ss->statemen_type == ast_statement::type::assign)
            {
                auto &ass = ss->assign;
                if (ass->newsymbol)
                {
                    write_op(stream, opcode::VAR, 0, get_const_index(ass->object_find->caller[0]));
                    //cout << "[VAR] " << ass->name << endl;
                }

                run_expr(stream, ass->expr);
                // if (ass->object_find == "")
                if (ass->object_find->left != nullptr || ass->object_find->caller.size() > 1)
                {
                    run_expr(stream, ass->object_find);
                    //write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(ass->object_chain[ass->object_chain.size() - 1]));
                    write_op(stream, opcode::LET_C, 0, 0);
                    //cout << "[LET_C] " << ass->object_chain[ass->object_chain.size() - 1] << " = [TOP]" << endl;
                }
                else
                {
                    //write_op(stream, opcode::VAR_FIND, 0, get_const_index(ass->name));
                    write_op(stream, opcode::LET, 0, get_const_index(ass->object_find->caller[0]));
                    //cout << "[LET] " << ass->name << " at " << get_const_index(ass->name) << " = [TOP]" << endl;
                }
            }
            else if (ss->statemen_type == ast_statement::type::expr)
            {
                run_expr(stream, ss->expr);
            }
            else if (ss->statemen_type == ast_statement::type::ret)
            {
                run_expr(stream, ss->expr);
                write_op(stream, opcode::RETURN, 0, 0);
                //cout << "[RET] [TOP]" << endl;
            }
            else if (ss->statemen_type == ast_statement::type::if_)
            {
                run_expr(stream, ss->if_->cond);
                auto body_size = 0;
                // IF N TRUE ->
                body_size += op_size;
                //cout << "[IF] [TOP]" << endl;

                // if true body
                auto if_true_states = run_statements(*(ss->if_->if_true));
                body_size += if_true_states->size();

                // JUMP TO END ->
                body_size += op_size;
                // write_op(stream, opcode::JUMP, 0, _if_final_end);
                // -----------

                auto _next_elif = body_size;
                vector<context::Ptr> elif_conds;
                vector<context::Ptr> elif_bodys;
                for (auto i = ss->if_->else_if->begin(); i != ss->if_->else_if->end(); i++)
                {
                    auto ss = context::New();
                    run_expr(ss, i->cond);
                    auto cond_s = ss;
                    elif_conds.push_back(cond_s);
                    body_size += cond_s->size();
                    // IF N TRUE ->
                    body_size += op_size;
                    //cout << "[ELIF] [TOP]" << endl;
                    auto sb = run_statements(*(i->if_true));
                    body_size += sb->size();
                    //cout << "[ELIF] [END]" << endl;
                    elif_bodys.push_back(sb);
                    body_size += op_size;
                    // JUMP TO END ->
                }
                context::Ptr else_body;
                if (ss->if_->if_false->size() > 0)
                {
                    //cout << "[ELSE]" << endl;
                    else_body = run_statements(*(ss->if_->if_false));
                    body_size += else_body->size();
                    //cout << "[ELSE] [END]" << endl;
                }
                // -------------------------
                write_op(stream, opcode::JUMP_NIF, 0, _next_elif / 4 - 1); // ok
                stream->append(if_true_states);
                write_op(stream, opcode::JUMP, 0, (body_size - _next_elif) / 4); // ok

                body_size -= _next_elif;
                for (auto i = 0; i < ss->if_->else_if->size(); i++)
                {
                    stream->append(elif_conds[i]);
                    body_size -= elif_conds[i]->size();
                    body_size -= op_size;
                    body_size -= elif_bodys[i]->size();
                    body_size -= op_size;
                    write_op(stream, opcode::JUMP_NIF, 0, (elif_bodys[i]->size() / 4) + 1);
                    stream->append(elif_bodys[i]);
                    write_op(stream, opcode::JUMP, 0, body_size / 4); // ok
                }
                stream->append(else_body);
            }
            else if (ss->statemen_type == ast_statement::type::while_)
            {
                context::Ptr sw = context::New();
                run_expr(sw, ss->while_->cond);

                stream->append(sw);
                auto body_size = sw->size();
                body_size += op_size;

                //cout << "[WHILE] [TOP]" << endl;
                auto body = run_statements(*(ss->while_->statements));
                for (auto i = 0; i < body->codes.size(); i++)
                {
                    auto &it = body->codes[i];
                    if (it.code == opcode::JUMP && it.info == 2)
                    {
                        // continue
                        it.info = 0;
                        it.data = -i - 1;
                    }
                    else if (it.code == opcode::JUMP && it.info == 1)
                    {
                        // break;
                        it.info = 0;
                        it.data = body->codes.size() - i;
                    }
                }
                body_size += body->size();
                body_size += op_size;
                //cout << "[WHILE] [END]" << endl;
                // -----------------------
                // while (jump nif)
                // ...
                // jump back
                write_op(stream, opcode::JUMP_NIF, 0, (body_size - sw->size() - 1) / 4);
                stream->append(body);
                write_op(stream, opcode::JUMP, 0, -int(body_size) / 4);
            }
            else if (ss->statemen_type == ast_statement::type::break_)
            {
                write_op(stream, opcode::STACK_OP, 1, 0);
                write_op(stream, opcode::JUMP, 1, 0);
            }
            else if (ss->statemen_type == ast_statement::type::continue_)
            {
                write_op(stream, opcode::STACK_OP, 1, 0);
                write_op(stream, opcode::JUMP, 2, 0);
            }

            //cout << "------------" << endl;
        }
        write_op(stream, opcode::STACK_OP, 1, 0);
        return stream;
    }

    void run_expr(context::Ptr &stream, shared_ptr<ast_expr> expr)
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
                    write_op(stream, expr->start_with_dot ? opcode::VAR_FIND_C : opcode::VAR_FIND, 0, get_const_index(expr->caller[i]));
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
                    write_op(stream, expr->start_with_dot ? opcode::VAR_FIND_C : opcode::VAR_FIND, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
                else
                {
                    write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND_C] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
            }
            break;
        case ast_expr::type::array_:
        {
            for (auto &exp : expr->array)
            {
                run_expr(stream, exp);
            }
            write_op(stream, opcode::MAKE_ARRAY, 0, expr->array.size());
        }
        break;
        case ast_expr::type::find_child:
        {
            for (auto i = 0; i < expr->caller.size(); i++)
            {
                if (i == 0)
                {
                    write_op(stream, expr->start_with_dot ? opcode::VAR_FIND_C : opcode::VAR_FIND, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
                else
                {
                    write_op(stream, opcode::VAR_FIND_C, 0, get_const_index(expr->caller[i]));
                    //cout << "[VAR_FIND_C] " + expr->caller[i] << " " << get_const_index(expr->caller[i]) << endl;
                }
            }
            if (expr->left)
            {
                run_expr(stream, expr->left);
                if (expr->left->expr_type == ast_expr::type::instance_num || expr->left->expr_type == ast_expr::type::instance_string)
                    write_op(stream, opcode::VAR_FIND_D, 0, 0);
            }

            if (expr->right)
            {
                run_expr(stream, expr->right);
                if (expr->right->expr_type == ast_expr::type::instance_num || expr->right->expr_type == ast_expr::type::instance_string)
                    write_op(stream, opcode::VAR_FIND_D, 0, 0);
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
                        write_op(stream, expr->start_with_dot ? opcode::VAR_FIND_C : opcode::VAR_FIND, 0, get_const_index(expr->caller[i]));
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
            break;
        default:
            break;
        }
    }

    void write_op(context::Ptr &s, opcode code, char info, int16_t data)
    {
        s->write_op(code, info, data);
    }
};
#endif