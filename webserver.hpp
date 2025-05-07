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
class Server : public Route
{
    private:
        std::string port;
        std::vector<std::string> server_names;
        std::map<int, std::string> error_page;
        size_t client_max_body_size;
        std::string   root;
        std::string   autoindex;
        std::vector<std::string>    index_files;
        std::vector<Route> routes;
    public:
        void pars_server(std::vector<std::string> server,int size);
        void prse_error_page(std::string str);
        void printServer() const;
};

class HttpResponse
{
public:
    int status_code;
    std::map<std::string, std::string> headers;
    std::string body;
};
std::vector<std::string> split(std::string line, char target);
std::vector<Server> parst_configfile(char *filename);
void handel_post(int fd,std::vector<std::string> method,std::string request);
#endif