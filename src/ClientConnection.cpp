#include "../include/ClientConnection.hpp"

// static int p = 0;
ClientConnection::ClientConnection() : _fd(-1), _request(), _response(_request), _virtualHost(NULL)
{
    _cgirun = false;
}

ClientConnection::ClientConnection(int fd) : _fd(fd), _request(), _response(_request), _virtualHost(NULL)
{

}

ClientConnection::~ClientConnection()
{
}
int ClientConnection::getFd() const
{
    return _fd;
}

time_t ClientConnection::getLastActive() const
{
    return _lastActive;
}
bool ClientConnection::isCGIRequest()
{
    return (_cgirun);
}

int ClientConnection::handleCGIWrite(EpollManager &epollManager)
{
    (void)epollManager; // To avoid unused parameter warning
    if (_request.getMethod() != "POST")
    {
        //epollManager.removeFd(_cgiInputFd);
        close(_cgiInputFd);
        _cgiInputFd = -1;
        //epollManager.modifyFd(_fd, EPOLLIN);
        _response._cgi.file.close();
        return 0;
    }
    char buff[1024];
    _response._cgi.file.read(buff, sizeof(buff));
    std::streamsize bytesToWrite = _response._cgi.file.gcount();
    if (bytesToWrite > 0)
     write(_cgiInputFd, buff, bytesToWrite);
    if (_response._cgi.file.eof())
    {
        //epollManager.removeFd(_cgiInputFd);
        close(_cgiInputFd);
        _cgiInputFd = -1;
        //epollManager.modifyFd(_fd, EPOLLIN);
        _response._cgi.file.close();
    }
    return 0;
}

