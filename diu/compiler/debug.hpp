#ifndef DEBUG_H_
#define DEBUG_H_
#define DEBUG_LOG(message) std::cout << "[LOG]" << codecvt::utf8().wide2local(message) << std::endl
#define DEBUG_ERROR(message) std::cout << "[ERROR]" << message << std::endl
#define DEBUG_LOG2(type, message) std::cout << "[LOG] " << type << " " << codecvt::utf8().wide2local(message) << std::endl
#endif