#ifndef JSON_H_
#define JSON_H_
#include <string>
#include <unordered_map>
#include "object.h"
#include "../../compiler/token.hpp"

using std::string;
using std::unordered_map;

class Object;
class json
{
private:
    
    static shared_ptr<Object> parse(std::deque<token_base *> &tokens, std::deque<token_base *>::iterator& it);
public:
    json() {}
    ~json() {}
    unordered_map<string, shared_ptr<Object>> data;
    static shared_ptr<Object> from(string str);
};

#endif