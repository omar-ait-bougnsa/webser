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
    int         countChar(const std::string &str, char ch);

public:
    HttpRequest();
    ~HttpRequest();
    void addReadBuffer(const char *buffer, int n); // add a buffer to class
    bool isHeaderComplete();                // is header exist in the readBuffer
    bool isBodyComplete();                  // is the size of body complete or not, if not we need to accumulate data
    bool isDone();                            // is everythin in the request is done like post with body or get with just  a valid header
    bool isMethodSpacesValid();
    bool fillHeaderMap(std::string headers);
    bool parseHeader();
    bool checkBodyIsReady();
    bool isHeaderLineValid(const std::string& line);
    std::string getKeyValue(const std::string& key) const ;
    void reset();
    void getNextRequest();
    std::string getFullpath () const;
    std::string getMethod() const;
    std::string getPath() const;
    std::string getBody() const;
    std::string getVersion() const;
    void        setIsReqValid(bool status);
    // we well remove this function
    void setFullpath (std::string path);
};




#endif