#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include "./VirtualHost.hpp"
#include "VirtualHost.hpp"
#include "HttpResponse.hpp"
#include "RequestProcessor.hpp"

#define BUFFER_SIZE 1024

// class HttpResponse;
// class RequestProcessor;
class ClientConnection
{
private:
    int             _fd;
    time_t          _lastActive;
    std::string     _writeBuffer;
    HttpRequest     _request;
    HttpResponse    _response;
    VirtualHost     *_virtualHost;

public:
    ClientConnection();
    ClientConnection(int fd);
    ~ClientConnection();

    int getFd() const;
    time_t getLastActive() const;

    int     handleRead(int epollFd);
    int     handleWrite(int epollFd, const RequestProcessor& reqProcessor);

    int     sendErrorResponse(int statusCode);
    void    sendAutoIndexResponse(const std::string& dir_path);
    void    sendCGIResponse();
    bool    isTimedOut(time_t now, int timeoutSec);
    void    setVirtualHost(VirtualHost  *host);
};

#endif
