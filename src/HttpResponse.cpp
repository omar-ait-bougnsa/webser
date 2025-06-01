#include "../include/HttpResponse.hpp"
#include <sys/wait.h>
#include <unistd.h>

// HttpResponse::HttpResponse ()
// {

// }
HttpResponse::HttpResponse(const HttpRequest &request) : _request(request)
{
}
HttpResponse::~HttpResponse()
{
}

void HttpResponse::setStatus(int code)
{
    _statusCode = code;
    _reasonPhrase = getDefaultReasonPhrase(code);
}

void HttpResponse::setHeader(const std::string &key, const std::string &value)
{
    _headers[key] = value;
}

void HttpResponse::setBody(const std::string &body)
{
    _body = body;
}

int             HttpResponse::getStatus() const
{
    return _statusCode;
}

std::string HttpResponse::getReasonPhrase() const
{
    return _reasonPhrase;
}

std::string HttpResponse::getHeader(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end())
        return it->second;
    return "";
}

std::string HttpResponse::getBody() const
{
    return _body;
}

std::string HttpResponse::getDefaultReasonPhrase(int code)
{
    switch (code)
    {
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 204:
        return "No Content";
    default:
    return "Unknown Success";
    }
    return "Unknown Success";
}

void HttpResponse::_error_403(int fd)
{
    std::ifstream   file ("static/errors/403.html",std::ios::binary);
    std::string     line;
    if (!file.is_open())
    {
        std::cout << "error: can't open file 404";
        return;
    }
    file.seekg(0, std::ifstream::end);
    size_t size = file.tellg();
    file.seekg(0, std::ifstream::beg);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    std::stringstream ss;
    ss << "HTTP/1.1 403  Forbidden \r\n";
    ss << "Content-Type: text/html\r\n";
    ss << "Content-Length: " << size << "\r\n";
    ss << "Connection: close\r\n\r\n";
    std::string response = ss.str();
    send(fd, response.c_str(), response.size(), 0);
    send(fd, buffer.data(), size, 0);
}

void HttpResponse::_erro_404(int fd)
{
    std::ifstream   file ("static/errors/404.html",std::ios::binary);
    std::string     line;
    if (!file.is_open())
    {
        std::cout << "error: can't open file 404";
        return;
    }
    file.seekg(0, std::ifstream::end);
    size_t size = file.tellg();
    file.seekg(0, std::ifstream::beg);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    std::stringstream ss;
    ss << "HTTP/1.1 404  Not Found \r\n";
    ss << "Content-Type: text/html\r\n";
    ss << "Content-Length: " << size << "\r\n";
    ss << "Connection: close\r\n\r\n";
    std::string response = ss.str();
    send(fd, response.c_str(), response.size(), 0);
    send(fd, buffer.data(), size, 0);
}

void HttpResponse::successfullPOST(int fd)
{
    std::stringstream   ss;
    size_t              size;
    std::ifstream file("static/seccefull.html",std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "can't open file\n";
        exit (1);
    }
    file.seekg(0, std::ifstream::end);
    size = file.tellg();
    file.seekg(0, std::ifstream::beg);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: txt/html\r\n";
    ss << "Content-Length: " << size << "\r\n";
    ss << "Connection: close\r\n\r\n";
    std::string response = ss.str();
    send(fd, response.c_str(), response.size(), 0);
    send (fd,buffer.data(),size,0);
}
void HttpResponse::handel_post(int fd)
{
    // std::string     str;
    // std::string     filename;
    // size_t          pos;
    // size_t          contentPos;
    _body = _request.getBody();
    std::cout << _body <<std::endl;
    // contentPos = _body.find("Content-Disposition:");
    // if (contentPos != std::string::npos)
    // {
    //     size_t filenamePos = _body.find("filename=", contentPos);
    //     if (filenamePos != std::string::npos)
    //     {
    //         size_t startQuote = _body.find("\"", filenamePos);
    //         size_t endQuote = _body.find("\"", startQuote + 1);
    //         filename = _body.substr(startQuote + 1, endQuote - startQuote - 1);
    //     }
    // }
    // pos = _body.find("boundary=");
    // if (pos != std::string::npos)
    //     _boundary = "--" + _body.substr(pos + 10,_body.find("\r\n"));
    // pos = _body.find("\r\n\r\n");
    // if (pos != std::string::npos)
    //     str = _body.substr(pos + 4);
    // std::ofstream file(filename.c_str());
    // if (!file.is_open())
    // {
    //     _error_403(fd);
    //     return;
    // }
    // file << str;
     successfullPOST(fd);
}

