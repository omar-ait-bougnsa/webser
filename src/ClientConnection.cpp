#include "../include/ClientConnection.hpp"

ClientConnection::ClientConnection() : _fd(-1)
{
}

ClientConnection::ClientConnection(int fd) : _fd(fd)
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

int ClientConnection::handleRead(int epollFd)
{
    char buffer[BUFFER_SIZE];
    Validator validator;
    int status_code;
    (void) epollFd;

    int n = recv(_fd, buffer, BUFFER_SIZE - 1, 0);
    if (n <= 0)
    {
        return -1;
    }
    else
    {
        buffer[n] = '\0';
        std::cout << "Client [" << _fd << "] send this message: " << buffer;
        
        _request.addReadBuffer(buffer);

        if (_request.isHeaderComplete())
        {
            std::cout << "+++++++++++++++++++++ {before header parse}++++++++++++++\n";
            if (!_request.parseHeader())
                return sendErrorResponse(400);
            std::cout << "+++++++++++++++++++++ {before validator}++++++++++++++\n";
            if (!validator.validate(_request, status_code))
                return sendErrorResponse(status_code);
            // if (!_request.checkBodyIsReady())
            // return false;
            std::cout << "+++++++++++++++++++++ {set isReqValid}++++++++++++++\n";

            _request.setIsReqValid(true);
        }
    }
    return 0;
}

int ClientConnection::handleWrite(int epollFd)
{
    (void)epollFd;

    std::cout << "+++++++++++++++++++++ {inside HandleWrite}++++++++++++++\n";
    if (_request.isDone())
    {
        std::cout << "+++++++++++++++++++++ {inside isDone()}++++++++++++++\n";
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: " +
            std::string("15") + "\r\n"
                                "Connection: keep-alive\r\n"
                                "\r\n"
                                "<h1>Helllo!</h1>";
        send(_fd, response.c_str(), response.size(), 0);
        std::cout << "send fail\n";
    }
    return 0;
}

bool ClientConnection::isTimedOut(time_t now, int timeoutSec)
{
    (void)now;
    (void)timeoutSec;
    return false;
}

int ClientConnection::sendErrorResponse(int statusCode)
{
    std::stringstream ss;
    std::stringstream bodySize;
    std::string body;

    std::cout << "+++++++++++++++++++++ {inside send error} ++++++++++++++\n";
    HttpError error(statusCode);
    ss << statusCode;
    body = Tools::readFile("./static/errors/" + ss.str() + ".html");
    if (body.empty())
    {
        body = "<h1>" + error.getMessage() + "</h1>";
    }
    bodySize << body.size();

    std::string response = error.getStatusLine() + "\r\n" +
                           "Content-Length: " + bodySize.str() + "\r\n" +
                           "Content-Type: text/html\r\n" +
                           "Connection: close\r\n" +
                           "\r\n" +
                           body;


    std::cout << "+++++++++++++++++++++ {before send() function in error}++++++++++++++\n";
    ssize_t sent = send(_fd, response.c_str(), response.length(), 0);
    if (sent < 0)
    {
        perror("send");
        return -1;
    }
    std::cout << "+++++++++++++++++++++ {after send() function in error}++++++++++++++\n";
    return -1;
}

