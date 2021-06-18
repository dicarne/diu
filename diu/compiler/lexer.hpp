#ifndef LEXER_H_
#define LEXER_H_
#include <codecvt>
#include <cwctype>
#include <climits>
#include <string>
#include <deque>
#include <locale>
#include "compiler_type.hpp"
#include "token.hpp"
#include "debug.hpp"

enum class charset
{
    ascii,
    utf8,
    gbk
};

namespace codecvt
{
    class charset
    {
    public:
        virtual ~charset() = default;

        virtual std::u32string local2wide(const std::deque<char> &) = 0;

        virtual std::string wide2local(const std::u32string &) = 0;

        virtual bool is_identifier(char32_t) = 0;
    };

    class ascii final : public charset
    {
    public:
        std::u32string local2wide(const std::deque<char> &local) override
        {
            return std::u32string(local.begin(), local.end());
        }

        std::string wide2local(const std::u32string &str) override
        {
            std::string local;
            local.reserve(str.size());
            for (auto ch : str)
                local.push_back(ch);
            return std::move(local);
        }

        bool is_identifier(char32_t ch) override
        {
            return ch == '_' || std::iswalnum(ch);
        }
    };

    class utf8 final : public charset
    {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;

        static constexpr std::uint32_t ascii_max = 0x7F;

    public:
        std::u32string local2wide(const std::deque<char> &local) override
        {
            std::string str(local.begin(), local.end());
            return cvt.from_bytes(str);
        }

        std::string wide2local(const std::u32string &str) override
        {
            return cvt.to_bytes(str);
        }

        bool is_identifier(char32_t ch) override
        {
            if (compiler_type::issignal(ch))
                return false;
            /**
				 * Chinese Character in Unicode Charset
				 * Basic:    0x4E00 - 0x9FA5
				 * Extended: 0x9FA6 - 0x9FEF
				 * Special:  0x3007
				 */
            if (ch > ascii_max)
                return (ch >= 0x4E00 && ch <= 0x9FA5) || (ch >= 0x9FA6 && ch <= 0x9FEF) || ch == 0x3007;
            else
                return ch == '_' || std::iswalnum(ch);
        }
    };

    class gbk final : public charset
    {
        static inline char32_t set_zero(char32_t ch)
        {
            return ch & 0x0000ffff;
        }

        static constexpr std::uint8_t u8_blck_begin = 0x80;
        static constexpr std::uint32_t u32_blck_begin = 0x8000;

    public:
        std::u32string local2wide(const std::deque<char> &local) override
        {
            std::u32string wide;
            uint32_t head = 0;
            bool read_next = true;
            for (auto it = local.begin(); it != local.end();)
            {
                if (read_next)
                {
                    head = *(it++);
                    if (head & u8_blck_begin)
                        read_next = false;
                    else
                        wide.push_back(set_zero(head));
                }
                else
                {
                    std::uint8_t tail = *(it++);
                    wide.push_back(set_zero(head << 8 | tail));
                    read_next = true;
                }
            }
            if (!read_next)
                throw; // compile_error("Codecvt: Bad encoding.");
            return std::move(wide);
        }

        std::string wide2local(const std::u32string &wide) override
        {
            std::string local;
            for (auto &ch : wide)
            {
                if (ch & u32_blck_begin)
                    local.push_back(ch >> 8);
                local.push_back(ch);
            }
            return std::move(local);
        }

        bool is_identifier(char32_t ch) override
        {
            if (compiler_type::issignal(ch))
                return false;
            /**
				 * Chinese Character in GBK Charset
				 * GBK/2: 0xB0A1 - 0xF7FE
				 * GBK/3: 0x8140 - 0xA0FE
				 * GBK/4: 0xAA40 - 0xFEA0
				 * GBK/5: 0xA996
				 */
            if (ch & u32_blck_begin)
                return (ch >= 0xB0A1 && ch <= 0xF7FE) || (ch >= 0x8140 && ch <= 0xA0FE) ||
                       (ch >= 0xAA40 && ch <= 0xFEA0) || ch == 0xA996;
            else
                return ch == '_' || std::iswalnum(ch);
        }
    };

}

class lexer
{
private:
    /* data */
public:
    lexer(/* args */);
    ~lexer();
    void process_char_buff(const std::deque<char> &raw_buff, std::deque<token_base *> &tokens, charset encoding);
};

lexer::lexer(/* args */)
{
}

lexer::~lexer()
{
}
#include <iostream>

void lexer::process_char_buff(const std::deque<char> &raw_buff, std::deque<token_base *> &tokens, charset encoding)
{
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
    for (auto it = buff.begin(); it != buff.end();)
    {
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
                    DEBUG_LOG2("STRING\t\t", tmp);
                    tokens.push_back(new token_string(cvt->wide2local(tmp)));
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
        if (*it == '\"')
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
                if (*it == '.' || *it == ',')
                {
                    tmp += *it;
                    it++;
                    continue;
                }
                else
                {
                    // REVIEW: MAKE NUMBER TOKEN
                    DEBUG_LOG2("NUMBER\t\t", tmp);
                    tokens.push_back(new token_number(cvt->wide2local(tmp)));

                    tmp.clear();
                    type = token_types::none;
                    // dont increase {it}
                    continue;
                }
            }
        }
        // end make number token
        // make end file
        if (*it == -1)
        {
            break;
        }
        // end file
        // make op token
        if (type == token_types::none && compiler_type::issignal(*it))
        {
            type = token_types::op;
            tmp += *it;
            it++;
            continue;
        }
        if (type == token_types::op)
        {
            if (compiler_type::issignal(*it))
            {
                tmp += *it;
                it++;
                continue;
            }
            else
            {
                type = token_types::none;

                if (cvt->wide2local(tmp) == "//")
                {
                    type = token_types::comment;
                }
                else
                {
                    // REVIEW: MAKE SIGNAL TOKEN
                    DEBUG_LOG2("OP\t\t", tmp);
                    auto find_op = compiler_type::op_map.find(cvt->wide2local(tmp));
                    if (find_op != compiler_type::op_map.end())
                    {
                        switch (find_op->second)
                        {
                        case op_type::esb_:
                            tokens.push_back(new token_op(op_type::slb_));
                            tokens.push_back(new token_op(op_type::srb_));
                            break;
                        case op_type::emb_:
                            tokens.push_back(new token_op(op_type::mlb_));
                            tokens.push_back(new token_op(op_type::mrb_));
                            break;
                        case op_type::elb_:
                            tokens.push_back(new token_op(op_type::llb_));
                            tokens.push_back(new token_op(op_type::lrb_));
                            break;
                        default:
                            tokens.push_back(new token_op(find_op->second));
                            break;
                        }
                    }
                    else
                    {
                        throw compile_error("UNKNOWN OP " + cvt->wide2local(tmp));
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
                DEBUG_LOG2("NAME\t\t", tmp);
                auto name_str = cvt->wide2local(tmp);
                auto find_keyword = compiler_type::keyword_map.find(name_str);
                if (find_keyword != compiler_type::keyword_map.end())
                {
                    // this is keyword
                    tokens.push_back(new token_keyword(find_keyword->second));
                }
                else
                {
                    // this is name
                    tokens.push_back(new token_name(cvt->wide2local(tmp)));
                }
                tmp.clear();
                // dont increase {it}
                continue;
            }
        }
        // end make name token
        if (!compiler_type::isempty(*it))
        {
            DEBUG_ERROR(*it);
        }
        it++;
    }
}

#endif