#include "../include/ClientConnection.hpp"
#include "../include/HttpResponse.hpp"
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
    // char buffer[BUFFER_SIZE];
    // Validator validator;
    // int status_code;
     (void) epollFd;
   char         buf[1024];
   std::string  full_request;
   ssize_t      bytes_read;
   size_t       length_body;
   size_t       body_read;
   size_t       pos;
   HttpResponse request;
   while ((bytes_read = read(_fd, buf, 1023)) > 0)
   {
      buf[bytes_read] = '\0';
      full_request += buf;
      pos = full_request.find("\r\n\r\n");
      if (pos != std::string::npos)
      {
         size_t start =  full_request.find("Content-Length:") + 16;

         size_t end = full_request.find("\r\n", start);
         std::string content_length_str = full_request.substr(start, end - start);
         length_body = atol(content_length_str.c_str());
         body_read = full_request.size() - pos;
         if (length_body <= body_read)
             break;
      }
      else 
         break;
   }
   pos = full_request.find("\n");
   std::string path = full_request.substr(0,pos);
   std::vector <std::string> method = Tools::split (path,' ');
   // size_t pos = full_request.find("\r\n\r\n");
   // if (pos != std::string::npos)
   // full_request.erase(0,pos);
   std::cout <<full_request <<std::endl;
   if (method[0] == "GET")
      request.handel_get(_fd,method);
   else if (method[0] == "POST")
     request.handel_post(_fd,method,full_request);
   else if (method[0] == "DELETE")
      std::cout <<"baki makhdam\n";
    return (0);
}
int ClientConnection::handleWrite(int epollFd)
{
    (void)epollFd;

   // std::cout << "+++++++++++++++++++++ {inside HandleWrite}++++++++++++++\n";
    if (_request.isDone())
    {
       // std::cout << "+++++++++++++++++++++ {inside isDone()}++++++++++++++\n";
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: " +
            std::string("15") + "\r\n"
                                "Connection: keep-alive\r\n"
                                "\r\n"
                                "<h1>Helllo!</h1>";
        send(_fd, response.c_str(), response.size(), 0);
        //std::cout << "send fail\n";
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

