#include "../include/HttpResponse.hpp"

HttpResponse::HttpResponse(const HttpRequest &request) : _request(request)
{
    _bodyiscompletl = false;
    _checkDespostionIfExist = false;
    _chunkSize = 0;
    _readbodySize = 0;
    _isNewfile = true;
    _waitingToSend = false;
    _headerSent = false;
}
HttpResponse::~HttpResponse()
{
    
}

int HttpResponse::getStatus() const
{
    return _statusCode;
}

std::string HttpResponse::getReasonPhrase() const
{
    return _reasonPhrase;
}

std::string HttpResponse::getBody() const
{
    return _body;
}

void HttpResponse::successfullrequest(std::string path,int fd)
{
    std::stringstream ss;
    size_t size;
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open())
    {
        _reqProcessor->sendErrorResponse(404, fd);
        return;
    }
    file.seekg(0, std::ifstream::end);
    size = file.tellg();
    file.seekg(0, std::ifstream::beg);
    std::vector<char> buffer(size);
    file.read(buffer.data(), size);
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: text/html\r\n";
    ss << "Content-Length: " << size << "\r\n";
    ss << "Connection: close\r\n\r\n";
    std::string response = ss.str();
    send(fd, response.c_str(), response.size(), 0);
    send(fd, buffer.data(), size, 0);
}

void HttpResponse::setBoundary()
{
    std::string header;
    size_t pos = 0;
    header =  _request.getHeader();
    pos = header.find("Content-Type");
    if (pos != std::string::npos)
    {
        pos = header.find("boundary=", pos);
        size_t end = header.find("\r\n", pos);
        if (pos != std::string::npos)
            _boundary = "--" + header.substr(pos + 9,end);
    }
}

void       HttpResponse::setReqProcessor(RequestProcessor *processor)
{
    _reqProcessor = processor;
}

void HttpResponse::decodeChunked(int fd)
{
    size_t          line_end; 
    std::string     filename;
    size_t          pos;
    while (true)
    {
        if (_chunkSize == 0)
        {
            line_end = _body.find("\r\n");
            if (line_end == std::string::npos)
                return;
            char* endptr = 0;
            std::string hex = _body.substr(0, line_end);
            _chunkSize = std::strtol(hex.c_str(), &endptr, 16);
            _readbodySize += _chunkSize;
            _body.erase(0, line_end + 2);
            if (_isNewfile)
            {
                filename =  getFilename();
                if (filename.empty())
                    return;
                pos = _body.find("\r\n\r\n");
                if (pos == std::string::npos)
                    return ;
                _isNewfile =   false;
                _body.erase(0, pos + 4);
                _chunkSize -= pos + 4;
                _file.open(filename.c_str());
                if (!_file.is_open())
                {
                _reqProcessor->sendErrorResponse(403,fd);
                 return;
                }
            }
            if (_chunkSize == 0)
            {
                _bodyiscompletl = true;
                _body.clear();
                break;
            }
        }
        pos = _body.find(_boundary + "--");
        if (pos !=std::string::npos)
        {
            _file.write(_body.data(),pos);
            _bodyiscompletl = true;
            break;
        }
        size_t sizeWrite = std::min(_body.size(),_chunkSize);
        _file.write(_body.data(), sizeWrite);
        _body.erase(0, sizeWrite);
        _chunkSize -= sizeWrite;
        if (_chunkSize > 0)
            return;
        if (_body.size() < 2)
            return;
        _body.erase(0, 2);
    }
    if (_bodyiscompletl)
    {
        if (_reqProcessor->isCGI())
            execut_cgi (fd,_request.getFullpath(),"/bin/python3");
        else
            successfullrequest("static/seccefull.html",fd);
        return;
    }
}

std::string HttpResponse::getFilename()
{
    std::string     filename;
    size_t          contentPos;
    if (_reqProcessor->isCGI())
        return "/tmp/cgioutput";
    contentPos = _body.find("Content-Disposition:");
    if (contentPos != std::string::npos)
    {
        size_t filenamePos = _body.find("filename=", contentPos);
        if (filenamePos != std::string::npos)
        {
            size_t start    = _body.find("\"", filenamePos);
            size_t end  = _body.find("\"", start + 1);
            filename =  _body.substr(start+ 1, end- start- 1);
        }
    }
    if (filename.empty())
        return "";
    return _request.getFullpath() +  filename;
}

void HttpResponse::handle_post(int fd)
{
    std::string filename;
    size_t pos;
     std::string buffer = _request.getReadBuffer();
    _body.append(buffer);
    setBoundary();
    if (_request.getKeyValue("Transfer-Encoding") == "chunked")
    {
        decodeChunked(fd);
        return;
    }
    _readbodySize += buffer.size();
    while (true)
    {
        if (_isNewfile)
        {
            filename =  getFilename();
            if (filename.empty())
                return;
            _isNewfile =  false;
            _file.open(filename.c_str());
            pos = _body.find("\r\n\r\n");
            if (pos == std::string::npos)
                return ;
            if (!_file.is_open())
            {
                _reqProcessor->sendErrorResponse (403,fd);
                return;
            }
            if (_request.getCGI())
                _file.write(_body.c_str(),pos + 4);
            _body.erase(0, pos + 4);
        }
        pos = _body.find (_boundary);
        if (pos != std::string::npos)
        {
            _isNewfile = true;
            _file.write (_body.data(),pos);
            _body.erase(0,pos);
            _file.close();
            pos = _body.find (_boundary + "--");
            if (pos != std::string::npos)
                _bodyiscompletl = true;
        }
        else
        {
            size_t endbody = _body.size () - _boundary.size();
            _file.write (_body.data(),endbody);
            pos = _body.find (_boundary);
            _body.erase(0,endbody);
            if (_readbodySize >= _contentLenght)
            {
                _bodyiscompletl = true;
                _file.write (_body.data(),_body.size());
                _file.close();
            }
            else
                return;
        }
        if (_bodyiscompletl)
        {
            if (_request.getCGI())
                execut_cgi (fd,_request.getFullpath(),_request.getCGIPath());
            else
            {
                successfullrequest("static/seccefull.html",fd);
            }
            return;
        }
    }
}

