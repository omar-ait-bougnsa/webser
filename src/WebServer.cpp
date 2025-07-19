#include "../include/WebServer.hpp"

WebServer::WebServer(std::string fileName)
{
    //  create all virtual host and config parser
    ConfigParser praser(fileName);

    _VHost = praser.parse();
    _epollManager.createEpollManger();
    std::vector<VirtualHost>::iterator it;
    for (it = _VHost.begin() ;it != _VHost.end(); ++it)
        std::cout << "_VHost fd : "<< it->GetServerFd() <<" | port: " << it->_port << "\n";
}

WebServer::~WebServer()
{
    std::map<int, ClientConnection *>::iterator it;
    for (it = _Clients.begin(); it != _Clients.end(); ++it)
        delete it->second;
    _Clients.clear();
    _ClientLastActive.clear();
}
int WebServer::_AcceptNewClients(int fd)
{
    // New Client Connection
    std::map<int, ClientConnection *>::iterator it;
    int client_fd;

    client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // No new clients now — normal!
            return -1;
        }
        else
        {
            perror("accept");
            return -1;
        }
    }
    std::cout << "new_client : " << client_fd << "\n";

    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags == -1) flags = 0;
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    _epollManager.addFd(client_fd);

    it = _Clients.find(client_fd);
    if (it == _Clients.end())
        _Clients[client_fd] = new ClientConnection(client_fd);
    return client_fd;
}

void WebServer::_SetupSockets()
{
    std::vector<VirtualHost>::iterator it;
    int server_fd;

    for (it = _VHost.begin(); it != _VHost.end(); ++it)
    {
        server_fd = it->SocketSetup();
        _epollManager.addFd(server_fd);
    }
}

bool WebServer::_IsItServerFd(int fd)
{
    std::vector<VirtualHost>::iterator it;

    for (it = _VHost.begin(); it != _VHost.end(); ++it)
    {
        std::cout << "[DEBUG] checking fd: " << fd << " against server_fd: " << it->GetServerFd() << "\n";
        if (fd == it->GetServerFd())
            return true;
    }
    return false;
}

void WebServer::_EventLoop()
{
    std::vector<EpollEvent> readyEvents;
    int client_fd;

    while (true)
    {
        readyEvents = _epollManager.waitEvents();
        for (size_t i = 0; i < readyEvents.size(); ++i)
        {
            int fd = readyEvents[i].fd;
            uint32_t events = readyEvents[i].events;

            if (_IsItServerFd(fd))
            {
                client_fd = _AcceptNewClients(fd);
                if (_Clients.find(client_fd) != _Clients.end())
                    _Clients[client_fd]->setVirtualHost(_GetVHostPtr(fd));
            }
            else
            {
                std::cout <<"--------"<< _Clients.count(fd) <<"---------------\n";
                if (_Clients.count(fd))
                {
                    ClientConnection *client = _Clients[fd];
                    if (client->isCGIRequest())
                    {
                        if (events & EPOLLOUT)
                        {
                            if (client->handleCGIWrite(_epollManager) == -1)
                            {
                                _RemoveClient(fd);
                                continue;
                            }
                        }
                        if (events & EPOLLIN)
                        {
                            exit (1);
                            if (client->handleCGIRead(_epollManager) == -1)
                            {
                                _RemoveClient(fd);
                                continue;
                            }
                        }
                    }
                    else
                    {
                        if (events & EPOLLIN)
                        {
                            if (client->handleRead(_epollManager) == -1)
                            {
                                _RemoveClient(fd);
                                continue;
                            }
                        }
                        if (events & EPOLLOUT)
                        {
                            if (client->handleWrite(_epollManager) == -1)
                            {
                                _RemoveClient(fd);
                                continue;
                            }
                        }
                    }
                }
            }
        }
        _CleanupInactiveConnection();
    }
}


void WebServer::_CleanupInactiveConnection()
{
    std::cout << "++++++++{cleanup inactive connection }++++++++====\n";
    time_t now;
    time_t lastActive;
    int fd;
    std::map<int, time_t>::iterator it = _ClientLastActive.begin();

    now = time(NULL);
    while (it != _ClientLastActive.end())
    {
        lastActive = it->second;
        fd = it->first;

        ++it;
        if (now - lastActive > 8)
        {
            _RemoveClient(fd);

            std::cout  << "Client [" << fd << "] Closed because timeout\n";
        }
    }
}

void WebServer::Run()
{
    try
    {
        _SetupSockets();
        _EventLoop();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() <<  "[Server part]\n";
    }
}

void WebServer::_RemoveClient(int fd)
{
    _epollManager.removeFd(fd);
    close(fd);

    std::map<int, ClientConnection *>::iterator map_it = _Clients.find(fd);
    if (map_it != _Clients.end())
    {
        delete map_it->second;
        _Clients.erase(map_it);
    }

    _ClientLastActive.erase(fd);
    std::cout << "Client [" << fd << "] Closed\n";
}


VirtualHost *WebServer::_GetVHostPtr(const int& index)
{
    std::vector<VirtualHost>::iterator it;

    for (it = _VHost.begin(); it != _VHost.end(); ++it)
    {
        int fd_server = it->GetServerFd();

        std::cout << "+++++++++++++++ { fd_server : " << fd_server << "| index: " << index << " } +++++++++++++++++\n"; 
        if (fd_server == index)
           return (&(*it));
    }
    return NULL;
}