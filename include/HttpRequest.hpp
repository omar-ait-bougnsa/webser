#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "./Tools.hpp"

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

class HttpRequest
{
private:
    std::string _readBuffer;
    std::string _header;
    std::string _method;
    std::string _path;
    std::string _version;
    std::string _fullpath;
    std::map<std::string, std::string> _headerMap;
    std::string _body;
    bool        _isHeaderReady;
    bool        _isBodyReady;
    bool        _isHeadeParse;
    bool        _isReqValid;
    bool        _isCGI;
    std::string _CGIpath;
    bool        _isFirstLineReqValid;
    int         countChar(const std::string &str, char ch);

public:
    HttpRequest();
    ~HttpRequest();
    void addReadBuffer(char *buffer, int n); // add a buffer to class
    bool isHeaderComplete();                // is header exist in the readBuffer
    bool isBodyComplete();                  // is the size of body complete or not, if not we need to accumulate data
    bool isHeaderValidated();
    bool isMethodSpacesValid();
    bool isRequestValid();
    bool fillHeaderMap(std::string headers);
    bool parseHeader();
    bool isHeaderLineValid(const std::string& line);
    std::string getKeyValue(const std::string& key) const ;
    void reset();
    void getNextRequest();

    std::string getFullpath () const;
    std::string getMethod() const;
    std::string getPath() const;
    std::string getBody() const;
    std::string getVersion() const;
    std::string getReadBuffer() const;
    std::string getHeader() const;
    bool        getCGI()    const;
    std::string getCGIPath() const;
    void        setIsReqValid(bool status);
    void        setCGI(bool status);
    void        setCGIPath(std::string path);
    // we well remove this function
    void setFullpath (std::string path);
    void clearReadBuffer();
};




#endif