std::string HttpResponse::check_extation(std::string path)
{
    std::string str;
    size_t pos = path.find_last_of(".");
    if (pos == std::string::npos)
        return "";
    path.erase(0, pos);
    std::string image_extension = ".jpg .jpeg .png .gif .bmp .tiff .webp .svg";
    std::string document_extension = ".pdf .doc .docx .xls .xlsx .ppt .pptx";
    std::string audio_extension = ".mp3 .wav .ogg .flac .aac .m4a";
    std::string video_extension = ".mp4 .avi .mov .mkv .webm .flv .wmv";
    std::string archive_extension = ".zip .rar .tar .gz .7z";
    std::string executable_extension = ".apk .exe .dmg";
    path.erase(0, 1);
    if (image_extension.find(path) != std::string::npos)
        str = "image/" + path;
    else if (audio_extension.find(path) != std::string::npos)
        str = "audio/" + path;
    else if (video_extension.find(path) != std::string::npos)
        str = "video/" + path;
    else if (archive_extension.find(path) != std::string::npos)
        str = "archive" + path;
    else if (executable_extension.find(path) != std::string::npos)
        str = "executable" + path;
    else
        str = "text/" + path;
    return str;
}

bool is_file(std::string path)
{
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
    return S_ISREG(path_stat.st_mode);
}

void HttpResponse::sendAutoIndexResponse(int fd, const std::string &dir_path)
{
    DIR *dir = opendir(dir_path.c_str());
    if (!dir)
    {
       _reqProcessor->sendErrorResponse(500,fd);
        return;
    }
    std::stringstream body;
    body << "<html><head><title>Index of " << _request.getFullpath() << "</title></head><body>";
    body << "<h1>Index of " << dir_path << "</h1><ul>";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
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

    send(fd, response.str().c_str(), response.str().length(), 0);
}

void HttpResponse::handle_get(int fd, std::string path)
{
    std::stringstream header;
    std::string str;
    
    if (_reqProcessor->useAutoIndex())
    {
        sendAutoIndexResponse (fd,path);
        return;
    }
    str = check_extation(path);
    if (_reqProcessor->isCGI())
    {
        execut_cgi(fd,_request.getFullpath(), _request.getCGIPath());
        return;
    }
    if (access(path.c_str(), F_OK) == -1)
    {
        _reqProcessor->sendErrorResponse(404,fd);
        return;
    }
        _infile.open(path.c_str(), std::ios::binary);
        if (!_infile.is_open() || !is_file(path))
        {
            _reqProcessor->sendErrorResponse(403,fd);
            return;
        }
        _infile.seekg(0, std::ifstream::end);
        _fileSize = _infile.tellg();
        _infile.seekg(0, std::ifstream::beg);
        header << "HTTP/1.1 200 OK\r\nContent-Type: " << str << "\r\n";
        header << "Content-Length: " << _fileSize << "\r\n";
        header << "Connection: close\r\n\r\n";
        str = header.str();
        send(fd, str.c_str(), str.length(), 0);
        _firstTime = false;
    // _infile.read(buf, sizeof(buf));
    // size_t bytes_read = _infile.gcount();
    // _sendBuffer.append(buf, bytes_read);
    _headerSent = true;
    _waitingToSend = true;
    


    // send(fd, buf, bytes_read, 0);
    // _readSize += bytes_read;
    // if (_readSize >= _fileSize)
    // {
    //     //std::cout <<"------------------------- conction is closed -------------------------------------------------------------\n";
    //   _infile.close();
    // }
}

void  HttpResponse::handle_delete (int fd)
{
    std::string path = _request.getFullpath();
    if (access(path.c_str(), F_OK) == -1)
    {
        _reqProcessor->sendErrorResponse(404,fd);
        return;
    }
    if (!is_file(path))
    {
        _reqProcessor->sendErrorResponse(403,fd);
        return;
    }
    if (access(path.c_str(), W_OK | X_OK) == -1)
    {
        _reqProcessor->sendErrorResponse(403,fd);
        return;
    }
    int result = unlink(path.c_str());
    if (result == -1)
    {
        _reqProcessor->sendErrorResponse(403,fd);
        return;
    }
    successfullrequest("static/seccefullDelete.html",fd);
}

void HttpResponse::process(int fd)
{
    if (_request.getMethod() == "GET")
        handle_get(fd, _request.getFullpath());
    else if (_request.getMethod() == "POST")
        handle_post(fd);
    else if (_request.getMethod() == "DELETE")
        handle_delete (fd);
}

