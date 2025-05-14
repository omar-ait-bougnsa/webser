#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "./HttpRequest.hpp"
#include "./Tools.hpp"

class HttpResponse
{
private:
    const HttpRequest                       _request;
    int                                     _statusCode;
    std::string                             _boundary;
    std::string                             _reasonPhrase;                
    std::map<std::string, std::string>      _headers;
    std::string                             _body;

public:
    HttpResponse(const HttpRequest &request);
    HttpResponse ();
    ~HttpResponse();

    // Setters
    void            setStatus(int code);
    void            setHeader(const std::string &key, const std::string &value);
    void            setBody(const std::string &body);

    // Getters
    int             getStatus() const;
    std::string     getReasonPhrase() const;
    std::string     getHeader(const std::string &key) const;
    std::string     getBody() const;

    // Build Response
    std::string     buildResponse();

    // Handling Methods
    std::string     check_extation (std::string path);
    void            handel_post(int fd,std::vector<std::string> method,std::string request);
    void            handel_get(int fd,std::vector<std::string> method);

private:
    // Utility: Maps status code to reason phrase
    std::string     getDefaultReasonPhrase(int code);
};

#endif