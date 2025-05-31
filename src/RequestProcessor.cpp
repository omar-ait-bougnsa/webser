#include "../include/RequestProcessor.hpp"

#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <algorithm>

RequestProcessor::RequestProcessor(const HttpRequest &request, const VirtualHost &server)
    : request(request), matchedServer(server), statusCode(200), closeConnection(false),
      useCGI(false), autoIndex(false), error(false) {}

void RequestProcessor::process()
{
    matchLocation();
    validateMethod();
    resolvePath();
    checkTraversal();
    checkAutoIndex();
    checkCGI();
    checkContentLength();
    decideConnection();
    handleErrors();

    matchedRoute.printRoute();
    std::cout << "=++++++++++++=======================================++++++++++++++=\n";
    std::cout << "PrefixPath: " << matchedRoute.path_prefix << " | PathResolved: " << resolvedPath << std::endl;
    std::cout << "is AutoIndex: " << autoIndex << " | isCGI: " << useCGI << "| error: " << this->error << std::endl;
    std::cout << "matchedRout Auto index: " << matchedRoute.autoindex << std::endl;

}

void RequestProcessor::matchLocation()
{
    matchedRoute = matchedServer.GetMatchRoute(request.getPath());
}

void RequestProcessor::validateMethod()
{
    const std::string &method = request.getMethod();
    if (std::find(matchedRoute.methods.begin(), matchedRoute.methods.end(), method) == matchedRoute.methods.end())
    {
        statusCode = 405; // Method Not Allowed
        error = true;
    }
}

void RequestProcessor::resolvePath()
{
    resolvedPath = matchedRoute.root_directory + request.getPath().substr(0,matchedRoute.path_prefix.size());
    std::cout << "RESOLVED PATH : " << resolvedPath << " | path:  " << request.getPath() << " | size: " << matchedRoute.path_prefix.size() << " @@@@@@@@@@@@@@@@@@@@\n";
}

void RequestProcessor::checkTraversal()
{
    char realBase[PATH_MAX];
    char realTarget[PATH_MAX];

    realpath(matchedRoute.root_directory.c_str(), realBase);
    realpath(resolvedPath.c_str(), realTarget);

    if (strncmp(realBase, realTarget, strlen(realBase)) != 0)
    {
        statusCode = 403; // Forbidden
        error = true;
    }
}

void RequestProcessor::checkAutoIndex()
{
    struct stat pathStat;
    std::string path = request.getPath();

    if (path.empty())
        return;
    
    if (stat(resolvedPath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
    {
        if (path[path.length() - 1] == '/' && matchedRoute.autoindex == "on")
        {
            autoIndex = true;
        }
        else
        {
            for (size_t i = 0; i < matchedRoute.index_files.size(); ++i)
            {
                std::string indexPath = resolvedPath + (resolvedPath[resolvedPath.size() -1 ] == '/' ? "": "/") + matchedRoute.index_files[i];
                std::cout << "$$$$$$$$$$$$$$$$$$$ { indexpath: " << indexPath << "} $$$$$$$$$$$$$$$$$\n";
                if (access(indexPath.c_str(), F_OK) == 0)
                {
                    resolvedPath = indexPath;
                    return;
                }
            }
            std::cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            statusCode = 403; // Directory listing denied
            error = true;
        }
    }
}

void RequestProcessor::checkCGI()
{
    for (size_t i = 0; i < matchedRoute.cgi.size(); ++i)
    {
        if (resolvedPath.find(matchedRoute.cgi[i]) != std::string::npos)
        {
            useCGI = true;
            return;
        }
    }
}

void RequestProcessor::checkContentLength() {
    if (request.getMethod() == "POST") {
        std::string contentLengthStr = request.getKeyValue("Content-Length");

        // If Content-Length is missing
        if (contentLengthStr.empty()) {
            statusCode = 411; // Length Required
            error = true;
            return;
        }

        // Manually check if Content-Length is a valid number
        for (size_t i = 0; i < contentLengthStr.length(); ++i) {
            if (!std::isdigit(contentLengthStr[i])) {
                statusCode = 400; // Bad Request
                error = true;
                return;
            }
        }

        size_t bodySize = std::atoi(contentLengthStr.c_str());

        // Check if body size exceeds the max allowed
        if (bodySize > matchedServer._clientMaxBodySize) {
            statusCode = 413; // Payload Too Large
            error = true;
        }
    }
}

void RequestProcessor::decideConnection()
{
    std::string connection = request.getKeyValue("Connection");
    closeConnection = (connection == "close");
}

void RequestProcessor::handleErrors()
{
    if (error && matchedServer._errorPages.count(statusCode))
    {
        resolvedPath = matchedServer._errorPages.at(statusCode);
    }
}

bool RequestProcessor::shouldCloseConnection() const
{
    return closeConnection;
}

bool RequestProcessor::hasError() const
{
    return error;
}

int RequestProcessor::getStatusCode() const
{
    return statusCode;
}

bool RequestProcessor::useAutoIndex() const
{
    return autoIndex;
}

bool RequestProcessor::isCGI() const
{
    return useCGI;
}

std::string RequestProcessor::getResolvedPath() const
{
    return resolvedPath;
}

Route RequestProcessor::getMatchedRoute() const
{
    return matchedRoute;
}

std::string RequestProcessor::getErrorPagePath() const
{
    return resolvedPath;
}
