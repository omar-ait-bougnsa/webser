#ifndef ROUTE_HPP
#define ROUTE_HPP

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

std::vector<std::string> split_withspace(std::string str);

class Route
{
    private :
        std::string path_prefix;
        std::string root_directory;
        std::string autoindex;
        std::vector<std::string> methods;
        std::vector<std::string> index_files;
        std::vector<std::string> cgi;
        std::vector<std::string> cgi_path;
    public:
        void printRoute() const;
        Route pars_Route(std::vector<std::string>location);
};

#endif