std::string HttpResponse::check_extation(std::string path)
{
    std::string str;
    size_t pos = path.find_last_of(".");
    if (pos == std::string::npos)
        return "";
    path.erase(0, pos);
    std::string image_extension = ".jpg .jpeg .png .gif .bmp .tiff .webp .svg";
    std::string text_extension = ".txt .c .html .css .js .json .xml .csv .rtf .odt .php .py .java .cpp .sql .bat";
    std::string document_extension = ".pdf .doc .docx .xls .xlsx .ppt .pptx";
    std::string audio_extension = ".mp3 .wav .ogg .flac .aac .m4a";
    std::string video_extension = ".mp4 .avi .mov .mkv .webm .flv .wmv";
    std::string archive_extension = ".zip .rar .tar .gz .7z";
    std::string executable_extension = ".apk .exe .dmg";
    path.erase(0, 1);
    if (image_extension.find(path) != std::string::npos)
        str = "image/" + path;
    else if (text_extension.find(path) != std::string::npos)
        str = "text/" + path;
    else if (audio_extension.find(path) != std::string::npos)
        str = "audio/" + path;
    else if (video_extension.find(path) != std::string::npos)
        str = "video/" + path;
    else if (archive_extension.find(path) != std::string::npos)
        str = "archive" + path;
    else if (executable_extension.find(path) != std::string::npos)
        str = "executable" + path;
    return str;
}
std::vector<const char *> set_env(std::string filePath)
{
    std::vector<const char *> env;
    std::string str = "SCRIPT_NAME=" + filePath;
    env.push_back("REQUEST_METHOD=GET");
    env.push_back(str.c_str());
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");

    return (env);
}
void HttpResponse::execut_cgi(int fd,std::string filePath,std::string cgi_path)
{
    int pid;
    cgi_path = "/usr/bin/python";
    int status = 0;
    std::string body;
    char buffer[4096000];
    int pip[2];

    std::vector<const char *> env = set_env(filePath);
    std::cout << "env = " <<env[0] <<std::endl;
    if (access(cgi_path.c_str(), X_OK) == -1)
    {
        _error_403(fd);
        return;
    }
    if (access(filePath.c_str(),F_OK) == -1)
    {
        _erro_404(fd);
        return;
    }
    if (pipe(pip) == -1)
        exit (1);
    pid = fork();
    if (pid == -1)
    {
        std::cout << "fork failed\n";
        exit(1);
    }
    if (pid == 0)
    {
        if (dup2(pip[1],1) == -1)
           exit (1);
        std::vector<const char*> argv;
        argv.push_back(cgi_path.c_str());
        argv.push_back(filePath.c_str());
        close(pip[0]);
        if (execve (cgi_path.c_str(),const_cast<char **>(argv.data()),const_cast<char **>(env.data())) == -1)
        {
            std::cout << "execve failed\n";
            exit (1);
        }
    }
    waitpid (pid,&status,0);
    close(pip[1]);
    while (1)
    {
        ssize_t bytesRead = read(pip[0], buffer, sizeof(buffer));
        if (bytesRead <= 0)
            break;
        body.append(buffer, bytesRead);
    }
    close(pip[0]);
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: text/html\r\n";
    ss << "Content-Length: " << body.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    std::string response = ss.str();
    send(fd, response.c_str(), response.size(), 0);
    send(fd, body.c_str(), body.size(), 0);
}

void HttpResponse::handel_get(int fd,std::string path)
{
    std::stringstream   header;
    std::string         str;
    char                buf[1024];
    str  = check_extation(path);
    if (str == "text/py")
    {
        execut_cgi(fd,path,"/usr/bin/python");
            return;
    }
    std::ifstream file;
    file.open(path.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        _erro_404(fd);
        return;
    }

    file.seekg(0, std::ifstream::end);
    size_t size = file.tellg();
    file.seekg(0, std::ifstream::beg);
    std::cout <<"here is working yes \n\n\n\n";
    header << "HTTP/1.1 200 OK\r\nContent-Type: " << str << "\r\n";
    header << "Content-Length: " << size << "\r\n";
    header << "Connection: close\r\n\r\n";

    str = header.str();
    send(fd,str.c_str(),str.length(),0);
    while (1)
    {
        file.read(buf, sizeof(buf));
        std::streamsize bytes_read = file.gcount();
        if (bytes_read > 0)
        {
            ssize_t sent = send(fd, buf, bytes_read, 0);
            if (sent == -1)
            {
                perror("\n\nsend failed\n\n");
                break;
            }
        }
        if (file.eof())
            break;
    }
    file.close();
}
void   HttpResponse::process(int fd)
{
    if (_request.getMethod() == "GET")
        handel_get (fd,_request.getFullpath());
    else if (_request.getMethod() == "POST")
        handel_post(fd);
    else
         std::cout <<"handel_delet";
}