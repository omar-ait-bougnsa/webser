#include "../include/HttpResponse.hpp"
#include "../include/cgi.hpp"


cgi::cgi()
{
    headerSend = false;
    contentLength = -1;
    readbytes = 0;
}
void cgi::setCgiheader(std::string header)
{
    size_t pos;
    while (1)
    {
        pos = header.find("\r\n");
        if (pos == std::string::npos)
            break;
        std::string line = header.substr(0, pos);
        header.erase(0, pos + 2);
        if (line.empty())
            break;
        pos = line.find(":");
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            _cgiHeader.insert(std::make_pair(key, value));
        }
    }
}

std::string cgi::getValuKey(std::string key)
{
    std::map<std::string, std::string>::iterator it = _cgiHeader.find(key);
    if (it != _cgiHeader.end())
        return it->second;
    return "";
}

std::vector<char*> HttpResponse::set_env(std::string filePath)
{
    std::vector<std::string> temp_env;
    std::vector<char*> env;

    temp_env.push_back("SCRIPT_NAME = " + filePath); 
    temp_env.push_back("REQUEST_METHOD=" + _request.getMethod());
    temp_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
    std::string cookie = _request.getKeyValue("Cookie");
    if (!cookie.empty())
        temp_env.push_back("HTTP_COOKIE=" + cookie);
    if (_request.getMethod() == "POST")
    {
        temp_env.push_back("CONTENT_LENGTH=" + Tools::int_to_string(_readbodySize));
        temp_env.push_back("CONTENT_TYPE=" + getContentType());
    }

    for (size_t i = 0; i < temp_env.size(); ++i)
    {
        char* env_entry = strdup(temp_env[i].c_str());
        env.push_back(env_entry);
    }
    env.push_back(NULL);
    return env;
}
std::string HttpResponse::getContentType()
{
    std::string str;
    std::string header = _request.getHeader();
    size_t pos = header.find("Content-Type:");
    size_t end = header.find("\r\n");
    if (pos != std::string::npos && end !=std::string::npos)
        str = header.substr(pos +14);
    return str;
}

void cgi::sendHeader(int fd,std::string buffer)
{

    size_t pos = buffer.find("\r\n\r\n");
    if (pos == std::string::npos)
        pos = buffer.find ("\n\n");
    std::string header_part;
    if (pos != std::string::npos)
    {
        header_part = buffer.substr(0, pos);
        buffer.erase(0, pos + 2);
        setCgiheader(header_part);
    }
    std::stringstream header;
    header << "HTTP/1.1 200 OK\r\n";
    if (getValuKey("Content-Type") == "")
        header << "Content-Type: text/html\r\n";
    else
        header << "Content-Type: " <<getValuKey("Content-Type") << "\r\n";

    std::string setCookie = getValuKey("Set-Cookie");
    if (!setCookie.empty())
        header << "Set-Cookie: " << setCookie << "\r\n";
    std::string str = getValuKey("Content-Type");
    if (str != "")
    {
        header << "Content-Length: " << str << "\r\n";
        contentLength = std::strtol(str.c_str(), NULL, 16);
    }
    header << "Connection: close\r\n\r\n";
    std::string response = header.str() + buffer;
    send(fd, response.c_str(), response.size(), 0);
    headerSend = true;

}
bool cgi::sendResponse(int fd)
{
    std::cout << "send is exicut "<<std::endl;
    // char buffer[60000];
    // ssize_t bytesRead = read(pip[0], buffer, sizeof(buffer));
    // readbytes += bytesRead;
    // if (bytesRead <= 0)
    //     return 1;
    // if (!headerSend)
        // sendHeader(fd,buffer);
    // else if (contentLength != 1)
    // {
    //     size_t sent = std::min(readbytes, contentLength - readbytes);
    //     send(fd,buffer,sent,0);
    //     if (readbytes >= contentLength)
    //         return (1);
    // }
    (void)fd;
     return (0);
}
void HttpResponse::cgi_response(int fd, std::string body)
{
    cgi obj;

    size_t pos = body.find("\r\n\r\n");
    if (pos == std::string::npos)
        pos = body.find ("\n\n");
    std::string header_part;
    if (pos != std::string::npos)
    {
        header_part = body.substr(0, pos);
        body.erase(0, pos + 2);
        obj.setCgiheader(header_part);
    }

    std::stringstream header;
    header << "HTTP/1.1 200 OK\r\n";
    if (obj.getValuKey("Content-Type") == "")
        header << "Content-Type: text/html\r\n";
    else
        header << "Content-Type: " << obj.getValuKey("Content-Type") << "\r\n";

    std::string setCookie = obj.getValuKey("Set-Cookie");
    if (!setCookie.empty())
        header << "Set-Cookie: " << setCookie << "\r\n";
    header << "Content-Length: " << body.size() << "\r\n";
    header << "Connection: close\r\n\r\n";
    std::string response = header.str() + body;
    send(fd, response.c_str(), response.size(), 0);
}

void HttpResponse::execut_cgi(int fd, std::string filePath, std::string cgi_path)
{
    // int status = 0;
    std::string body;
    // char buffer[60000];
    _cgi.time = time(NULL);
    std::vector<char *> env = set_env(filePath);
    if (access(cgi_path.c_str(), X_OK) == -1)
    {
        _reqProcessor->sendErrorResponse(403,fd);
        return;
    }
    if (access(filePath.c_str(), F_OK) == -1)
    {
        _reqProcessor->sendErrorResponse(404,fd);
        return;
    }
    if (pipe(_cgi.pip) == -1 || pipe(_cgi.input) == -1)
        exit(1);
    _cgi.pid = fork();
    if (_cgi.pid == -1)
    {
        std::cout << "fork failed\n";
        exit(1);
    }
    if (_cgi.pid == 0)
    {
        if (dup2(_cgi.pip[1], 1) == -1 || dup2(_cgi.input[0],0) == -1)
            exit(1);
        std::string dir = filePath.substr(0, filePath.find_last_of("/"));
        if (chdir(dir.c_str()) == -1)
            exit(1);
        std::vector<const char *> argv;
        argv.push_back(cgi_path.c_str());
        argv.push_back(filePath.c_str());
        close(_cgi.pip[0]);
        close (_cgi.pip[1]);
        close(_cgi.input[0]);
        close (_cgi.input[1]);
        if (execve(cgi_path.c_str(), const_cast<char **>(argv.data()),const_cast<char **>(env.data())) == -1)
                exit(0);
    }
    if (_request.getMethod() == "POST")
    {
       _cgi.file.open("/tmp/cgioutput", std::ios::binary);
        if (!_cgi.file.is_open())
        {
            _reqProcessor->sendErrorResponse(403,fd);
            return;
        }
    }
    close(_cgi.input[0]);
    close(_cgi.pip[1]);
    _waitingToSend = true;
    // waitpid(_cgi.pid, &status,1);
    // if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
    // {
    //     _reqProcessor->sendErrorResponse(403,fd);
    //     return;
    // }
    // while (1)
    // {
    //     ssize_t bytesRead = read(_cgi.pip[0], buffer, sizeof(buffer));
    //     if (bytesRead <= 0)
    //         break;
    //     body.append(buffer, bytesRead);
    // }
    //close(_cgi.pip[0]);
   // cgi_response(fd, body);
}