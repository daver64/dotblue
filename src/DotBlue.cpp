#include "DotBlue/DotBlue.h"
#include <iostream>

namespace DotBlue {

void Hello() {
#if defined(DOTBLUE_WINDOWS)
    std::cout << "Hello from Windows!" << std::endl;
#elif defined(DOTBLUE_LINUX)
    std::cout << "Hello from Linux!" << std::endl;
#elif defined(DOTBLUE_FREEBSD)
    std::cout << "Hello from FreeBSD!" << std::endl;
#else
    std::cout << "Hello from Unknown OS!" << std::endl;
#endif
}

}
