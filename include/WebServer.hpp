#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "./VirtualHost.hpp"
#include "./ClientConnection.hpp"
#include "./EpollManager.hpp"

class WebServer
{
private:
    EpollManager                    _epollManager;    
    std::vector<VirtualHost>        _VHost;
    std::map<int, time_t>           _ClientLastActive;
    std::map<int, ClientConnection*> _Clients;

    void    _SetupSockets();
    bool    _IsItServerFd(int fd);
    void    _EventLoop();
    void    _AcceptNewClients(int fd);
    void    _CleanupInactiveConnection();
    void    _RemoveClient(int fd);
public:
    WebServer(std::string ConfigFile);
    ~WebServer();
    
    void    Run();
};



#endif