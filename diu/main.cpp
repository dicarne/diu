#include <iostream>
#include "Engine/Engine.hpp"
#include "Engine/Node.hpp"
#include "Engine/Type.hpp"
#include "compiler/lexer.hpp"
#include <deque>
#include <fstream>
#include "compiler/ast/ast.hpp"
#include "compiler/ast/ast_runner.hpp"

int main(int, char **)
{
    char buffer[100];
    std::deque<char> buffer_q;
    std::ifstream file("test.txt");
    while (!file.eof())
    {
        auto c = file.get();
        if (c == -1)
            break;
        buffer_q.push_back(c);
    }
    std::deque<token_base *> tokens;
    lexer lex;
    lex.process_char_buff(buffer_q, tokens, charset::utf8);
    shared_ptr<AST> ast = make_shared<AST>();
    ast->build_ast_from_tokens(tokens);
    ast_runner runner(ast);
    runner.run();
    return 0;
}
