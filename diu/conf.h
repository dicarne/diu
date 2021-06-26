#ifndef CONF_H_
#define CONF_H_
#include "confc.h"
#include <sstream>
inline double engine_version_get()
{
    std::stringstream ss;
    ss << ENGINE_VERSION_MASTER << "." << ENGINE_VERSION_SLAVER;
    double v;
    ss >> v;
    return v;
}

#endif
