#include "lexer.h"
#include <iostream>

lexer::lexer(/* args */)
{
}

lexer::~lexer()
{
}

std::deque<token_base *> lexer::process_file(string filepath, charset encoding)
{
    std::deque<char> buffer_q;
    std::ifstream file(filepath);
    while (!file.eof())
    {
        auto c = file.get();
        if (c == -1)
            break;
        buffer_q.push_back(c);
    }
    std::deque<token_base *> tokens;
    process_char_buff(buffer_q, tokens, charset::utf8);
    return tokens;
}

std::deque<token_base *> lexer::process_string(string str, charset encoding)
{
    std::deque<char> buffer_q;
    std::stringstream file(str);
    while (!file.eof())
    {
        auto c = file.get();
        if (c == -1)
            break;
        buffer_q.push_back(c);
    }
    std::deque<token_base *> tokens;
    process_char_buff(buffer_q, tokens, charset::utf8);
    return tokens;
}


void lexer::process_char_buff(const std::deque<char> &raw_buff, std::deque<token_base *> &tokens, charset encoding)
{
    int line_num = 1;
    if (raw_buff.empty())
        throw; // compile_error("Received empty character buffer.");
    std::unique_ptr<codecvt::charset> cvt = nullptr;
    switch (encoding)
    {
    case charset::ascii:
        cvt = std::make_unique<codecvt::ascii>();
        break;
    case charset::utf8:
        cvt = std::make_unique<codecvt::utf8>();
        break;
    case charset::gbk:
        cvt = std::make_unique<codecvt::gbk>();
        break;
    }
    std::u32string buff = cvt->local2wide(raw_buff);
    buff.push_back(-1);
    std::u32string tmp;
    token_types type = token_types::none;
    auto insideStr = false;
    auto escape = false;
    unordered_map<std::ptrdiff_t, int> nextlines;
    for (auto it = buff.begin(); it != buff.end();)
    {
        //cout << *it << endl;
        if (*it == '\n')
        {
            auto l = it - buff.begin();
            auto f_l = nextlines.find(l);
            if (f_l == nextlines.end())
            {
                nextlines[l] = line_num;
                line_num++;
                //DEBUG_ERROR(line_num);
            }
        }
        // make string token
        if (insideStr)
        {
            if (*it == '\\' && !escape)
            {
                escape = true;
                tmp += *it;
                it++;
                continue;
            }
            if (*it == '\"')
            {
                if (escape)
                {
                    tmp += *it;
                    escape = false;
                    it++;
                    continue;
                }
                else
                {
                    insideStr = false;
                    // REVIEW: ADD NEW ITER TOKEN
                    if (show_debug)
                        DEBUG_LOG2("STRING\t\t", tmp);
                    tokens.push_back(new token_string(cvt->wide2local(tmp), line_num));
                    tmp.clear();
                    it++;
                    type = token_types::none;
                    continue;
                }
            }
            escape = false;
            tmp += *it;
            it++;
            continue;
        }
        if (type == token_types::none && *it == '\"')
        {
            insideStr = true;
            it++;
            type = token_types::string_l;
            continue;
        }
        // end make string token

        // make number token
        if (type == token_types::none && compiler_type::isnumber(*it))
        {
            type = token_types::number;
            tmp += *it;
            it++;
            continue;
        }
        if (type == token_types::number)
        {
            if (compiler_type::isnumber(*it))
            {
                tmp += *it;
                it++;
                continue;
            }
            else
            {
                if (*it == '.')
                {
                    tmp += *it;
                    it++;
                    continue;
                }
                else
                {
                    // REVIEW: MAKE NUMBER TOKEN
                    //DEBUG_LOG2("NUMBER\t\t", tmp);
                    tokens.push_back(new token_number(cvt->wide2local(tmp), line_num));

                    tmp.clear();
                    type = token_types::none;
                    // dont increase {it}
                    continue;
                }
            }
        }
        // end make number token

        // make op token
        if (type == token_types::none && compiler_type::issignal(*it))
        {
            type = token_types::op;
            tmp += *it;
            if (*it == ')' || *it == '}' || *it == ']' || *it == ':' || *it == ',')
            {
                it++;
                goto CompleteOPtoken;
            }
            it++;
            continue;
        }
        if (type == token_types::op)
        {
            if (compiler_type::issignal(*it))
            {
                tmp += *it;
                if (*it == ')' || *it == '}' || *it == ']')
                {
                    it++;
                    goto CompleteOPtoken;
                }
                it++;
                continue;
            }
            else
            {
            CompleteOPtoken:
                type = token_types::none;

                if (cvt->wide2local(tmp) == "//")
                {
                    type = token_types::comment;
                }
                else
                {
                    // REVIEW: MAKE SIGNAL TOKEN
                    if (show_debug)
                        DEBUG_LOG2("OP\t\t", tmp);
                    auto find_op = compiler_type::op_map.find(cvt->wide2local(tmp));
                    if (find_op != compiler_type::op_map.end())
                    {
                        switch (find_op->second)
                        {
                        case op_type::esb_:
                            tokens.push_back(new token_op(op_type::slb_, line_num));
                            tokens.push_back(new token_op(op_type::srb_, line_num));
                            break;
                        case op_type::emb_:
                            tokens.push_back(new token_op(op_type::mlb_, line_num));
                            tokens.push_back(new token_op(op_type::mrb_, line_num));
                            break;
                        case op_type::elb_:
                            tokens.push_back(new token_op(op_type::llb_, line_num));
                            tokens.push_back(new token_op(op_type::lrb_, line_num));
                            break;
                        default:
                            tokens.push_back(new token_op(find_op->second, line_num));
                            break;
                        }
                    }
                    else
                    {
                        throw compile_error("UNKNOWN OP " + cvt->wide2local(tmp), line_num);
                    }
                }
                tmp.clear();
                continue;
            }
        }
        // end make op token

        // make comment token
        if (type == token_types::comment)
        {
            if (*it == '\n' || *it == -1)
            {
                type = token_types::none;
                if (show_debug)
                    DEBUG_LOG2("COMMENT\t\t", tmp);
                tmp.clear();
                continue;
            }
            else
            {
                tmp += *it;
                it++;
                continue;
            }
        }
        // end make comment token

        // make name token
        if (type == token_types::none && !compiler_type::isempty(*it))
        {
            type = token_types::identifier;
            tmp += *it;
            it++;
            continue;
        }
        if (type == token_types::identifier)
        {
            if (!compiler_type::isempty(*it) && !compiler_type::issignal(*it))
            {
                tmp += *it;
                it++;
                continue;
            }
            else
            {
                type = token_types::none;
                // REVIEW: MAKE NAME TOKEN
                if (show_debug)
                    DEBUG_LOG2("NAME\t\t", tmp);
                auto name_str = cvt->wide2local(tmp);
                auto find_keyword = compiler_type::keyword_map.find(name_str);
                if (find_keyword != compiler_type::keyword_map.end())
                {
                    // this is keyword
                    tokens.push_back(new token_keyword(find_keyword->second, line_num));
                }
                else
                {
                    // this is name
                    tokens.push_back(new token_name(cvt->wide2local(tmp), line_num));
                }
                tmp.clear();
                // dont increase {it}
                continue;
            }
        }
        // end make name token
        // make end file
        if (*it == -1)
        {
            break;
        }
        // end file
        if (!compiler_type::isempty(*it))
        {
            if (show_debug)
                DEBUG_ERROR(*it);
        }
        it++;
    }
}
