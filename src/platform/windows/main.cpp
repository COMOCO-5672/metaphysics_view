#include "../../core/Application.h"
#include <iostream>

int main()
{
    Metaphysics::Application app;
    
    if (!app.Init()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }
    
    app.Run();
    app.Shutdown();
    
    return 0;
}
