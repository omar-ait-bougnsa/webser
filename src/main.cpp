#include "../include/WebServer.hpp"

int main()
{

    try
    {
        WebServer tmp("ConfigFile.conf");
        tmp.Run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "[Config Parsing Part]\n";
    }
}