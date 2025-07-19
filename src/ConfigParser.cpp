#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename): _fileName(filename) {}

bool ConfigParser::areServerNamesEqual(std::vector<std::string> str1,std::vector<std::string> str2)
{
    std::set<std::string> a(str1.begin(),str1.end());
    std::set<std::string> b(str2.begin(),str2.end());
    if (a == b)
        return true;
    return false;
}
void ConfigParser::handleMatchedVHosts()
{
    size_t  i = 0;
    size_t  j = 0;
    //std::vector<std::string> ServerName1; check server name is dublicate in some port
    //std::vector<std::string> ServerName2;
    while (i < _virtualHosts.size())
    {
        j = i + 1;
        if (_virtualHosts[i]._port <= 0)
            throw std::logic_error("Error: port is not set or invalid");
        while(j < _virtualHosts.size())
        {
            if (_virtualHosts[i]._port == _virtualHosts[j]._port)
            {
                //ServerName1 = _virtualHosts[i]._serverNames;
                //ServerName2 = _virtualHosts[j]._serverNames;
                //if (areServerNamesEqual(ServerName1,ServerName2))
                   // throw std::logic_error("Error: duplicate server with same host:port and server_name");
                if (_virtualHosts[i]._host.empty() || _virtualHosts[i]._host == "0.0.0.0")
                {
                _virtualHosts[i]._virtualHosts.push_back(_virtualHosts[j]);
                _virtualHosts.erase(_virtualHosts.begin() + j);
                continue;
                }
                else if (_virtualHosts[j]._host.empty() || _virtualHosts[j]._host == "0.0.0.0")
                {
                    _virtualHosts[j]._virtualHosts.push_back(_virtualHosts[i]);
                    _virtualHosts.erase(_virtualHosts.begin() + i);
                    break;
                      i -= 1;
                }
            }
            j++;
        }
        i++;
    }
}

std::vector<VirtualHost> ConfigParser::parse()
{
    VirtualHost                             host;
    std::ifstream                           file;
    std::vector<std::string>                str;
    bool                                    insideServer = false;
    size_t                                  pos;
    std::string                             line;
    std::vector<std::vector<std::string> >  servers;
    (void)insideServer;
    file.open(_fileName.c_str());
    if (!file.is_open())
      throw std::logic_error("can't open Configuration file");
    while (getline(file, line))
    {
        pos = line.find("#");
        if (pos != std::string::npos)
            line.erase(pos, line.length() - pos);
        if (!remove_space(line))
            continue;
        if (!line.empty () && line != "server")
        {
            if (line[line.size() - 1] != '{' && line[line.size() - 1] != '}' && line[line.size() - 1] != ';')
                throw std::logic_error("Error: line not close by semicolon");
            if (line[line.size() - 1] == ';')
                line.erase(line.size() - 1, 1);
            str.push_back(line);
        }
        if (line == "server" && !str.empty())
        {
            host.pars_server(str);
            _virtualHosts.push_back(host);
            host.clear();
            str.clear();
        }
        if (line == "server")
            str.push_back(line);
    }
    if (!str.empty())
    {
     host.pars_server(str);
    _virtualHosts.push_back(host);
    }
    handleMatchedVHosts();
     return _virtualHosts;
}

int     ConfigParser::remove_space(std::string &line)
{
    size_t pos;
    if (line.empty())
        return (0);
    pos = line.find_last_not_of("\n\t\r ");
    if (pos !=std::string::npos)
        line.erase(pos +1);
    else
        line.clear();
    pos = line.find_first_not_of("\n\t\r ");
    if (pos != std::string::npos)
        line.erase(0,pos);
    if (!line.empty())
        return 1;
    return 0;
}