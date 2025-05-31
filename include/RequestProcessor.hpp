#ifndef REQUESTPROCESSOR_HPP
#define REQUESTPROCESSOR_HPP

#include "./HttpRequest.hpp"
#include "./HttpResponse.hpp"
#include "./Validator.hpp"
#include "./HttpError.hpp"
#include "./Tools.hpp"
#include "./ConfigParser.hpp"
#include <dirent.h>
#include <sys/stat.h>


class RequestProcessor {
public:
    RequestProcessor(const HttpRequest& request, const VirtualHost& server);

    void process();  // main function to handle all request checks

    // Exposed Results
    bool shouldCloseConnection() const;
    bool hasError() const;
    int getStatusCode() const;
    bool useAutoIndex() const;
    bool isCGI() const;
    std::string getResolvedPath() const;
    Route getMatchedRoute() const;
    std::string getErrorPagePath() const;

private:
    const HttpRequest& request;
    const VirtualHost& matchedServer;
    Route matchedRoute;
    std::string resolvedPath;
    int statusCode;
    bool closeConnection;
    bool useCGI;
    bool autoIndex;
    bool error;

    // Request handling steps
    void matchLocation();
    void validateMethod();
    void resolvePath();
    void checkTraversal();
    void checkAutoIndex();
    void checkCGI();
    void checkContentLength();
    void decideConnection();
    void handleErrors();
};

#endif
