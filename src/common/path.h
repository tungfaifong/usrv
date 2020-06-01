// Copyright (c) 2019-2020 TungFai Fong 

#ifndef USRV_PATH_H
#define USRV_PATH_H

#ifdef __unix__

#include <unistd.h>
char * GetCWD(char * buf, size_t size)
{
    return getcwd(buf, size);
}

#elif _WIN32

#include <direct.h>
char * GetCWD(char * buf, int size)
{
    return _getcwd(buf, size);
}

#endif // __unix__

namespace usrv
{
    static std::string PTAH_ROOT = GetCWD(NULL, 1024);
}

#endif // USRV_PATH_H
