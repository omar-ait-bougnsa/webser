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
#include <new>
class Server
{
    private:
        std::string port;
        std::string doman_name;
        std::string path;
        std::string hostname;
    public:
        void pars_server(std::vector<std::string> server);
};
#endif