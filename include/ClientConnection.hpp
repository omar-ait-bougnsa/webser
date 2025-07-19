#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include "./VirtualHost.hpp"
#include "VirtualHost.hpp"
#include "HttpResponse.hpp"
#include "./EpollManager.hpp"
// #include "RequestProcessor.hpp"


#define BUFFER_SIZE 60000

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
    bool            _cgirun;
    int _cgiInputFd;
    int _cgiOutputFd;
    pid_t _cgiPid;
    time_t _cgiStartTime;
    std::string _cgiBuffer;
public:
    int addFileCGI(EpollManager &epollManager);
    bool isCGIRequest();
    int handleCGIWrite(EpollManager &epollManager);
    int handleCGIRead(EpollManager &epollManager);
    int startCGIProcess(EpollManager &epollManager, const std::string &scriptPath, const std::string &queryString);
    bool isCGITimedOut();
    void cleanupCGI();
    ClientConnection();
    ClientConnection(int fd);
    ~ClientConnection();

    int getFd() const;
    time_t getLastActive() const;

    int     handleRead(EpollManager& epollManager);
    int     handleWrite(EpollManager& epollManager);
    
    int     SendRedirectResponse(const RequestProcessor &processor) const;
    void    sendCGIResponse();
    bool    isTimedOut(time_t now, int timeoutSec);
    void    setVirtualHost(VirtualHost  *host);
};

#endif
