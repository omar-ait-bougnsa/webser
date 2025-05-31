#include "../include/ClientConnection.hpp"

ClientConnection::ClientConnection() : _fd(-1), _request(), _response(_request), _virtualHost(NULL)
{
}

ClientConnection::ClientConnection(int fd) :  _fd(fd), _request(), _response(_request), _virtualHost(NULL)
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
    (void)epollFd;

    int n = recv(_fd, buffer, BUFFER_SIZE - 1, 0);
    if (n <= 0)
    {
        std::cerr << "[Failed] in recv in handleRead\n";
        return -1;
    }
    else
    {
        buffer[n] = '\0';
        std::cout << "Client [" << _fd << "] send this message: " << buffer;

        _request.addReadBuffer(buffer, n);

        if (_request.isHeaderComplete())
        {
            std::cout << "+++++++++++++++++++++ {before header parse}++++++++++++++\n";
            if (!_request.parseHeader())
                return sendErrorResponse(400);
            std::cout << "+++++++++++++++++++++ {before validator}++++++++++++++\n";
            if (!validator.validate(_request, status_code))
                return sendErrorResponse(status_code);
            
            RequestProcessor processor(_request, *_virtualHost);

            processor.process();
            if (processor.hasError())
                return sendErrorResponse(processor.getStatusCode());

            std::cout << "############ resolved path : " << processor.getResolvedPath() << "| statuscode : " << processor.getStatusCode() << "#########\n";
         
            // _response.process(*this);
            // if (_request.getMethod() == "POST")
            // {
            //     std::string fullPath =  processor.getResolvedPath();
            //     _response.handel_post(_fd, fullPath);
            //     if (!_response.isBodyComplet())
            //         return 0;
            //     else
            //     {
            //         _request.reset();     
            //     }

            // }
            // else
            // {

            // }

            _request.setIsReqValid(true);
            if (_request.getMethod() == "GET" && processor.useAutoIndex())
                sendAutoIndexResponse(processor.getResolvedPath());
            else if (processor.isCGI())
                sendCGIResponse();
            else
                handleWrite(epollFd, processor);
            //

            if (!_request.checkBodyIsReady())
                return false;
            std::cout << "+++++++++++++++++++++ {set isReqValid}++++++++++++++\n";
        }
    }
    return 0;
}

int ClientConnection::handleWrite(int epollFd, const RequestProcessor &reqProcessor)
{
    (void)epollFd;

    std::cout << "+++++++++++++++++++++ {inside HandleWrite}++++++++++++++\n";

    if (_request.isDone())
    {
        std::string filePath = reqProcessor.getResolvedPath();
        std::string body = Tools::readFile(filePath); // readFile should return content or empty string
        std::stringstream bodySize;
        bodySize << body.size();

        if (body.empty())
        {
            return sendErrorResponse(404); // or return appropriate error
        }

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: " +
            bodySize.str() + "\r\n"
                             "Content-Type: text/html\r\n"
                             "Connection: keep-alive\r\n"
                             "\r\n" +
            body;

        send(_fd, response.c_str(), response.size(), 0);
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

    std::string errorPagePath;

    // Step 1: Try to find custom error page from VirtualHost config
    if (_virtualHost->_errorPages.count(statusCode))
    {
        errorPagePath = _virtualHost->_errorPages.at(statusCode);
    }
    else
    {
        // Step 2: Use fallback static path
        errorPagePath = "./static/errors/" + ss.str() + ".html";
    }

    // Read the content of the error file
    body = Tools::readFile(errorPagePath);

    // Step 3: Final fallback if file missing
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

void ClientConnection::sendAutoIndexResponse(const std::string &dir_path)
{
    std::cout << "$$$$$$$$$ { Inside Auto Index Responose } $$$$$$$$$$$\n";
    DIR *dir = opendir(dir_path.c_str());
    if (!dir)
    {
        perror("opendir");
        sendErrorResponse(500); // Internal Server Error
        return;
    }
    std::stringstream body;
    body << "<html><head><title>Index of " << dir_path << "</title></head><body>";
    body << "<h1>Index of " << dir_path << "</h1><ul>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        // Skip "." and ".."
        if (name == "." || name == "..")
            continue;

        std::string fullPath = dir_path + "/" + name;

        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode))
            name += "/";

        body << "<li><a href=\"" << name << "\">" << name << "</a></li>";
    }

    body << "</ul></body></html>";
    closedir(dir);

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html\r\n"
             << "Content-Length: " << body.str().size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body.str();

    ssize_t sent = send(_fd, response.str().c_str(), response.str().length(), 0);
    if (sent < 0)
        perror("send");
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