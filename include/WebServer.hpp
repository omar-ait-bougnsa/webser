#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "./ClientConnection.hpp"

class WebServer
{
private:
    EpollManager                    _epollManager;    
    std::vector<VirtualHost>        _VHost;
    std::map<int, time_t>           _ClientLastActive;
    std::map<int, ClientConnection*> _Clients;

    void            _SetupSockets();
    bool            _IsItServerFd(int fd);
    void            _EventLoop();
    int             _AcceptNewClients(int fd);
    void            _CleanupInactiveConnection();
    void            _RemoveClient(int fd);
    VirtualHost     *_GetVHostPtr(const int& index);
public:
    WebServer(std::string fileName);
    ~WebServer();
    
    void    Run();
};



#endif