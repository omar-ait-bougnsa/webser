#include "../include/VirtualHost.hpp"

VirtualHost::VirtualHost() : _serverFd(-1), _backlog(100), _port(0)
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
    std::stringstream ss;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;

    ss << _port;
    std::string str_port = ss.str();
    
    if (_host.empty())
        _host = "0.0.0.0"; 

    std::cout << "------------- { host:" << _host <<  "} +====================\n";
    std::cout << "------------- { port:" << str_port <<  "} +====================\n";
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;        // Only IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    
    if (getaddrinfo(_host.c_str(), str_port.c_str(), &hints, &res) != 0)
        throw std::runtime_error("Failed to getaddrinfo()");
    
    p = res;
    if (!p)
    {
        freeaddrinfo(res);
        throw std::runtime_error("No address info found");
    }
    
    _serverFd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (_serverFd == -1)
    {
        freeaddrinfo(res);
        _ErrorExit("Socket Failed");
    }

    int yes = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        freeaddrinfo(res);
        _ErrorExit("setsockopt failed");
    }

    int flags = fcntl(_serverFd, F_GETFL, 0);
    fcntl(_serverFd, F_SETFL, flags | O_NONBLOCK);

    if (bind(_serverFd, p->ai_addr, p->ai_addrlen) == -1)
    {
        freeaddrinfo(res);
        _ErrorExit("Bind Failed");
    }

    if (listen(_serverFd, _backlog) == -1)
    {
        freeaddrinfo(res);
        _ErrorExit("Listening Failed");
    }

    freeaddrinfo(res);
    return _serverFd;
}


void VirtualHost::_ErrorExit(const char *msg)
{
    perror(msg);
    exit(1);
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

// ======================= { OMAR PART } ==========================
void VirtualHost::parsHOST(std::string host)
{
    char *end;
    if (_port != 0)
        throw std::logic_error("Error: dublicat listen");
    std::vector<std::string> str = Tools::split(host, ':');
    if (str.size() > 2 || str.size() == 0)
        throw std::logic_error("Error: invalid port");
    if (str.size() == 2)
    {
        _host = str[0];
        _port = strtoll(str[1].c_str(), &end, 10);
    }
    else
        _port = strtoll(str[0].c_str(), &end, 10);
    if (end[0] != '\0' || _port <= 0)
        throw std::logic_error("Error: invalid port ");
}

void VirtualHost::clear()
{
    _port = 0;
    _host.clear();
    _serverNames.clear();
    _errorPages.clear();
    _clientMaxBodySize = 0;
    _route.clear();
    _virtualHosts.clear();
}

void VirtualHost::pars_body_size(std::string str)
{
    char *end;
    if (_clientMaxBodySize != 0)
        std::logic_error("Error: dublicat max body size");
    _clientMaxBodySize = strtoll(str.c_str(), &end, 10);
    if (end[0] != '\0')
        throw std::logic_error("Error client max body size");
}

void VirtualHost::pars_server(std::vector<std::string> server)
{
    size_t pos;
    Route route;
    std::pair<std::string, std::string> Pair;
    std::vector<std::string> str;

    check_bracket(server);
    for (size_t i = 0; i < server.size(); i++)
    {
        if ((pos = server[i].find(" ")) != std::string::npos || (pos = server[i].find("   ")) != std::string::npos)
        {
            Pair.first = server[i].substr(0, pos);
            Pair.second = server[i].substr(pos + 1, server[i].length() - pos);
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
            route.clear();
            str.clear();
        }
        else if (Pair.first == "listen")
        {
            parsHOST(Pair.second);
        }
        else if (Pair.first == "server_name")
        {
            if (!_serverNames.empty())
                throw std::logic_error("Error: dublicat server name");
            _serverNames = Tools::split_withspace(Pair.second);
        }
        else if (Pair.first == "error_page")
            prse_error_page(Pair.second);
        else if (Pair.first == "client_max_body_size")
            pars_body_size(Pair.second);
        else if ((Pair.first != "}" || !Pair.second.empty()) && !Pair.first.empty())
            throw std::logic_error("error in server");
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
        throw std::logic_error("Error: dublicat error page");
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
            throw std::logic_error("Error: invalid error page\n");
        i++;
    }
}

void VirtualHost::check_bracket(std::vector<std::string> &server)
{
    size_t i = 0;
    int bracket = 0;
    size_t pos;
    if (server[0] != "server")
        throw std::logic_error("Error: config some line is not in block server ");
    server.erase(server.begin());
    if (server[0] != "{")
        throw std::logic_error("Error: expected '{' after server");
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
        throw std::logic_error("Error bracket not closed\n");
    if (server[server.size() - 1] != "}")
        throw std::logic_error("Error: Server not closed by '}' ");
}

const VirtualHost &VirtualHost::GetMatchServer(const std::string &host) const

{
    std::vector<VirtualHost>::const_iterator it;

    if (host.empty() || _virtualHosts.size() == 0)
        return *this;
    for (it = _virtualHosts.begin(); it != _virtualHosts.end(); ++it)
    {
        if (std::find(it->_serverNames.begin(), it->_serverNames.end(), host) != it->_serverNames.end())
            return *it;
    }
    return *this;
}
