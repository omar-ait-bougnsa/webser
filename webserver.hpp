#ifndef webserver_hpp
#define webserver_hpp
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
    protected :
        std::string path_prefix;
        std::string root_directory;
        std::string autoindex;
        std::vector<std::string> methods;
        std::vector<std::string> index_files;
};
class Server : public Route
{
    private:
        std::string port;
        std::vector<std::string> server_names;
        std::map<int, std::string> error_page;
        size_t client_max_body_size;
        std::vector<Route> routes;
    public:
        void pars_server(std::vector<std::string> server,int size);
        void pars_Route(std::vector<std::string>location);
        std::map<int,std::string> prse_error_page(std::string str);
        void print()
        {
            std::cout << "port = " <<std::endl;
        }
};
std::vector<std::string> split(std::string line, char target);
std::vector<Server> parst_configfile(char *filename);
#endif