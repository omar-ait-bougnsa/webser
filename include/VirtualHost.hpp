#ifndef VIRTUALHOST_HPP
#define VIRTUALHOST_HPP

#include "./Route.hpp"
#include "./SessionData.hpp"
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
#include <netdb.h>

class VirtualHost
{
public:
    std::string                             _host;
    int                                     _serverFd;
    int                                     _backlog;
    std::vector<std::string>                _serverNames;
    std::map<int, std::string>              _errorPages;
    size_t                                  _clientMaxBodySize;
    std::vector<Route>                      _route;
    int                                     _port;
    std::vector<VirtualHost>                _virtualHosts;
    std::map<std::string, SessionData>      _sessions;
    
    VirtualHost();
    ~VirtualHost();
    int                     SocketSetup();
    int                     GetServerFd() const;
    const VirtualHost&      GetMatchServer(const std::string &host) const;
    Route                   GetMatchRoute(const std::string &request_Path) const;
    void                    _ErrorExit(const char *msg);
    void                    clear();

    //  Omar part
    void    pars_body_size(std::string str);
    void    parsHOST(std::string host);
    void    pars_server(std::vector<std::string> server);
    void    prse_error_page(std::string str);
    void    check_bracket(std::vector<std::string> &server);
};

#endif