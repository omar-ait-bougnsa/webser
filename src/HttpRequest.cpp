#include "../include/HttpRequest.hpp"

HttpRequest::HttpRequest() : _isHeaderReady(false), _isBodyReady(false), _isHeadeParse(false), _isReqValid(false), _isCGI(false), _isFirstLineReqValid(false)
{
}

HttpRequest::~HttpRequest()
{
}
std::string HttpRequest::getFullpath() const
{
    return _fullpath;
}
void HttpRequest::setFullpath(std::string path)
{
    _fullpath = path;
}
std::string HttpRequest::getHeader() const
{
    return _header;
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

void HttpRequest::setCGIPath(std::string path)
{
    _CGIpath = path;
}

std::string HttpRequest::getCGIPath () const
{
    return _CGIpath;
}

std::string HttpRequest::getReadBuffer() const
{
    return _readBuffer;
}

bool HttpRequest::getCGI() const
{
    return _isCGI;
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
    // getNextRequest();
    _isHeaderReady = false;
    _isHeadeParse = false;
    _isBodyReady = false;
    _isReqValid = false;
    _isFirstLineReqValid = false;
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

void HttpRequest::addReadBuffer(char *buffer, int n)
{

    if (!_isHeadeParse)
    {

        for (int i = 0; i < n; ++i)
        {
            if (buffer[i] == '\n')
            {
                if (_readBuffer.empty() || _readBuffer[_readBuffer.size() - 1] != '\r')
                    _readBuffer += '\r';
            }
            _readBuffer += buffer[i];
        }
    }
    else
        _readBuffer.insert(_readBuffer.end(), buffer, buffer + n);
}

bool HttpRequest::isRequestValid()
{
    if (_isFirstLineReqValid)
        return true;

    size_t line_end = _readBuffer.find('\n');
    if (line_end == std::string::npos || !isMethodSpacesValid())
        return false;

    std::string line = _readBuffer.substr(0, line_end - 1);
    size_t first_space = line.find(' ');
    size_t second_space = line.find(' ', first_space + 1);
    if (first_space == std::string::npos || second_space == std::string::npos)
        return false;

    _method = line.substr(0, first_space);
    _path = line.substr(first_space + 1, second_space - first_space - 1);
    _version = line.substr(second_space + 1);

    // Method must be uppercase alphabetic
    for (size_t i = 0; i < _method.size(); ++i)
        if (!std::isupper(_method[i]))
            return false;

    if (_path.empty() || _path[0] != '/')
        return false;

    if (_version != "HTTP/1.1" && _version != "HTTP/1.0")
        return false;

    _isFirstLineReqValid = true;
    return true;
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
        // std::cout << "$$$$$$$$$$$$$$$$$$$ { ReadBuffer : " << _readBuffer << " } $$$$$$$$$$$$$$$$$$$$$$$";

        _isHeaderReady = true;
        return true;
    }
    reset();
    return false;
}

bool HttpRequest::parseHeader()
{
    size_t line_end;

    line_end = _header.find("\r\n");

    if (_isHeadeParse)
        return true;

    if (!fillHeaderMap(_header.substr(line_end + 2)))
    {
        reset();
        return false;
    }

    _isHeadeParse = true;
    return true;
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
    if (*endptr == '\0' || static_cast<long>(n) < 0)
        return false;
    if (_body.size() < n)
        return false;
    return true;
}
void HttpRequest::clearReadBuffer()
{
    _readBuffer.clear();
}
bool HttpRequest::isMethodSpacesValid()
{
    int offset;
    int offset2;

    offset = _readBuffer.find('\n');
    offset2 = _readBuffer.find("\r\n");
    std::string line = _readBuffer.substr(0, offset);
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
    _headerMap[line.substr(0, index)] = line.substr(index + 2, index3 - index - 2);
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

// bool HttpRequest::checkBodyIsReady()
// {
//     size_t  offset;
//     size_t  n;
//     char    *endptr;

//     if (_method == "POST" && !_isBodyReady)
//     {
//         offset = _readBuffer.find("\r\n\r\n") + 4;
//         n = strtol(getKeyValue("Content-Length").c_str(), &endptr, 10);
//         if (*endptr != '\0' || static_cast<long>(n) < 0)
//             return false;
//         if (_readBuffer.size() - offset < n)
//         {
//             _isBodyReady = false;
//             return false;
//         }
//         _body = _readBuffer.substr(offset, _readBuffer.size() - offset);
//     }
//     _isBodyReady = true;
//     return true;
// }

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

bool HttpRequest::isHeaderValidated() // is everything in the request is done like post with body or get with just  a valid header
{
    if (_isHeaderReady && _isHeadeParse && _isReqValid)
        return true;
    return false;
}
std::string HttpRequest::getBody() const
{
    return _body;
}
void HttpRequest::setIsReqValid(bool status)
{
    _isReqValid = status;
}

void HttpRequest::setCGI(bool status)
{
    _isCGI = status;
}