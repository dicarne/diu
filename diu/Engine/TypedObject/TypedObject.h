#ifndef TYPED_OBJECT_H
#define TYPED_OBJECT_H
#include "../../compiler/type/user_type.h"
#include <cstring>
#include <memory>
#include <utility>
class TypedObject {
  public:
    user_type *type = nullptr;
    std::unique_ptr<char *> data = nullptr;
    TypedObject *parent = nullptr;
    TypedObject(user_type *type) : type(type) { alloc_data(); }

    std::pair<TypedObject *, std::shared_ptr<user_type::type_member>>
    try_find_member(std::string name) {
        auto f_m = type->find_member(name);
        if (f_m != nullptr) {
            return std::make_pair(this, f_m);
        }
        if (parent != nullptr) {
            return parent->try_find_member(name);
        }
        return std::make_pair(this, nullptr);
    }

    std::shared_ptr<char> get_member_data(std::string name) {
        auto member = type->find_member(name);
        if (member == nullptr) {
            return nullptr;
        }
        auto vdata = std::shared_ptr<char>(new char[member->type->size]);
        std::memcpy(vdata.get(), data.get() + member->bias, member->type->size);
        return vdata;
    }

    void set_member_data(std::string name, std::shared_ptr<char> data) {
        auto member = type->try_find_member(name);
        if (member == nullptr) {
            return;
        }
        std::memcpy(this->data.get() + member->bias, data.get(),
                    member->type->size);
    }

  private:
    void alloc_data() { data = std::make_unique<char *>(new char[type->size]); }
};

#endif