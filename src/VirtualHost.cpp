#include "../include/VirtualHost.hpp"

VirtualHost::VirtualHost() : _serverFd(-1), _backlog(100), _port(8080)
{
}

VirtualHost::~VirtualHost()
{
    if (_serverFd > 0)
        close(_serverFd);
}

int VirtualHost::GetServerFd() const
{
    return (_serverFd);
}

int VirtualHost::SocketSetup()
{
    sockaddr_in addr;

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);

    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd == -1)
        _ErrorExit("Socket Failed");

    int flags = fcntl(_serverFd, F_GETFL, 0);
    fcntl(_serverFd, F_SETFL, flags | O_NONBLOCK);

    if (bind(_serverFd, (sockaddr *)&addr, sizeof(addr)) == -1)
        _ErrorExit("Bind Failed"); // add the name of the server if possible
    if (listen(_serverFd, _backlog) == -1)
        _ErrorExit("Listning Failed"); // add the name of the server if possible
    return (_serverFd);
}

void VirtualHost::_ErrorExit(const char *msg)
{
    perror(msg);
    exit(1);
}

// ======================= { OMAR PART } ==========================

void VirtualHost::pars_server(std::vector<std::string> server, int size)
{
    size_t pos;
    char *end;
    (void)size;
    Route route;
    std::pair<std::string, std::string> Pair;
    std::vector<std::string> str;

    check_bracket(server);
    if (server[server.size() - 1] != "}")
        throw("Error some server not closed by '}' ");
    for (size_t i = 0; i < server.size(); i++)
    {
        if ((pos = server[i].find(" ")) != std::string::npos || (pos = server[i].find("   ")) != std::string::npos)
        {
            Pair.first = server[i].substr(0, pos);
            Pair.second = server[i].substr(pos, server[i].length() - pos);
        }
        else
            Pair.first = server[i];
        if (Pair.first == "location")
        {
            while (server[i] != "}")
            {
                str.push_back(server[i]);
                i++;
            }
            _route.push_back(route.pars_Route(str));
            str.clear();
        }
        else if (Pair.first == "listen")
            _port = atoi(Pair.second.c_str());
        else if (Pair.first == "server_name")
            _serverNames = Tools::split_withspace(Pair.second);
        else if (Pair.first == "error_page")
            prse_error_page(Pair.second);
        else if (Pair.first == "client_max_body_size")
        {
            size_t nb = strtoll(Pair.second.c_str(), &end, 10);
            if (end[0] == '\0')
                _clientMaxBodySize = nb;
            else
                throw("Error client max body size");
        }
        else if ((Pair.first != "}" || !Pair.second.empty()) && !Pair.first.empty())
            throw("error in server");
        Pair.first = "", Pair.second = "";
    }
}

void VirtualHost::prse_error_page(std::string str)
{
    std::pair<int, std::string> Pair;
    long int nb;
    char *pos;
    std::vector<std::string> v = Tools::split_withspace(str);
    size_t i = 0;
    if (v.size() < 2)
    {
        std::cout << "error\n";
        exit(1);
    }
    while (i < v.size() - 1)
    {
        nb = strtoll(v[i].c_str(), &pos, 10);
        if (pos[0] == '\0')
        {
            Pair.first = nb;
            Pair.second = v[v.size() - 1];
            _errorPages.insert(Pair);
        }
        else
        {
            std::cout << "error in page error\n";
            exit(1);
        }
        i++;
    }
}

void VirtualHost::check_bracket(std::vector<std::string> &server)
{
    size_t i = 0;
    int bracket = 0;
    size_t pos;
    while (i < server.size())
    {
        pos = server[i].find("{");
        if (pos != std::string::npos)
        {
            bracket += 1;
            server[i].erase(pos, 1);
        }
        pos = server[i].find("}");
        if (pos != std::string::npos)
            bracket -= 1;
        i++;
    }
    if (bracket != 0)
        throw("Error bracket not closed\n");
}

void VirtualHost::printServer() const 
{
    std::cout << "Server Configuration:\n";
        std::cout << "  Port: " << _port << "\n";
        
        std::cout << "  Server Names: ";
        for (size_t i = 0; i < _serverNames.size(); ++i)
            std::cout << _serverNames[i] << " ";
        std::cout << "\n";

        std::cout << "  Error Pages:\n";
        for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); ++it)
            std::cout << "    Error " << it->first << ": " << it->second << "\n";

        std::cout << "  Client Max Body Size: " << _clientMaxBodySize << "\n";
        
        for (size_t i = 0; i < _route.size(); ++i) 
        {
            std::cout << "    Route " << i + 1 << ":\n";
            _route[i].printRoute();
        }
}

Route VirtualHost::GetMatchRoute(const std::string &request_Path) const
{
    std::vector<Route>::const_iterator it;
    std::vector<Route>::const_iterator it_save = _route.end();

    for (it = _route.begin(); it != _route.end(); ++it)
    {
        if (request_Path.compare(0, it->path_prefix.size(), it->path_prefix) == 0)
        {
            if (it_save == _route.end() || it_save->path_prefix.size() < it->path_prefix.size())
                it_save = it;
        }
    }

    if (it_save == _route.end())
        throw std::runtime_error("No matching route found for path: " + request_Path);

    return (*it_save);
}
