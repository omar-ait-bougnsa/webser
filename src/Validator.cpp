#include "../include/Validator.hpp"

// class HttpRequest;

Validator::Validator() : _status_code(0) {}
Validator::~Validator() {}

// ✅ Check if method is valid (GET, POST...) and version is HTTP/1.1
bool Validator::validateRequestLine(const HttpRequest& request)
{
    std::string method = request.getMethod();
    std::string version = request.getVersion();

    if (method != "GET" && method != "POST" && method != "DELETE")
    {
        std::cerr << "405 Method Not Allowed\n";
        _status_code = 405;
        return false;
    }

    if (version != "HTTP/1.1")
    {
        std::cerr << "505 HTTP Version Not Supported\n";
        _status_code = 505;
        return false;
    }

    return true;
}

// ✅ Check required headers (Host, Content-Length if POST...)
bool Validator::validateHeaders(const HttpRequest& request)
{
    char *endptr;
    std::string host = request.getKeyValue("Host");
    if (host.empty())
    {
        std::cerr << "400 Bad Request: Host header missing";
        _status_code = 400;
        return false;
    }

    // If POST, Content-Length is mandatory
    if (request.getMethod() == "POST")
    {
        std::string contentLength = request.getKeyValue("Content-Length");
        if (contentLength.empty())
        {
            std::cerr << "411 Length Required";
            _status_code = 411;
            return false;
        }

        // Optional: Check if Content-Length is a valid number
        long length = strtol(contentLength.c_str(), &endptr, 10);
        if (*endptr != '\0' || length < 0 || length > 1000000) // limit 1MB for example
        {
            std::cerr << "413 Payload Too Large";
            _status_code = 413;
            return false;
        }
        // maybe i will need to add also connection haeder
    }

    return true;
}

// ✅ Full validation
bool Validator::validate(const HttpRequest& request, int &status)
{
    status = _status_code;
    
    return validateRequestLine(request) &&
           validateHeaders(request);
}
