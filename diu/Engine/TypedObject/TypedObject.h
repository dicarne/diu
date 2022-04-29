#ifndef TYPED_OBJECT_H
#define TYPED_OBJECT_H
#include "../../compiler/type/user_type.h"
#include <memory>
#include <cstring>
class TypedObject {
  public:
    user_type *type = nullptr;
    std::unique_ptr<char *> data = nullptr;
    TypedObject(user_type *type) : type(type) { alloc_data(); }

    std::shared_ptr<char> get_member_data(std::string name) {
        auto member = type->try_find_member(name);
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
        std::memcpy(this->data.get() + member->bias, data.get(), member->type->size);
    }
  private:
    void alloc_data() { data = std::make_unique<char *>(new char[type->size]); }
};

#endif