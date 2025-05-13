#include "./include/HttpResponse.hpp"

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

std::string HttpResponse::buildResponse()
{
    if (_request.getMethod() == "GET")
    {
        _body = Tools::readFile(_request.getPath());
    }
    // I STOPED HERE CUASE I NEED THE ROUTE LOCATION AND OTHER ATTRIBUTE
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
    }
    return "Unknown Success";
}

void HttpResponse::handel_post(int fd, std::vector<std::string> method, std::string request)
{
    std::string str;
    std::string filename;
    size_t pos = request.find("\r\n\r\n");

    if (pos != std::string::npos)
        _body = request.substr(pos + 5, request.length() - 1);
    size_t contentDispositionPos = _body.find("Content-Disposition:");
    if (contentDispositionPos != std::string::npos)
    {
        size_t filenamePos = _body.find("filename=", contentDispositionPos);
        if (filenamePos != std::string::npos)
        {
            size_t startQuote = _body.find("\"", filenamePos);
            size_t endQuote = _body.find("\"", startQuote + 1);
            filename = _body.substr(startQuote + 1, endQuote - startQuote - 1);
        }
    }
    pos = _body.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        str = _body.substr(pos + 10);
        std::cout << str << std::endl;
        std::cout << _body << std::endl;
    }
    std::ofstream file(filename.c_str());
    if (!file.is_open())
    {
        std::cout << "File already exists" << std::endl;
        return;
    }
    file << str;
    (void)method;
    std::string body = "{\"txt\":\"Upload successful and all it is good\"}";
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: txt/txt\r\n";
    ss << "Content-Length: " << body.size() << "\r\n";
    ss << "Connection: close\r\n\r\n";
    ss << body;

    std::string response = ss.str();
    send(fd, response.c_str(), response.size(), 0);
}

std::string HttpResponse::check_extation(std::string path)
{
    std::string str;
    size_t pos = path.find_last_of(".");
    if (pos == std::string::npos)
        return "";
    path.erase(0, pos);
    std::string image_extension = ".jpg .jpeg .png .gif .bmp .tiff .webp .svg";
    std::string text_extension = ".txt .html .css .js .json .xml .csv .rtf .odt .php .py .java .cpp .sql .bat";
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

void HttpResponse::handel_get(int fd, std::vector<std::string> method)
{
    std::string str = check_extation(method[1]);
    std::string header = "HTTP/1.1 200 OK\r\nContent-Type: " + str + "\r\n\r\n";
    send(fd, header.c_str(), header.length(), 0);
    std::ifstream file;
    if (method[1] == "/")
        file.open("index.html", std::ios::binary);
    else
        file.open(method[1].c_str(), std::ios::binary);
    std::stringstream ss;
    if (!file.is_open())
        file.open("404.html", std::ios::binary);
    ss << file.rdbuf();
    file.seekg(0, file.end);
    size_t pos = file.tellg();
    send(fd, ss.str().c_str(), pos, 0);
    file.close();
    return;
}