#include <iostream>
#include <string>

#include "lib.hpp"

auto main(int argc, char* argv[]) -> int
{
    if (argc > 1)
    {
        printf("rom path: %s\n", argv[1]);
        library lib(argv[1]);
//        std::string message = "Hello from " + lib.name + "!";
//        std::cout << message << '\n';
    }
    else
    {
        std::cout << "Not enough command line arguments\n";
    }
    return 0;
}
