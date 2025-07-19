#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "./RequestProcessor.hpp"
#include "cgi.hpp"
#include <sys/stat.h>
// #include "./HttpRequest.hpp"
// #include "ClientConnection.hpp"
class ClientConnection;

class HttpResponse
{
private:
    const HttpRequest&                       _request;
    int                                     _statusCode;
    std::string                             _boundary;
    std::string                             _reasonPhrase;   
    std::map<std::string, std::string>      _headers;
    std::string                             _body;
    bool                                    _bodyiscompletl;
    bool                                    _checkDespostionIfExist;
    RequestProcessor                        *_reqProcessor;
    size_t                                  _chunkSize;
    size_t                                  _readbodySize;
    bool                                    _isNewfile;
    size_t                                  _fileSize;
    bool                                    _firstTime;
    std::ofstream                           _file;
    
public:
    cgi                                      _cgi;
    size_t                                  _readSize;
    std::ifstream                           _infile;
    std::string                             _sendBuffer;
    bool                                    _waitingToSend;
    bool                                    _headerSent;
    size_t                                  _contentLenght;
    HttpResponse(const HttpRequest &request);
   // HttpResponse ();
    ~HttpResponse();
    // Setters
    void            decodeChunked(int fd);  
    // void            setHeader(const std::string &key, const std::string &value);
    void            setBoundary();
    void            setReqProcessor(RequestProcessor *processor);
    std::vector<char *> set_env(std::string filePath);
    int             getStatus() const;
    std::string     getReasonPhrase() const;
    std::string     getHeader(const std::string &key) const;
    std::string     getBody() const;
    void            process(int fd);
    std::string     check_extation (std::string path);
    void            handle_post(int fd);
    void            handle_get(int fd,std::string path);
    void            handle_delete (int fd);
    std::string     getFilename();
    //cgi
    void            execut_cgi(int fd,std::string filePath,std::string cgi_path);
    void            sendAutoIndexResponse(int fd, const std::string &dir_path);
    void            successfullrequest (std::string path, int fd);
    void            cgi_response(int fd, std::string body);
    std::string     getContentType();
};

#endif