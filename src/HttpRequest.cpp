#include "../include/HttpRequest.hpp"

HttpRequest::HttpRequest() : _isHeaderReady(false), _isBodyReady(false), _isHeadeParse(false), _isReqValid(false)
{
}

HttpRequest::~HttpRequest()
{
}

std::string HttpRequest::getMethod() const
{
    return _method;
}

std::string HttpRequest::getVersion() const
{
    return _version;
}

std::string HttpRequest::getPath() const
{
    return _path;
}

void HttpRequest::getNextRequest()
{
    // this function to handle the recv if get multi request
    size_t offset;
    size_t n;

    if (!_isHeaderReady)
    {
        _readBuffer = "";
    }
    else if (_method == "POST" && _isBodyReady)
    {
        offset = _readBuffer.find("\r\n\r\n") + 4;
        n = atoi(getKeyValue("Content-Length").c_str());
        _readBuffer = _readBuffer.substr(offset + n, _readBuffer.size() - offset - n);
    }
    else
    {
        offset = _readBuffer.find("\r\n\r\n") + 4;
        if (offset != std::string::npos)
            _readBuffer = _readBuffer.substr(offset, _readBuffer.size() - offset);
    }
}

void HttpRequest::reset()
{
    getNextRequest();
    _isHeaderReady = false;
    _isHeadeParse = false;
    _isBodyReady = false;
    _isReqValid = false;
    _headerMap.clear();
    _header = "";
    _method = "";
    _version = "";
    _path = "";
    _body = "";
}

std::string HttpRequest::getKeyValue(const std::string &key) const 
{
    std::map<std::string, std::string>::const_iterator it = _headerMap.find(key);
    if (it == _headerMap.end())
        return "";
    return it->second;
}

void HttpRequest::addReadBuffer(const char *buffer, int n)
{
    _readBuffer.insert(_readBuffer.end(), buffer, buffer + n);
}

bool HttpRequest::isHeaderComplete()
{
    size_t read_offset;

    if (_isHeaderReady)
        return true;
    read_offset = _readBuffer.find("\r\n\r\n");
    if (read_offset != std::string::npos)
    {
        _header = _readBuffer.substr(0, read_offset);
        _readBuffer = _readBuffer.erase(0, read_offset + 4);
        std::cout << "$$$$$$$$$$$$$$$$$$$ { ReadBuffer : " << _readBuffer << " } $$$$$$$$$$$$$$$$$$$$$$$";
        _isHeaderReady = true;
        return true;
    }
    reset();
    return false;
}

bool HttpRequest::isBodyComplete()
{
    char *endptr;
    size_t n;

    if (_method != "POST")
        return false;
    std::string content_lenght = getKeyValue("Content-Length");
    if (content_lenght.empty())
        return false;
    n = strtol(content_lenght.c_str(), &endptr, 10);
    if (*endptr == '\0' || static_cast<long> (n) < 0)
        return false;
    if (_body.size() < n)
        return false;
    return true;
}

bool HttpRequest::isMethodSpacesValid()
{
    int offset;
    int offset2;

    offset = _header.find('\n');
    offset2 = _header.find("\r\n");
    std::string line = _header.substr(0, offset);
    if (countChar(line, ' ') != 2 || offset2 > offset)
        return false;
    return true;
}

bool HttpRequest::isHeaderLineValid(const std::string &line)
{
    size_t index = line.find(':');
    size_t index2 = line.find(' ');
    size_t index3 = line.find("\r");

    if (index == std::string::npos || index3 == std::string::npos)
        return false;
    if (index3 != std::string::npos && line.size() - 1 != index3)
        return false; // if \r exist but not in the end
    if (index2 != std::string::npos && index > index2)
        return false;
    _headerMap[line.substr(0, index)] = line.substr(index + 1, index3 - index - 1);
    return true;
}

bool HttpRequest::fillHeaderMap(std::string headers)
{
    size_t offset;
    std::string line;

    offset = headers.find('\n');
    if (offset == std::string::npos)
        return false;
    line = headers.substr(0, offset);
    headers = headers.substr(offset + 1);
    while (!line.empty() && offset != std::string::npos)
    {
        // check Syntax of every line
        if (!isHeaderLineValid(line))
            return false;
        // get next line
        offset = headers.find('\n');
        line = headers.substr(0, offset);
        headers = headers.substr(offset + 1);
    }
    return true;
}

bool HttpRequest::parseHeader()
{
    size_t line_end;
    size_t offset;
    size_t offset2;

    if (_isHeadeParse)
        return true;
    if (!isMethodSpacesValid())
    {
        reset();
        return false;
    }

    line_end = _header.find("\r\n");
    std::string line = _header.substr(0, line_end);
    
    offset = line.find(' ');
    _method = line.substr(0, offset);
    
    offset2 = line.find(' ', offset + 1);
    _path = line.substr(offset + 1, offset2 - offset - 1);

    _version = line.substr(offset2 + 1);

    if (!fillHeaderMap(_header.substr(line_end + 2)))
    {
        reset();
        return false;
    }

    _isHeadeParse = true;
    return true;
}

bool HttpRequest::checkBodyIsReady()
{
    size_t  offset;
    size_t  n;
    char    *endptr;

    if (_method == "POST" && !_isBodyReady)
    {
        offset = _readBuffer.find("\r\n\r\n") + 4;
        n = strtol(getKeyValue("Content-Length").c_str(), &endptr, 10);
        if (*endptr != '\0' || static_cast<long>(n) < 0) 
            return false;
        if (_readBuffer.size() - offset < n)
        {
            _isBodyReady = false;
            return false;
        }
        _body = _readBuffer.substr(offset, _readBuffer.size() - offset);
    }
    _isBodyReady = true;
    return true;
}

int HttpRequest::countChar(const std::string &str, char ch)
{
    int count = 0;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == ch)
            ++count;
    }
    return count;
}

bool HttpRequest::isDone() // is everything in the request is done like post with body or get with just  a valid header
{
    if (_isHeaderReady && _isHeadeParse  && _isReqValid) // &&_isBodyReady)
        return true;
    return false;
}

void        HttpRequest::setIsReqValid(bool status)
{
    _isReqValid = status;
}