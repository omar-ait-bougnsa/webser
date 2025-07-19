#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "./HttpRequest.hpp"
#include "./HttpError.hpp"
#include "./Tools.hpp"
#include "./ConfigParser.hpp"
#include <dirent.h>
#include <sys/stat.h>

class RequestProcessor {
public:
    RequestProcessor(HttpRequest& request, VirtualHost& server);

    void process();  // main function to handle all request checks

    // Exposed Results
    bool        shouldCloseConnection() const;
    bool        hasError() const;
    bool        hasRedirect() const;
    int         getStatusCode() const;
    bool        useAutoIndex() const;
    bool        isCGI() const;
    std::string getResolvedPath() const;
    Route       getMatchedRoute() const;
    std::string getErrorPagePath() const;
    int     sendErrorResponse(int statusCode,int fd);
    std::string generateRandomString(size_t length = 32);
    std::string getCGIPath() const;

private:
    HttpRequest& request;
    VirtualHost& matchedServer;
    Route matchedRoute;
    std::string resolvedPath;
    int  statusCode;
    std::string cgipath;
    bool closeConnection;
    bool useCGI;
    bool autoIndex;
    bool error;
    bool redirect;
    bool isCookiesExist;

    // Request handling steps
    void matchLocation();
    void checkRedirection();
    void validateMethod();
    void resolvePath();
    void checkVersion();
    void checkTraversal();
    void checkAutoIndex();
    void checkCGI();
    void checkContentLength();
    void decideConnection();
    void handleErrors();
    void handleCookies();
};

#endif
