#define SDL_MAIN_HANDLED  // Prevent SDL from redefining main
#include <iostream>
#include <DotBlue/DotBlue.h>
#include <DotBlue/GLPlatform.h>

int main()
{

    
    try {
        int result = DotBlue::GL_Test();
        std::cout << "GL_Test returned: " << result << std::endl;
    }
    catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "Unknown exception caught" << std::endl;
    }
    
    std::cout << "Test completed" << std::endl;
    return 0;
}

