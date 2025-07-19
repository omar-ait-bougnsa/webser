#ifndef CGI_HPP
#define CGI_HPP
#include "./RequestProcessor.hpp"
class cgi
{
public:
    std::map<std::string, std::string> _cgiHeader;
    std::string                        cgibody;
    std::vector<const char *>   env;
    int                         fd;
    int                         pip[2];
    int                         input[2];
    pid_t                       pid;
    std::ifstream               file;
    size_t                      time;
    bool                        headerSend;
    size_t                      contentLength;
    size_t                      readbytes;
    cgi();
    char buffer[4096];
    int status;
    void sendHeader(int fd,std::string buffer);
    void execute();
    bool sendResponse(int fd);
    void closePipes();
    void readOutput();
    void sendErrorResponse(int statusCode);
    void setCgiheader(std::string header);
    std::string getValuKey(std::string key);
};

#endif