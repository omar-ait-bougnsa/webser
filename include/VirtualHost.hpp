#ifndef VIRTUALHOST_HPP
#define VIRTUALHOST_HPP

#include "./Route.hpp"
#include "./Tools.hpp"
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <vector>
#include <exception>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <map>
#include <stdlib.h>
#include <fcntl.h>

class VirtualHost
{
private:
    std::string                     _host;
    int                             _serverFd;
    int                             _backlog;
    std::vector<std::string>        _serverNames;
    std::map<int, std::string>      _errorPages;
    size_t                          _clientMaxBodySize;
    std::vector<Route>              _route;

public:
    int                             _port;
    VirtualHost();
    ~VirtualHost();
    int SocketSetup();
    int GetServerFd() const;
    void _ErrorExit(const char *msg);

    //  Omar part

    void pars_server(std::vector<std::string> server,int size);
    void prse_error_page(std::string str);
    void printServer() const;
    void check_bracket(std::vector<std::string> &server);
};

#endif