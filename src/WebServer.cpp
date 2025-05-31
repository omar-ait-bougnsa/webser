#include "../include/WebServer.hpp"

WebServer::WebServer(std::string fileName)
{
    //  create all virtual host and config parser
    ConfigParser praser(fileName);

    _VHost = praser.parse();

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
    _epollManager.addFd(client_fd);

    it = _Clients.find(client_fd);
    if (it == _Clients.end())
        _Clients[client_fd] = new ClientConnection(client_fd);
    return client_fd;
}

void WebServer::_EventLoop()
{
    std::vector<int> readyFds;
    std::vector<int>::iterator it;
    int client_fd;

    while (true)
    {
        readyFds = _epollManager.waitEvents();
        for (it = readyFds.begin(); it != readyFds.end(); ++it)
        {
            if (_IsItServerFd(*it))
            {
                std::cout << "++++++++{new client }++++++++====\n";
                client_fd = _AcceptNewClients(*it);
                _Clients[client_fd]->setVirtualHost(_GetVHostPtr(*it));
                if (!_Clients[client_fd])
                    std::cerr << "++++++++++++++++++++++++ERORR ++++++++++++++++++++\n";
            }
            else
            {
                std::cout << "++++++++{old client }++++++++====\n";
                // Client sent data
                if (_Clients[*it]->handleRead(_epollManager.getEpollFd()) == -1)
                {
                    std::cerr << "[Error] in handle Read\n";
                    _RemoveClient(*it);
                    continue;
                }
                _ClientLastActive[*it] = time(NULL);
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
        if (now - lastActive > 10)
        {
            _RemoveClient(fd);
            std::cout << "Client [" << fd << "] Closed because timeout\n";
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