int ClientConnection::addFileCGI(EpollManager &epollManager)
{
    _cgiInputFd = _response._cgi.input[1];
    _cgiOutputFd = _response._cgi.pip[0];
    int flags = fcntl(_cgiInputFd, F_GETFL, 0);
    fcntl(_cgiInputFd, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(_cgiOutputFd, F_GETFL, 0);
    fcntl(_cgiOutputFd, F_SETFL, flags | O_NONBLOCK);
    epollManager.addFd(_cgiInputFd);
    epollManager.addFd(_cgiOutputFd);
    epollManager.modifyFd(_cgiInputFd, EPOLLOUT);
    epollManager.modifyFd(_cgiOutputFd, EPOLLIN);
    return (0);
}

int ClientConnection::handleCGIRead(EpollManager &epollManager)
{
    char buf[4096];
    ssize_t bytes = read(_cgiOutputFd, buf, sizeof(buf));
    if (bytes > 0)
    {
        _response._sendBuffer.append(buf, bytes);
        return 0;
    }
    else if (bytes == 0)
    {
        // std::cout << "buffer = " << _response._sendBuffer << "\n";
        _response.cgi_response(_fd,_response._sendBuffer);
        cleanupCGI();
        _cgirun = false;
        epollManager.modifyFd(_fd, EPOLLIN | EPOLLOUT);
         return -1;
    }
    return 0;
}


// bool ClientConnection::isCGITimedOut()
// {

// }

void ClientConnection::cleanupCGI()
{
    if (_cgiInputFd != -1)
    {
        close(_cgiInputFd);
        _cgiInputFd = -1;
    }
    if (_cgiOutputFd != -1)
    {
        close(_cgiOutputFd);
        _cgiOutputFd = -1;
    }
    if (_response._cgi.pid != -1)
    {
        kill(_response._cgi.pid, SIGKILL);
        waitpid(_response._cgi.pid, NULL, 0);
        _response._cgi.pid = -1;
    }
}
int ClientConnection::handleRead(EpollManager &epollManager)
{
    RequestProcessor processor(_request, *_virtualHost);
    char buffer[BUFFER_SIZE];

    int n = recv(_fd, buffer, BUFFER_SIZE - 1, 0);
    if (n <= 0)
    {
        std::cerr << "[Failed] in recv in handleRead\n";
        return -1;
    }
    else
    {
        buffer[n] = '\0';

        if (_request.getReadBuffer().empty() &&
            ((n == 1 && (buffer[0] == '\n' || buffer[0] == '\r')) ||
             (n == 2 && buffer[0] == '\r' && buffer[1] == '\n')))
            return 0;
        _request.addReadBuffer(buffer, n);
        if (!_request.isRequestValid())
        {
            std::cout << "#####################{inside  is request valid (false)}###########################\n";
            return processor.sendErrorResponse(400, _fd);
        }
        if (_request.isHeaderComplete() && !_request.isHeaderValidated())
        {
            std::cout << "+++++++++++++++++++++ {before header parse}++++++++++++++\n";
            if (!_request.parseHeader())
                return processor.sendErrorResponse(400, _fd);
            processor.process();
            if (processor.hasError())
                return processor.sendErrorResponse(processor.getStatusCode(), _fd);

            else if (processor.hasRedirect())
                return SendRedirectResponse(processor);
            _request.setIsReqValid(true);
        }
        if (_request.isHeaderComplete() && _request.isHeaderValidated())
        {
            static int i;
            std::cout << "\n@@@@@@@@@@@@@@@@@@@@@@@@ {" << i++ << "} @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
            _response.setReqProcessor(&processor);
            _response._contentLenght = std::strtol(_request.getKeyValue("Content-Length").c_str(), NULL, 10);
            _response.process(_fd);
            if (_request.getCGI() && _response._waitingToSend)
            {
                _cgirun = true;
                addFileCGI(epollManager);
                epollManager.modifyFd(_fd, EPOLLIN | EPOLLOUT);
            }
            if (_response._headerSent && _response._waitingToSend)
            _request.clearReadBuffer();
        }
    }
    return 0;
}

int ClientConnection::handleWrite(EpollManager &epollManager)
{
    RequestProcessor processor(_request, *_virtualHost);
char buf[6000];
int status = 0;
_response._infile.read(buf, sizeof(buf));
size_t bytes_read = _response._infile.gcount();
_response._sendBuffer.append(buf, bytes_read);
if (_request.getCGI())
{
    int n = waitpid(_response._cgi.pid, &status, 1);
    if (n == 0 && time(NULL) - _response._cgi.time > 6)
    {
          processor.sendErrorResponse (500,_fd);
            kill(_response._cgi.pid, SIGKILL);
            waitpid(_response._cgi.pid, NULL, 0);
            _response._waitingToSend = false;
            close (_response._cgi.input[1]);
            close (_response._cgi.pip[0]);
            epollManager.modifyFd(_fd, EPOLLIN);
            return 0;
    }
    if ( n != 0 && _response._cgi.sendResponse(_fd) ==0)
    {
            _response._waitingToSend = false;
            close (_response._cgi.input[1]);
            close (_response._cgi.pip[0]);
            epollManager.modifyFd(_fd, EPOLLIN);
        }
}
if (_response._waitingToSend && !_response._sendBuffer.empty())
{
    ssize_t sent = send(_fd, _response._sendBuffer.c_str(), _response._sendBuffer.size(), 0);
    if (sent == -1)
    {
        return -1;
    }
    _response._sendBuffer.erase(0, sent);

    if (_response._infile.eof())
    {
        _response._waitingToSend = false;
        epollManager.modifyFd(_fd, EPOLLIN); // Remove EPOLLOUT
        _response._infile.close();
    }
}
    return 0;
}

int ClientConnection::SendRedirectResponse(const RequestProcessor &processor) const
{
    std::string response;
    Route route = processor.getMatchedRoute();

    response = _request.getVersion() + " " + route.redirect_code + " Moved Permanently" + "\r\n" +
               "Location: " + processor.getMatchedRoute().redirect_location + "\r\n" +
               "Content-Length: 0" + "\r\n" +
               "Connection: close" + "\r\n";
    std::cout << "================= { Route: " << route.path_prefix << "} ===================\n";
    std::cout << "++++++++++++++ {Redirect Response } ==================\n"
              << response << std::endl;
    if (send(_fd, response.c_str(), response.size(), 0) < 0)
        perror("send redirect");
    return (-1);
}

void ClientConnection::sendCGIResponse()
{
}

void ClientConnection::setVirtualHost(VirtualHost *host)
{
    // std::cout << "| _VHost : "  << _virtualHost << " }+++++++++++++\n";
    if (!host)
    {
        std::cerr << "[Error] Tried to set NULL VirtualHost!" << std::endl;
        return;
    }
    // std::cout << "++++++++++{ host: " << host;
    _virtualHost = host;
    std::cout << "++++++++++++++++++++++++++++++++++++++++\n";
}