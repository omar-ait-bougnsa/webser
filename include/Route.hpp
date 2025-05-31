#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "./Tools.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <sstream>
#include <new>
#include <map>

class Route
{
    public:
        std::string path_prefix;
        std::string root_directory;
        std::string autoindex;
        std::vector<std::string> methods;
        std::vector<std::string> index_files;
        std::vector<std::string> cgi;
        std::vector<std::string> cgi_path;
        void printRoute() const;
        Route pars_Route(std::vector<std::string>location);
};

#endif