#include "Execute.h"

#include <windows.h>
#include <ShellApi.h>
#include <iostream>


void execute(const char* name,const  char* dir, const char* params)
{
    std::cerr << name << std::endl;
    std::cerr << dir << std::endl;
    ShellExecuteA(NULL, "open", name, params, NULL, SW_SHOWNORMAL);

}
