#include "../include/RequestProcessor.hpp"

#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <algorithm>

RequestProcessor::RequestProcessor( HttpRequest &request, VirtualHost &server)
    : request(request), matchedServer(server), statusCode(200), closeConnection(false), 
      useCGI(false), autoIndex(false), error(false), redirect(false), isCookiesExist(false) {}


int RequestProcessor::sendErrorResponse(int statusCode,int fd)
{
    std::stringstream ss;
    std::stringstream bodySize;
    std::string body;

    HttpError error(statusCode);
    ss << statusCode;

    std::string errorPagePath;

    if (matchedServer._errorPages.count(statusCode))
    {
        errorPagePath = matchedServer._errorPages.at(statusCode);
    }
    else
    {
        errorPagePath = "./static/errors/" + ss.str() + ".html";
    }

    body = Tools::readFile(errorPagePath);

    if (body.empty())
    {
        body = "<h1>" + error.getMessage() + "</h1>";
    }

    bodySize << body.size();

    std::string response = error.getStatusLine() + "\r\n" +
                           "Content-Length: " + bodySize.str() + "\r\n" +
                           "Content-Type: text/html\r\n" +
                           "Connection: close\r\n" +
                           "\r\n" +
                           body;
    send(fd, response.c_str(), response.length(), 0);
    return (-1);
}
void RequestProcessor::process()
{
    matchLocation();
    checkRedirection();
    validateMethod();
    checkVersion();
    resolvePath();
    checkTraversal();
    if (request.getMethod() != "POST")
        checkAutoIndex();
    checkCGI();
    checkContentLength();
    decideConnection();
    handleErrors();
    // std::cout  << "%%%%%%%%%%%%%%%%%%%%%%%[resolvepath] Error :" << this->error << "%%%%%%%%%%%%%%%%%%%%%%%%%%\n";

    // matchedRoute.printRoute();
    std::cout << "=++++++++++++=======================================++++++++++++++=\n";
    std::cout << "PrefixPath: " << matchedRoute.path_prefix << " | PathResolved: " << resolvedPath << std::endl;
    std::cout << "is AutoIndex: " << autoIndex << " | isCGI: " << useCGI << "| error: " << this->error << std::endl;
    std::cout << "matchedRout Auto index: " << matchedRoute.autoindex << std::endl;

}

void RequestProcessor::matchLocation()
{
    const VirtualHost &tmp_server = matchedServer.GetMatchServer(request.getKeyValue("Host"));
    if (tmp_server._serverNames.size() > 0)
        std::cout << "============= {servername :" << tmp_server._serverNames[0]<< "} +++===========\n";
    std::cout << "============={host :" << request.getKeyValue("Host") << "}=================\n";
    matchedRoute = tmp_server.GetMatchRoute(request.getPath());
    std::cout << "============={print route :}=================\n";
}


void RequestProcessor::checkRedirection()
{
    if (!matchedRoute.redirect_code.empty() && !matchedRoute.redirect_location.empty())
        redirect = true;
}

    
void RequestProcessor::checkVersion()
{
    std::string version = request.getVersion();

    if (version != "HTTP/1.1")
    {
        statusCode = 505;
        error = true;
    }
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
    resolvedPath = matchedRoute.root_directory + request.getPath();  //.substr(0,matchedRoute.path_prefix.size());
    std::cout << "RESOLVED PATH : " << resolvedPath << " | path:  " << request.getPath() << " | size: " << matchedRoute.path_prefix.size() << " @@@@@@@@@@@@@@@@@@@@\n";
    request.setFullpath(resolvedPath);
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
            statusCode = 403; // Directory listing denied
            error = true;
        }
    }
}
std::string RequestProcessor::getCGIPath() const 
{
    return cgipath;
}
void RequestProcessor::checkCGI()
{
    for (size_t i = 0; i < matchedRoute.cgi.size(); ++i)
    {
        if (resolvedPath.find(matchedRoute.cgi[i]) != std::string::npos)
        {
            request.setCGI(true);
            request.setCGIPath(matchedRoute.cgi_path[i]);
            useCGI = true;
            return;
        }
    }
}

void RequestProcessor::checkContentLength() {
    if (request.getMethod() == "POST") {
        std::string contentLengthStr = request.getKeyValue("Content-Length");

        size_t i;
        for (i = 0; i < contentLengthStr.length(); i++)
            if (contentLengthStr[i] != ' ')
                break; 
        contentLengthStr.erase(0, i);
        // If Content-Length is missing
        if (contentLengthStr.empty() && request.getKeyValue("Transfer-Encoding").empty()) {
            statusCode = 411; // Length Required
            error = true;
            return;
        }

        // Manually check if Content-Length is a valid number
        for (size_t i = 0; i < contentLengthStr.length(); ++i)
        {
            if (!std::isdigit(contentLengthStr[i])) {
                statusCode = 400; // Bad Request
                error = true;
                return;
            }
        }
        std::cout << "+++++++++++++++++++++++++++++++++\n";

        size_t  _contentLenght = strtol(contentLengthStr.c_str(),NULL,10);

        // Check if body size exceeds the max allowed
        std::cout << "MaxClienSize : " << matchedServer._clientMaxBodySize << "\n";
        std::cout << "contentLenght : " << _contentLenght << "\n";
        if (_contentLenght > matchedServer._clientMaxBodySize) {
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

bool RequestProcessor::hasRedirect() const
{
    return redirect;
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

int RequestProcessor::getStatusCode() const
{
    return statusCode;
}

std::string RequestProcessor::generateRandomString(size_t length)
{
    const std::string charset =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    std::string result;
    result.reserve(length);

    // Seed the random number generator only once
    static bool seeded = false;
    if (!seeded) {
        std::srand(std::time(NULL)); // Seed with current time
        seeded = true;
    }

    for (size_t i = 0; i < length; ++i)
    {
        result += charset[std::rand() % charset.size()];
    }

    return result;
}


void RequestProcessor::handleCookies()
{
    if (request.getKeyValue("Cookie").empty())
    {
        std::string sessionId = generateRandomString(32);
        matchedServer._sessions[sessionId] = SessionData();
    }
    else
        isCookiesExist = true;
}