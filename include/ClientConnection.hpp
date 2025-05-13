#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include "./HttpRequest.hpp"
#include "./Validator.hpp"
#include "./HttpError.hpp"
#include "./Tools.hpp"

#define BUFFER_SIZE 1024

class ClientConnection
{
private:
    int             _fd;
    time_t          _lastActive;
    std::string     _writeBuffer;
    HttpRequest     _request;
    // HttpResponse    _response;

public:
    ClientConnection();
    ClientConnection(int fd);
    ~ClientConnection();

    int getFd() const;
    time_t getLastActive() const;

    int handleRead(int epollFd);
    int handleWrite(int epollFd);
    int sendErrorResponse(int statusCode);
    bool isTimedOut(time_t now, int timeoutSec);
};

#endif
