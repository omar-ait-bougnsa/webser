#include "webserver.hpp"

void Route::printRoute() const {
    std::cout << "Route Configuration:\n";
    std::cout << "  Path Prefix: " << path_prefix << "\n";
    std::cout << "  Root Directory: " << root_directory << "\n";
    std::cout << "  Autoindex: " << autoindex << "\n";
    std::cout << "  Methods: ";
    
    // For-loop compatible with C++98:
    for (size_t i = 0; i < methods.size(); ++i)
        std::cout << methods[i] << " ";
    std::cout << "\n";

    std::cout << "  Index Files: ";
    // For-loop compatible with C++98:
    for (size_t i = 0; i < index_files.size(); ++i)
        std::cout << index_files[i] << " ";
    std::cout << "\n";
}
void Server::printServer() const 
{
    std::cout << "Server Configuration:\n";
        std::cout << "  Port: " << port << "\n";
        
        std::cout << "  Server Names: ";
        for (size_t i = 0; i < server_names.size(); ++i)
            std::cout << server_names[i] << " ";
        std::cout << "\n";

        std::cout << "  Error Pages:\n";
        for (std::map<int, std::string>::const_iterator it = error_page.begin(); it != error_page.end(); ++it)
            std::cout << "    Error " << it->first << ": " << it->second << "\n";

        std::cout << "  Client Max Body Size: " << client_max_body_size << "\n";
        printRoute();
        std::cout << "\n  Nested Routes:\n";
        for (size_t i = 0; i < routes.size(); ++i) {
            std::cout << "    Route " << i + 1 << ":\n";
            routes[i].printRoute();
        }
}