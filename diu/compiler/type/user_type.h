#ifndef USER_TYPE_H_
#define USER_TYPE_H_
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class user_type {
  public:
    class type_member {
      public:
        std::shared_ptr<user_type> type = nullptr;
        int bias = 0;
        type_member(std::shared_ptr<user_type> type, int bias)
            : type(type), bias(bias) {}
    };

    std::shared_ptr<user_type> parent = nullptr;
    std::vector<std::shared_ptr<user_type::type_member>> members;
    std::unordered_map<std::string, std::shared_ptr<user_type::type_member>>
        member_map;
    int size = 0;
    std::string name = "undefined type";

    std::shared_ptr<user_type::type_member> find_member(std::string name) {
        auto f_m = member_map.find(name);
        if (f_m == member_map.end()) {
            return nullptr;
        }
        return f_m->second;
    }

    static std::shared_ptr<user_type> make_int_type() {
        auto type = std::make_shared<user_type>();
        type->name = "@int";
        type->size = 4;
        return type;
    }
    static std::shared_ptr<user_type> make_bool_type() {
        auto type = std::make_shared<user_type>();
        type->name = "@bool";
        type->size = 1;
        return type;
    }
    static std::shared_ptr<user_type> make_double_type() {
        auto type = std::make_shared<user_type>();
        type->name = "@double";
        type->size = 8;
        return type;
    }
    static std::shared_ptr<user_type> make_float_type() {
        auto type = std::make_shared<user_type>();
        type->name = "@float";
        type->size = 4;
        return type;
    }
    std::shared_ptr<user_type::type_member>
    register_member(std::shared_ptr<user_type> member) {
        auto member_ptr =
            std::make_shared<user_type::type_member>(member, size);
        members.push_back(member_ptr);
        member_map[member->name] = member_ptr;
        size += member->size;
        return member_ptr;
    }
};

#endif