#include "../include/HttpError.hpp"

HttpError::HttpError(int code) : _code(code)
{
    setMessage();
}

void HttpError::setMessage()
{
    switch (_code)
    {
        case 400: _message = "Bad Request"; break;
        case 401: _message = "Unauthorized"; break;
        case 403: _message = "Forbidden"; break;
        case 404: _message = "Not Found"; break;
        case 405: _message = "Method Not Allowed"; break;
        case 413: _message = "413 Payload Too Large"; break;
        case 411: _message = "411 Length Required"; break;
        case 500: _message = "Internal Server Error"; break;
        case 502: _message = "Bad Gateway"; break;
        case 503: _message = "Service Unavailable"; break;
        default:  _message = "Unknown Error"; break;
    }
}

int HttpError::getCode() const
{
    return _code;
}

std::string HttpError::getMessage() const
{
    return _message;
}

std::string HttpError::getStatusLine() const
{
    std::stringstream ss;

    ss << _code;
    return "HTTP/1.1 " + ss.str() + " " + _message;
}
